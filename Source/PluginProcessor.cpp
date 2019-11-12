#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ThreadCutterAudioProcessor::ThreadCutterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", AudioChannelSet::stereo(), true)
#endif
	)
#endif
{
}

ThreadCutterAudioProcessor::~ThreadCutterAudioProcessor()
{
}

//==============================================================================
const String ThreadCutterAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool ThreadCutterAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool ThreadCutterAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool ThreadCutterAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double ThreadCutterAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int ThreadCutterAudioProcessor::getNumPrograms()
{
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
				// so this should be at least 1, even if you're not really implementing programs.
}

int ThreadCutterAudioProcessor::getCurrentProgram()
{
	return 0;
}

void ThreadCutterAudioProcessor::setCurrentProgram(int index)
{
}

const String ThreadCutterAudioProcessor::getProgramName(int index)
{
	return {};
}

void ThreadCutterAudioProcessor::changeProgramName(int index, const String& newName)
{
}

//==============================================================================
void ThreadCutterAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	// Use this method as the place to do any pre-playback
	// initialisation that you need..

	int frameSize = (int)round(sampleRate * 0.25);
	processor[0].setFrameSize(frameSize);
	processor[1].setFrameSize(frameSize);
	processor[0].getSoundDetector()->setSampRate((int)round(sampleRate));

	processor[0].setDoDetection(true);
	processor[1].setGetMuteTimeFrom(&processor[0]);
	
	setLatencySamples(frameSize * 2);
}

void ThreadCutterAudioProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ThreadCutterAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
	ignoreUnused(layouts);
	return true;
#else
	// This is the place where you check if the layout is supported.
	// In this template code we only support mono or stereo.
	if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
		&& layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
		return false;

	// This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
#endif

	return true;
#endif
}
#endif

void ThreadCutterAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
	ScopedNoDenormals noDenormals;
	auto totalNumInputChannels = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();

	// In case we have more outputs than inputs, this code clears any output
	// channels that didn't contain input data, (because these aren't
	// guaranteed to be empty - they may contain garbage).
	// This is here to avoid people getting screaming feedback
	// when they first compile a plugin, but obviously you don't need to keep
	// this code if your algorithm always overwrites all the output channels.
	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear(i, 0, buffer.getNumSamples());

	// This is the place where you'd normally do the guts of your plugin's
	// audio processing...
	// Make sure to reset the state if your inner loop is processing
	// the samples and the outer loop is handling the channels.
	// Alternatively, you can process the samples with the channels
	// interleaved by keeping the same state.

	int nSamples = buffer.getNumSamples();

	for (int channel = 0; channel < totalNumInputChannels; ++channel)
	{
		auto* channelData = buffer.getWritePointer(channel);

		std::vector<double> samples(nSamples);

		for (int i = 0; i < nSamples; ++i)
			samples[i] = (double)channelData[i];

		processor[channel].addSamples(samples);
		std::vector<double> processedSamples = processor[channel].getSamples(nSamples);

		for (int i = 0; i < nSamples; ++i)
			channelData[i] = (float)processedSamples[i];

		if (channel == 0)
		{
			currentMfccScoreDisplay = processor[channel].getCurrentMfccScore();
			currentAvgPeakLevelDisplay = processor[channel].getCurrentAvgPeakLevel();
			currentPeakLevelDisplay = processor[channel].getCurrentPeakLevel();
		}
	}
}

//==============================================================================
bool ThreadCutterAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ThreadCutterAudioProcessor::createEditor()
{
	editor = new ThreadCutterAudioProcessorEditor(*this);
	setEditorValues();
	return editor;
}

//==============================================================================
void ThreadCutterAudioProcessor::getStateInformation(MemoryBlock& destData)
{
	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.
	std::string curState = processor[0].dumpCurrentState();
	destData.replaceWith(curState.c_str(), curState.size() + 1);
}

void ThreadCutterAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	// You should use this method to restore your parameters from this memory block,
	// whose contents will have been created by the getStateInformation() call.
	std::string state = (char *)data;
	processor[0].loadState(state);
	if (editor != nullptr)
		setEditorValues();
}

Processor * ThreadCutterAudioProcessor::getMainAudioProcessor()
{
	return &processor[0];
}

void ThreadCutterAudioProcessor::setEditorValues()
{
	ThreadCutterAudioProcessorEditor *_editor = (ThreadCutterAudioProcessorEditor *)editor;
//	_editor->setThresholdSliderValue(processor[0].getThreshold());
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new ThreadCutterAudioProcessor();
}
