#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ThreadCutterAudioProcessorEditor::ThreadCutterAudioProcessorEditor(ThreadCutterAudioProcessor& p)
	: AudioProcessorEditor(&p), processor(p), matchScoreDisplay(p.currentMfccScoreDisplay)
{
	// Make sure that before the constructor has finished, you've set the
	// editor's size to whatever you need it to be.
	setSize(400, 265);


	this->addAndMakeVisible(&matchScoreDisplay);

	thresholdSlider.setSliderStyle(Slider::LinearHorizontal);
	thresholdSlider.setRange(0.0, 1.0, 0.01);
	thresholdSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 16);
	thresholdSlider.setPopupDisplayEnabled(true, false, this);
	thresholdSlider.setValue(0.5);
	thresholdSlider.setTextValueSuffix("(Threshold)");
	thresholdSlider.addListener(this);
	addAndMakeVisible(&thresholdSlider);

	agcSpeedSlider.setSliderStyle(Slider::LinearHorizontal);
	agcSpeedSlider.setRange(0.0, 0.3, 0.01);
	agcSpeedSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 16);
	agcSpeedSlider.setPopupDisplayEnabled(true, false, this);
	agcSpeedSlider.setValue(0.15);
	agcSpeedSlider.setTextValueSuffix("(AGC Speed)");
	agcSpeedSlider.addListener(this);
	addAndMakeVisible(&agcSpeedSlider);

	for (int i = 0; i < 3; ++i)
	{
		doSampleButton[i].setButtonText("Capture sample " + std::to_string(i + 1));
		addAndMakeVisible(doSampleButton[i]);
		doSampleButton[i].addListener(this);

		enableSampleButton[i].setButtonText("Enabled");
		addAndMakeVisible(enableSampleButton[i]);
		enableSampleButton[i].setToggleState(false, false);
		enableSampleButton[i].addListener(this);
	}

	saveToFile.setButtonText("Save to file");
	saveToFile.addListener(this);
	addAndMakeVisible(saveToFile);

	loadFromFile.setButtonText("Load from file");
	loadFromFile.addListener(this);
	addAndMakeVisible(loadFromFile);

}

void ThreadCutterAudioProcessorEditor::setThresholdSliderValue(double value)
{
	thresholdSlider.setValue(value);
}

void ThreadCutterAudioProcessorEditor::setEnabledCheckboxChecked(int n, bool checked)
{
	enableSampleButton[n].setToggleState(checked, false);
}

ThreadCutterAudioProcessorEditor::~ThreadCutterAudioProcessorEditor()
{
}

//==============================================================================
void ThreadCutterAudioProcessorEditor::paint(Graphics& g)
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

	g.setColour(Colours::white);
	g.setFont(15.0f);


}

void ThreadCutterAudioProcessorEditor::resized()
{
	// This is generally where you'll want to lay out the positions of any
	// subcomponents in your editor..
	thresholdSlider.setBounds(50, 60, 300, 30);
	matchScoreDisplay.setBounds(50, 30, 300, 30); 
	for (int i = 0; i < 3; ++i)
	{
		doSampleButton[i].setBounds(60 + i * 100, 130, 80, 30);
		enableSampleButton[i].setBounds(60 + i * 100, 160, 80, 30);
	}

	saveToFile.setBounds(80, 210, 110, 30);
	loadFromFile.setBounds(210, 210, 110, 30);

	agcSpeedSlider.setBounds(50, 90, 300, 30);
}

void ThreadCutterAudioProcessorEditor::sliderValueChanged(Slider * slider)
{
	if (slider == &agcSpeedSlider)
	{
		processor.setAgcSpeed(agcSpeedSlider.getValue());
	}
	else if (slider == &thresholdSlider)
	{
		processor.setMfccScoreThreshold(thresholdSlider.getValue());
	}
}

void ThreadCutterAudioProcessorEditor::buttonClicked(Button * button)
{
	for (int i = 0; i < 3; ++i)
	{
		if (button == &doSampleButton[i])
		{
			processor.doCaptureSample(i);
		}
		else if (button == &enableSampleButton[i])
		{
			processor.setSampleEnabled(i, enableSampleButton[i].getToggleState());
		}
	}

	if (button == &saveToFile)
	{
		MemoryBlock mb;
		processor.getStateInformation(mb);

		auto fc = new FileChooser("Save as", File::nonexistent, "*.json", true);
		if (fc->browseForFileToSave(true))
		{
			auto file = fc->getResult();
			file.replaceWithText(mb.toString());
		}
		delete fc;
	}
	else if (button == &loadFromFile)
	{
		auto fc = new FileChooser("Load from", File::nonexistent, "*.json", true);
		if (fc->browseForFileToOpen())
		{
			auto file = fc->getResult();
			StringArray content;
			file.readLines(content);
			auto contentStr = content.joinIntoString("\n").toStdString();
			processor.setStateInformation(contentStr.c_str(), contentStr.length() + 1);
		}
		else
		{
			NativeMessageBox::showMessageBox(AlertWindow::AlertIconType::NoIcon, "Error", "Failed to load file.");
		}
		delete fc;
	}
}
