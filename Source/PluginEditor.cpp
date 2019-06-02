#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ThreadCutterAudioProcessorEditor::ThreadCutterAudioProcessorEditor(ThreadCutterAudioProcessor& p)
	: AudioProcessorEditor(&p), processor(p), test(p.currentMfccScoreDisplay)
{
	// Make sure that before the constructor has finished, you've set the
	// editor's size to whatever you need it to be.
	setSize(400, 235);


	this->addAndMakeVisible(&test);

	thresholdSlider.setSliderStyle(Slider::LinearHorizontal);
	thresholdSlider.setRange(0.0, 1.0, 0.01);
	thresholdSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 16);
	thresholdSlider.setPopupDisplayEnabled(true, false, this);
	thresholdSlider.setValue(0.5);
	thresholdSlider.setTextValueSuffix("(Threshold)");
	thresholdSlider.addListener(this);
	addAndMakeVisible(&thresholdSlider);

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
	test.setBounds(50, 30, 300, 30);
	for (int i = 0; i < 3; ++i)
	{
		doSampleButton[i].setBounds(60 + i * 100, 100, 80, 30);
		enableSampleButton[i].setBounds(60 + i * 100, 130, 80, 30);
	}

	saveToFile.setBounds(80, 180, 110, 30);
	loadFromFile.setBounds(210, 180, 110, 30);
}

void ThreadCutterAudioProcessorEditor::sliderValueChanged(Slider * slider)
{
	processor.setMfccScoreOffset(mfccScoreOffsetSlider.getValue());
	processor.setMfccScoreScale(mfccScoreScaleSlider.getValue());
	processor.setMfccScoreThreshold(thresholdSlider.getValue());
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
			file.appendText(mb.toString());
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
			auto contentStr = content.joinIntoString("\n").toRawUTF8();
			processor.setStateInformation(contentStr, sizeof(contentStr));
		}
		else
		{
			NativeMessageBox::showMessageBox(AlertWindow::AlertIconType::NoIcon, "Error", "Failed to load file.");
		}
		delete fc;
	}
}
