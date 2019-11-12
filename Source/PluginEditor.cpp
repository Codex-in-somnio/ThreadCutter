#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ThreadCutterAudioProcessorEditor::ThreadCutterAudioProcessorEditor(ThreadCutterAudioProcessor& p)
	: AudioProcessorEditor(&p), processor(p), matchScoreDisplay(p.currentMfccScoreDisplay), avgLevelDisplay(p.currentAvgPeakLevelDisplay), peakLevelDisplay(p.currentPeakLevelDisplay)
{
	// Make sure that before the constructor has finished, you've set the
	// editor's size to whatever you need it to be.
	setSize(400, 330);


	this->addAndMakeVisible(&matchScoreDisplay);
	this->addAndMakeVisible(&avgLevelDisplay);
	this->addAndMakeVisible(&peakLevelDisplay);

	thresholdSlider.setSliderStyle(Slider::LinearHorizontal);
	thresholdSlider.setRange(-1.0, 1.0, 0.01);
	thresholdSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 16);
	thresholdSlider.setPopupDisplayEnabled(true, false, this);
	thresholdSlider.setValue(p.getMainAudioProcessor()->getThreshold());
	thresholdSlider.setTextValueSuffix("(Threshold)");
	thresholdSlider.addListener(this);
	addAndMakeVisible(&thresholdSlider);

	gateSlider.setSliderStyle(Slider::LinearHorizontal);
	gateSlider.setRange(0.0, 1.0, 0.01);
	gateSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 16);
	gateSlider.setPopupDisplayEnabled(true, false, this);
	gateSlider.setValue(p.getMainAudioProcessor()->getThreshold());
	gateSlider.setTextValueSuffix("");
	gateSlider.addListener(this);
	addAndMakeVisible(&gateSlider);

	gateSliderLabel.setText("Gate", NotificationType::dontSendNotification);
	svmLabel.setText("Detection", NotificationType::dontSendNotification);
	addAndMakeVisible(&gateSliderLabel);
	addAndMakeVisible(&svmLabel);


	doTrain.setButtonText("Train new model");
	doTrain.addListener(this);
	addAndMakeVisible(doTrain);

	saveToFile.setButtonText("Save model to file");
	saveToFile.addListener(this);
	addAndMakeVisible(saveToFile);

	loadFromFile.setButtonText("Load model from file");
	loadFromFile.addListener(this);
	addAndMakeVisible(loadFromFile);

	processor.getMainAudioProcessor()->getSoundDetector()->setDoTrainButton(&doTrain);
	bool isTraining = processor.getMainAudioProcessor()->getSoundDetector()->getIsTraining();
	if (isTraining) doTrain.setEnabled(false);
}

void ThreadCutterAudioProcessorEditor::setThresholdSliderValue(double value)
{
	thresholdSlider.setValue(value);
}

ThreadCutterAudioProcessorEditor::~ThreadCutterAudioProcessorEditor()
{
	processor.getMainAudioProcessor()->getSoundDetector()->setDoTrainButton(nullptr);
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

	gateSliderLabel.setBounds(50, 20, 300, 30);
	avgLevelDisplay.setBounds(50, 60, 300, 30);
	peakLevelDisplay.setBounds(50, 100, 300, 30);
	gateSlider.setBounds(50, 130, 300, 30);

	svmLabel.setBounds(50, 160, 300, 30);
	matchScoreDisplay.setBounds(50, 200, 300, 30);
	thresholdSlider.setBounds(50, 230, 300, 30);
	doTrain.setBounds(60, 260, 80, 30);
	saveToFile.setBounds(160, 260, 80, 30);
	loadFromFile.setBounds(260, 260, 80, 30);
}

void ThreadCutterAudioProcessorEditor::sliderValueChanged(Slider * slider)
{
	if (slider == &thresholdSlider)
	{
		processor.getMainAudioProcessor()->setThreshold(thresholdSlider.getValue());
	}
	else if (slider == &gateSlider)
	{
		processor.getMainAudioProcessor()->setGateLevel(gateSlider.getValue());
	}
}

void ThreadCutterAudioProcessorEditor::buttonClicked(Button * button)
{
	if (button == &doTrain)
	{
		vector<string> ticsSampleFiles;
		vector<string> nonTicsSampleFiles;

		for (int i = 0; i < 2; ++i)
		{
			auto fc = new FileChooser(i == 0 ? "Select folder contains tics samples (must be WAV audio files)" : "Select folder contains non-tics samples (must be WAV audio files)", File::nonexistent, "*", false);
			vector<string> *sampleFilesSet = i == 0 ? &ticsSampleFiles : &nonTicsSampleFiles;

			if (fc->browseForDirectory())
			{
				auto dir = fc->getResult();
				DirectoryIterator iter(dir, true, "*.wav");
				while (iter.next())
				{
					string pathname = iter.getFile().getFullPathName().toStdString();
					sampleFilesSet->push_back(pathname);
				}
				if (sampleFilesSet->empty())
				{
					NativeMessageBox::showMessageBox(AlertWindow::AlertIconType::NoIcon, "Error", "No WAV files found in this folder.");
				}
			}
			else
			{
				NativeMessageBox::showMessageBox(AlertWindow::AlertIconType::NoIcon, "Error", "Failed to load file.");
				delete fc;
				return;
			}
			delete fc;
		}

		NativeMessageBox::showMessageBox(AlertWindow::AlertIconType::NoIcon, "Training start", "About to start training. It may take a while.");
		
		doTrain.setEnabled(false);
		processor.getMainAudioProcessor()->getSoundDetector()->train(ticsSampleFiles, nonTicsSampleFiles);
	}

	if (button == &saveToFile)
	{
		MemoryBlock mb;
		processor.getStateInformation(mb);

		auto fc = new FileChooser("Save as", File::nonexistent, "*.model", true);
		if (fc->browseForFileToSave(true))
		{
			auto file = fc->getResult();
			string path = file.getFullPathName().toStdString();
			processor.getMainAudioProcessor()->getSoundDetector()->saveModel(path.c_str());
		}
		delete fc;
	}
	else if (button == &loadFromFile)
	{
		auto fc = new FileChooser("Load from", File::nonexistent, "*.model", true);
		if (fc->browseForFileToOpen())
		{
			auto file = fc->getResult();
			string path = file.getFullPathName().toStdString();
			processor.getMainAudioProcessor()->getSoundDetector()->loadModel(path.c_str());
		}
		else
		{
			NativeMessageBox::showMessageBox(AlertWindow::AlertIconType::NoIcon, "Error", "Failed to load file.");
		}
		delete fc;
	}
}
