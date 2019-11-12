#include <windows.h>
#include "SoundDetector.h"
#include <complex>
#include <algorithm>
#include "../ExternalCode/c_speech_features/c_speech_features.h"
#include <cassert>
#include "../JuceLibraryCode/JuceHeader.h"

#define M_PI 3.14159265358979323846

SoundDetector::SoundDetector()
{
	param.svm_type = C_SVC;
	param.kernel_type = LINEAR;
	param.degree = 3;
	param.gamma = 0.0001;
	param.coef0 = 0;
	param.nu = 0.5;
	param.cache_size = 100;
	param.C = 10;
	param.eps = 1e-3;
	param.shrinking = 1;
	param.probability = 0;
}

vector<float> SoundDetector::spectrumToMfcc(vector<float> spect)
{

	return spect;
}

void SoundDetector::normalize(vector<float>& frame)
{
	float _max = 0;
	for (int i = 0; i < frame.size(); ++i)
	{
		_max = max(_max, abs(frame[i]));
	}
	if (_max != 0)
	{
		for (int i = 0; i < frame.size(); ++i)
		{
			frame[i] /= _max;
		}
	}
}

void SoundDetector::setDoTrainButton(TextButton * btn)
{
	doTrainButton = btn;
}

vector<svm_node> SoundDetector::getSvmNodes(vector<float> features)
{
	vector<svm_node> nodes(features.size() + 1);
	for (int i = 0; i < features.size(); ++i)
	{
		nodes[i].index = i + 1;
		nodes[i].value = features[i];
	}
	nodes[features.size()].index = -1;
	return nodes;
}

vector<vector<svm_node>> SoundDetector::audioSamplesToNodes(vector<float> samples, int rate)
{
	normalize(samples);
	vector<short> sFrame(samples.size());
	for (int i = 0; i < samples.size(); ++i)
	{
		sFrame[i] = (short)round(samples[i] * 32766);
	}

	csf_float* mfcc;

	int nMfccFrames = csf_mfcc(
		&sFrame[0],
		samples.size(),
		rate,
		mfccWinLen,
		mfccWinStep,
		mfccNCep,
		mfccNCep * 2,
		8192,
		100,
		10000,
		0.97,
		22,
		false,
		NULL,
		&mfcc
	);

	csf_float* logFBankFeatures;

	int nLogFBankFrames = csf_logfbank(
		&sFrame[0],
		samples.size(),
		sampRate,
		mfccWinLen,
		mfccWinStep,
		nLogFilter,
		8192,
		100,
		10000,
		0.97,
		NULL,
		&logFBankFeatures,
		NULL
	);

	int nFrames = min(nLogFBankFrames, nMfccFrames);

	vector<vector<svm_node>> ret(nMfccFrames);

	for (int i = 0; i < nFrames; ++i)
	{
		vector<float> features(nLogFilter + mfccNCep);
		for (int j = 0; j < mfccNCep; ++j) features[j] = mfcc[i * mfccNCep + j];
		for (int j = 0; j < nLogFilter; ++j) features[mfccNCep + j] = logFBankFeatures[i * nLogFilter + j];
		vector<svm_node> nodes = getSvmNodes(features);
		ret[i] = nodes;
	}
	delete[] mfcc;
	delete[] logFBankFeatures;
	return ret;
}

float SoundDetector::process(vector<float> frame)
{
	if (!model) return -2;
	vector<vector<svm_node>> features = audioSamplesToNodes(frame, sampRate);
	float ret = 0.f;
	for (int i = 0; i < features.size(); ++i)
	{
		double test[2];
		float result = svm_predict_probability(model, &features[i][0], test);
		ret += result;
		if (result > 0) OutputDebugString("!");
		//OutputDebugString((to_string(test[0]) + " " + to_string(test[1]) + "\n").c_str());
	}
	ret /= features.size();
	return ret;
}

void SoundDetector::addToTrainingSet(vector<float> samples, int rate, double label)
{
	vector<vector<svm_node>> features = audioSamplesToNodes(samples, rate);
	for (int i = 0; i < features.size(); ++i)
	{
		trainingSet.push_back(features[i]);
		trainingSetLabel.push_back(label);
	}
}

