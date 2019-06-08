#pragma once
#include <vector>
#include <cstdint>
#include "SoundDetector.h"

class Processor
{
public:
	Processor();
	~Processor();
	void addSamples(std::vector<double> samples);
	std::vector<double> getSamples(int nSamples);
	void setFrameSize(int size);
	void setDoDetection(bool _doDetection);
	double getCurrentMfccScore();
	void setMfccScoreOffset(double value);
	void setMfccScoreScale(double value);
	void setMfccScoreThreshold(double value);
	void setAgcSpeed(double value);
	void reload();
	void doCaptureSample(int n);
	void setSampleEnabled(int n, bool en);
	double getMuteTime();
	void setGetMuteTimeFrom(Processor *p);
	std::string dumpCurrentState();
	void loadState(std::string jsonText);
	double getThreshold();
	bool getSampleEnabled(int n);

private:
	std::vector<double> buffer;
	uint16_t bufferPtr = 0;
	int frameSize;
	int step;
	bool process();
	double muteTime = 0;
	double releaseTime = 4096 * 4;
	std::vector<SoundDetector> detectors;
	std::vector<std::vector<double>> sampleSpectrum;
	std::vector<bool> sampleEnabled;
	bool doDetection = false;
	double currentMfccScore;
	bool capturingSample = false;
	int capturingSampleId;
	Processor *getMuteTimeFrom;
	double getAgcGained(double sample);

	double mfccScoreOffset;
	double mfccScoreScale;
	double threshold = 0.5;
	double agcSpeed = 0.2;
	double agcGain = 1;
	double agcMaxGain = 100.;

};

