#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class ThreadCutterAudioProcessorEditor : public AudioProcessorEditor, private Slider::Listener, private Button::Listener
{
public:
	ThreadCutterAudioProcessorEditor(ThreadCutterAudioProcessor&);
	void setThresholdSliderValue(double value);
	
	~ThreadCutterAudioProcessorEditor();

	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;

private:
	void sliderValueChanged(Slider* slider) override;
	void buttonClicked(Button* button) override;

	// This reference is provided as a quick way for your editor to
	// access the processor object that created it.
	ThreadCutterAudioProcessor& processor;

	ProgressBar matchScoreDisplay;
	ProgressBar avgLevelDisplay;
	ProgressBar peakLevelDisplay;
	Slider thresholdSlider;
	Slider gateSlider;
	TextButton doTrain;

	TextButton saveToFile;
	TextButton loadFromFile;

	Label gateSliderLabel;
	Label svmLabel;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThreadCutterAudioProcessorEditor)
};