void SoundDetector::resetTraningSet()
{
	trainingSet.resize(0);
	trainingSetLabel.resize(0);
	model = nullptr;
}

void SoundDetector::train(vector<string> symptomSet, vector<string> normalSet)
{
	if (isTraining) return;
	trainingThread = new thread(&SoundDetector::doTrain, this, symptomSet, normalSet);
}

void SoundDetector::doTrain(vector<string> symptomSet, vector<string> normalSet)
{
	if (model) svm_free_model_content(model);
	isTraining = true;
	resetTraningSet();

	AudioFormatManager formatManager;
	formatManager.registerBasicFormats();
	AudioFormat *audioFormat = formatManager.getDefaultFormat();

	map<string, double> sampleFiles;
	for (auto const& sampleFilePath : normalSet)
		sampleFiles[sampleFilePath] = -1;
	for (auto const& sampleFilePath : symptomSet)
		sampleFiles[sampleFilePath] = 1;

	for (auto const& sampleFile : sampleFiles)
	{
		OutputDebugString(("Preparing " + sampleFile.first + "\n").c_str());
		MemoryMappedAudioFormatReader *reader = audioFormat->createMemoryMappedReader(File(sampleFile.first));
		
		int rate = reader->sampleRate;
		int nChannels = reader->numChannels;
		int length = reader->lengthInSamples;

		int frameSize = rate / 2;
		
		for (int i = 0; i < length; i += frameSize)
		{
			vector<float> samples;
			reader->mapSectionOfFile(Range<int64>(i, min(i + frameSize, length)));
			for (int j = i; j < min(i + frameSize, length); ++j)
			{
				float *_samples = new float[nChannels];
				reader->getSample(j, _samples);

				float sample = 0.;
				for (int k = 0; k < nChannels; ++k)
					sample += _samples[k];
				sample /= (float)nChannels;
				delete _samples;

				samples.push_back(sample);
			}
			addToTrainingSet(samples, rate, sampleFile.second);
		}

		delete reader;
	}

	int nSymptomSamples = 0;
	int nNormalSamples = 0;
	for (int i = 0; i < trainingSet.size(); ++i)
	{
		if (trainingSetLabel[i] > 0) nSymptomSamples++;
		else nNormalSamples++;
	}
	double symptomWeight = nNormalSamples / (double)trainingSet.size();
	double normalWeight = nSymptomSamples / (double)trainingSet.size();

	vector<int> weightLabel = {-1, 1};
	vector<double> weight = {normalWeight, symptomWeight};
	

	param.nr_weight = 2;
	param.weight_label = &weightLabel[0];
	param.weight = &weight[0];

	svm_problem prob;
	prob.l = trainingSet.size();
	prob.y = &trainingSetLabel[0];
	prob.x = new svm_node*[trainingSet.size()];

	for (int i = 0; i < trainingSet.size(); ++i)
		prob.x[i] = &trainingSet[i][0];

	OutputDebugString("Start traning\n");

	model = svm_train(&prob, &param);
	delete[] prob.x;

	const MessageManagerLock mmLock;
	if (doTrainButton) doTrainButton->setEnabled(true);
	isTraining = false;
	NativeMessageBox::showMessageBox(AlertWindow::AlertIconType::NoIcon, "Success", "Model successfully trained. You can use it now or save it to a file.");
}

void SoundDetector::saveModel(string path)
{
	if (model->free_sv == 1)
	{
		NativeMessageBox::showMessageBox(AlertWindow::AlertIconType::NoIcon, "Error", "Cannot save externally loaded model.");
		return;
	}
	svm_save_model(path.c_str(), model);
}

void SoundDetector::loadModel(string path)
{
	if (model) svm_free_model_content(model);
	model = svm_load_model(path.c_str());
}

SoundDetector::~SoundDetector()
{
}

void SoundDetector::setSampRate(int rate)
{
	sampRate = rate;
}

bool SoundDetector::getIsTraining()
{
	return isTraining;
}
