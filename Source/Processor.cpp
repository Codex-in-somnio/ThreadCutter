#include "Processor.h"
#include <string>
#include "../JuceLibraryCode/JuceHeader.h"

Processor::Processor()
{
	buffer.resize(65536);
	step = 256;
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
	step = size / 2;
}

void Processor::setDoDetection(bool _doDetection)
{
	doDetection = _doDetection;
	if (doDetection)
	{
		reload();
	}
}

void Processor::reload()
{
	if (doDetection)
	{
		
	}
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
{/*
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
	return ret;*/
	return "";
}

void Processor::loadState(std::string jsonText)
{
	/*
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

	reload();
	*/

}

double Processor::getThreshold()
{
	return threshold;
}

void Processor::setThreshold(double value)
{
	threshold = value;
}

void Processor::setGateLevel(double value)
{
	gateLevel = value;
}

SoundDetector * Processor::getSoundDetector()
{
	return &detector;
}

double Processor::getCurrentMfccScore()
{
	return (currentMfccScore + 1) / 2.;
}

double Processor::getCurrentAvgPeakLevel()
{
	return avgPeakLevel;
}

double Processor::getCurrentPeakLevel()
{
	return currentPeakLevel;
}

bool Processor::process()
{
	std::vector<float> frame(frameSize);
	float peakLevel = 0.;
	for (int i = 0; i < frameSize; ++i)
	{
		uint16_t ptr = bufferPtr - frameSize + i;
		frame[i] = buffer[ptr];
		if (std::abs(frame[i]) > peakLevel)
			peakLevel = std::abs(frame[i]);
	}

	avgPeakLevel = avgPeakLevel * 0.95 + peakLevel * 0.05;

	currentPeakLevel = std::min(peakLevel / avgPeakLevel / 4., 1.);

	if (peakLevel == 0. || currentPeakLevel < gateLevel)
	{
		currentMfccScore = -2.;
		return false;
	}

	float result = detector.process(frame);
	currentMfccScore = result;

	return result > threshold;
}
