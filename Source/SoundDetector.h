#pragma once
#include <vector>
#include <map>
#include <thread>
#include "../ExternalCode/libsvm/svm.h"
#include "../JuceLibraryCode/JuceHeader.h"

using namespace std;

class SoundDetector
{
public:
	SoundDetector();
	vector<float> spectrumToMfcc(vector<float> spect);
	float process(vector<float> frame);
	void addToTrainingSet(vector<float> samples, int rate, double label);
	void resetTraningSet();
	void train(vector<string> symptomSet, vector<string> normalSet);
	void saveModel(string path);
	void loadModel(string path);
	~SoundDetector();
	void setSampRate(int rate);
	bool getIsTraining();

	static void normalize(vector<float>& frame);
	void setDoTrainButton(TextButton *btn);

	

private:
	void doTrain(vector<string> symptomSet, vector<string> normalSet);
	vector<svm_node> getSvmNodes(vector<float> features);
	vector<vector<svm_node>> audioSamplesToNodes(vector<float> samples, int rate);
	vector<float> frameBuffer;
	vector<float> sampleSpectrum;
	vector<float> sampleCepstrum;
	vector<float> linearScale;
	float calculatedDistance;
	int frameSize;
	thread *worker = nullptr;

	float lowFreqBound = 300;
	float highFreqBound = 10000;

	int sampRate = 44100;

	int sampleSpectMaxFreq;

	float mfccWinLen = 0.1;
	float mfccWinStep = 0.02;
	int mfccNCep = 30;
	int nLogFilter = 30;

	TextButton *doTrainButton = nullptr;

	svm_parameter param;
	svm_model *model = NULL;

	vector<vector<svm_node>> trainingSet;
	vector<double> trainingSetLabel;
	thread *trainingThread = nullptr;
	bool isTraining = false;

};

