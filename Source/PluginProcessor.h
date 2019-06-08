#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Processor.h"

//==============================================================================
/**
*/
class ThreadCutterAudioProcessor : public AudioProcessor
{
public:
	//==============================================================================
	ThreadCutterAudioProcessor();
	~ThreadCutterAudioProcessor();

	//==============================================================================
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

	void processBlock(AudioBuffer<float>&, MidiBuffer&) override;

	//==============================================================================
	AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override;

	//==============================================================================
	const String getName() const override;

	bool acceptsMidi() const override;
	bool producesMidi() const override;
	bool isMidiEffect() const override;
	double getTailLengthSeconds() const override;

	//==============================================================================
	int getNumPrograms() override;
	int getCurrentProgram() override;
	void setCurrentProgram(int index) override;
	const String getProgramName(int index) override;
	void changeProgramName(int index, const String& newName) override;

	//==============================================================================
	void getStateInformation(MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	double currentMfccScoreDisplay;

	void setMfccScoreOffset(double value);
	void setMfccScoreScale(double value);
	void setMfccScoreThreshold(double value);
	void setAgcSpeed(double value);
	void doCaptureSample(int n);
	void setSampleEnabled(int n, bool en);

private:
	AudioProcessorEditor *editor = nullptr;
	Processor processor[2];
	void setEditorValues();


	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThreadCutterAudioProcessor)
};
