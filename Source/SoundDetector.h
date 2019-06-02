#pragma once
#include <vector>
#include <thread>
#include "../ExternalCode/spline.h"

class SoundDetector
{
public:
	SoundDetector();
	SoundDetector(std::vector<double> _sampleSpectrum);
	void setFrameSize(int frameSize);
	void filter(std::vector<double>& spect);
	std::vector<double> spectrumToMfcc(std::vector<double> spect);
	double process(std::vector<double> frame);
	~SoundDetector();

	static void getSpectrum(std::vector<double>& frame);

private:
	void _process(std::vector<double> frame);
	std::vector<double> frameBuffer;
	std::vector<double> sampleSpectrum;
	std::vector<double> sampleCepstrum;
	std::vector<double> linearScale;
	double calculatedDistance;
	int frameSize;
	std::thread *worker = nullptr;

	double lowFreqBound = 300;
	double highFreqBound = 10000;

	int sampRate = 44100;

	tk::spline sampleSpectSpline;
	int sampleSpectMaxFreq;

};

