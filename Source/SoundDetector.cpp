#include "SoundDetector.h"
#include "../ExternalCode/FftComplex.hpp"
#include "../ExternalCode/FastDctFft.hpp"
#include <complex>
#include <algorithm>

#define M_PI 3.14159265358979323846

int findMaxFromSpectrum(std::vector<double> spect)
{
	int max_pos = 0;
	double max_ = 0;

	for (int i = 0; i < spect.size() / 2; ++i)
	{
		if (max_ < spect[i])
		{
			max_ = spect[i];
			max_pos = i;
		}
	}
	return max_pos;
}

SoundDetector::SoundDetector()
{
}

SoundDetector::SoundDetector(std::vector<double> _sampleSpectrum)
{
	sampleSpectrum = _sampleSpectrum;

	SoundDetector::filter(sampleSpectrum);

	sampleSpectMaxFreq = findMaxFromSpectrum(sampleSpectrum);

	linearScale.resize(sampleSpectrum.size());

	for (int i = 0; i < linearScale.size(); ++i)
	{
		linearScale[i] = i;
	}

	std::vector<double> samples(std::begin(sampleSpectrum), std::end(sampleSpectrum));
	sampleSpectSpline.set_points(linearScale, samples);
	sampleCepstrum = spectrumToMfcc(sampleSpectrum);
}

void SoundDetector::setFrameSize(int _frameSize)
{
	frameSize = _frameSize;
	frameBuffer.resize(frameSize);
}

void SoundDetector::filter(std::vector<double>& spect)
{
	int _lowFreqBound = std::round(lowFreqBound / sampRate * spect.size());
	int _highFreqBound = std::round(highFreqBound / sampRate * spect.size());
	for (int i = 0; i < _lowFreqBound; ++i) spect[i] = 0, spect[spect.size() - 1 - i] = 0;
	for (int i = _highFreqBound; i < spect.size() / 2; ++i) spect[i] = 0, spect[spect.size() - 1 - i] = 0;
}

std::vector<double> SoundDetector::spectrumToMfcc(std::vector<double> spect)
{
	tk::spline s;
	s.set_points(linearScale, spect);

	double scaler = spect.size() / std::log10(spect.size() / 700.);

	double sum = 0;
	for (int i = 0; i < spect.size() / 2; i++)
	{
		spect[i] = s(scaler * std::log10(i / 700.));
		if (std::isnan(spect[i])) spect[i] = 0;
		spect[spect.size() - 1 - i] = spect[i];
		sum += spect[i];
	}
	for (int i = 0; i < spect.size(); i++)
	{
		spect[i] /= sum * 2 / spect.size();
	}
	FastDctFft::transform(spect);

	return spect;
}

void SoundDetector::getSpectrum(std::vector<double>& frame)
{
	std::vector<std::complex<double>> samples(frame.size());

	for (int i = 0; i < frame.size(); i++)
	{
		samples[i] = 0.5 * (1 - cos(2 * M_PI * i / (double)(frame.size() - 1))) * frame[i];
	}

	Fft::transform(samples);

	for (int i = 0; i < frame.size(); i++)
	{
		frame[i] = std::abs(samples[i]);
	}
}

void SoundDetector::_process(std::vector<double> frame)
{
	getSpectrum(frame);
	SoundDetector::filter(frame);
	std::vector<double> mfcc = spectrumToMfcc(frame);

	double distance = 0;
	for (int i = 0; i < frame.size(); i++)
	{
		distance += std::pow(mfcc[i] - sampleCepstrum[i], 2);
	}
	distance = (std::pow(distance, 1. / frame.size()) - 1) * 100;

	// ------------------

	int maxFreq = findMaxFromSpectrum(frame);

	double ratio = maxFreq / (double)sampleSpectMaxFreq;

	int scaled_size = std::min((int)(ratio * 300), (int)frame.size() / 2);

	if (ratio > 2 || ratio < 0.5) scaled_size = 0;

	std::vector<double> scaled_sample_spectrum(scaled_size);

	for (int i = 0; i < scaled_sample_spectrum.size(); ++i)
	{
		if (i * ratio >= frame.size() / 2)
		{
			scaled_size = i;
			break;
		}
		scaled_sample_spectrum[i] = sampleSpectSpline(i * ratio);
		//scaled_spectrum[i] = spectrum[i];
	}

	double sum_ = 0;

	for (int i = 0; i < scaled_size; ++i)
	{
		sum_ += pow(frame[i], 2) * pow(scaled_sample_spectrum[i], 2) / std::pow(scaled_size, 4);
	}

	calculatedDistance = distance - sum_ / 5;
}

double SoundDetector::process(std::vector<double> frame)
{
	if (worker != nullptr)
	{
		worker->join();
		delete worker;
	}

	worker = new std::thread(&SoundDetector::_process, this, frame);
	return calculatedDistance;
}

SoundDetector::~SoundDetector()
{
}
