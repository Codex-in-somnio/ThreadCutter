#include "Processor.h"
#include <windows.h>
#include <string>
#include "../ExternalCode/picojson.h"
#include "../JuceLibraryCode/JuceHeader.h"

Processor::Processor()
{
	buffer.resize(65536);
	step = 2048;

	frameSize = 4096;

	sampleSpectrum.resize(3);
	sampleEnabled.resize(3);
	for (int i = 0; i < 3; ++i)
	{
		sampleSpectrum[i].resize(frameSize);
		for (int j = 0; j < frameSize; ++j) sampleSpectrum[i][j] = 0;
		sampleEnabled[i] = false;
	}
}


Processor::~Processor()
{
}

void Processor::addSamples(std::vector<double> samples)
{
	for (int i = 0; i < samples.size(); ++i)
	{
		buffer[bufferPtr] = samples[i];
		bufferPtr++; 
		if (bufferPtr % step == 0)
		{
			if (doDetection)
			{
				if (process()) muteTime = releaseTime;
			}
			else
			{
				muteTime = getMuteTimeFrom->getMuteTime();
			}
		}
	}
}

std::vector<double> Processor::getSamples(int nSamples)
{
	std::vector<double> ret(nSamples);
	for (int i = 0; i < nSamples; ++i)
	{
		uint16_t ptr = bufferPtr - frameSize * 2 + i;
		if (muteTime > 0)
		{
			ret[i] = 0;
			muteTime--;
		}
		else
		{
			ret[i] = buffer[ptr];
		}
	}
	return ret;
}

void Processor::setFrameSize(int size)
{
	frameSize = size;
}

void Processor::setDoDetection(bool _doDetection)
{
	doDetection = _doDetection;
	if (doDetection)
	{
		reload();
	}
}

double Processor::getCurrentMfccScore()
{
	return currentMfccScore;
}

void Processor::setMfccScoreOffset(double value)
{
	mfccScoreOffset = value;
}

void Processor::setMfccScoreScale(double value)
{
	mfccScoreScale = value;
}

void Processor::setMfccScoreThreshold(double value)
{
	threshold = value;
}

void Processor::reload()
{
	if (doDetection)
	{
		detectors.resize(sampleSpectrum.size());
		for (int i = 0; i < detectors.size(); ++i)
		{
			std::vector<double> spect = sampleSpectrum[i];
			SoundDetector sd(spect);
			detectors[i] = sd;
		}
	}
}

void Processor::setSampleEnabled(int n, bool en)
{
	sampleEnabled[n] = en;
}

double Processor::getMuteTime()
{
	return muteTime;
}

void Processor::setGetMuteTimeFrom(Processor *p)
{
	getMuteTimeFrom = p;
}

std::string Processor::dumpCurrentState()
{
	picojson::array _sampleSpect(sampleSpectrum.size());
	for (int i = 0; i < sampleSpectrum.size(); ++i)
	{
		auto spect = picojson::array(sampleSpectrum[i].size());
		for (int j = 0; j < sampleSpectrum[i].size(); ++j)
		{
			spect[j] = picojson::value(sampleSpectrum[i][j]);
		}
		_sampleSpect[i] = picojson::value(spect);
	}

	picojson::array _sampleEnabled(sampleEnabled.size());
	for (int i = 0; i < sampleEnabled.size(); ++i)
	{
		_sampleEnabled[i] = picojson::value(sampleEnabled[i]);
	}

	picojson::object jsonObj;

	jsonObj["sample_spectrum"] = picojson::value(_sampleSpect);
	jsonObj["sample_enabled"] = picojson::value(_sampleEnabled);
	jsonObj["threshold"] = picojson::value(threshold);

	std::string ret = picojson::value(jsonObj).serialize();
	return ret;
}

void Processor::loadState(std::string jsonText)
{
	if (jsonText.empty()) return;

	picojson::value val;
	std::string err = picojson::parse(val, jsonText);
	if (!err.empty())
	{
		NativeMessageBox::showMessageBox(AlertWindow::AlertIconType::NoIcon, "Error", err);
		return;
	}

	picojson::object jsonObj = val.get<picojson::object>();

	picojson::array _sampleSpect = jsonObj["sample_spectrum"].get<picojson::array>();
	if (_sampleSpect.size() != sampleSpectrum.size())
	{
		NativeMessageBox::showMessageBox(AlertWindow::AlertIconType::NoIcon, "Error", "Frame size is different.");
		return;
	}

	for (int i = 0; i < _sampleSpect.size(); ++i)
	{
		picojson::array __sampleSpect = _sampleSpect[i].get<picojson::array>();
		sampleSpectrum[i].resize(__sampleSpect.size());
		for (int j = 0; j < sampleSpectrum[i].size(); ++j)
		{
			sampleSpectrum[i][j] = __sampleSpect[j].get<double>();
		}
	}

	picojson::array _sampleEnabled = jsonObj["sample_enabled"].get<picojson::array>();
	sampleEnabled.resize(_sampleEnabled.size());
	for (int i = 0; i < _sampleEnabled.size(); ++i)
	{
		sampleEnabled[i] = _sampleEnabled[i].get<bool>();
	}

	threshold = jsonObj["threshold"].get<double>();

}

void Processor::doCaptureSample(int n)
{
	capturingSample = true;
	capturingSampleId = n;
}

bool Processor::process()
{
	std::vector<double> frame(frameSize);
	for (int i = 0; i < frameSize; ++i)
	{
		uint16_t ptr = bufferPtr - frameSize + i;
		frame[i] = buffer[ptr];
	}

	if (capturingSample)
	{
		SoundDetector::getSpectrum(frame);
		sampleSpectrum[capturingSampleId] = frame;
		capturingSample = false;
		reload();
		return false;
	}

	double mfccScore = 0;
	for (int i = 0; i < detectors.size(); ++i)
	{
		if (sampleEnabled[i])
		{
			double distance = detectors[i].process(frame);
			mfccScore = std::max((1 - distance - 0.35) * 3, mfccScore);
		}
	}
	currentMfccScore = std::min(mfccScore, 1.);

	//OutputDebugString(std::to_string(currentMfccScore).c_str());
	//OutputDebugString("\n");

	bool result = currentMfccScore > threshold;

	return result;
}
