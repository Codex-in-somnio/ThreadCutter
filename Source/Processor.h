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
	void reload();
	double getMuteTime();
	void setGetMuteTimeFrom(Processor *p);
	std::string dumpCurrentState();
	void loadState(std::string jsonText);
	double getThreshold();
	void setThreshold(double value);
	void setGateLevel(double value);
	SoundDetector * getSoundDetector();
	double getCurrentMfccScore();
	double getCurrentAvgPeakLevel();
	double getCurrentPeakLevel();

private:
	std::vector<double> buffer;
	uint16_t bufferPtr = 0;
	int frameSize;
	int step;
	bool process();
	double muteTime = 0;
	double releaseTime = 4096 * 8;
	SoundDetector detector;
	bool doDetection = false;
	double currentMfccScore;
	Processor *getMuteTimeFrom;

	double threshold = 0.5;
	double gateLevel = 0.5;
	float avgPeakLevel = 0.;
	float currentPeakLevel = 0.;

};

