# ThreadCutter

A VST plugin that automatically detects and cuts unwanted tics sound by using SVM.

## How to use

### Installation

1. Get the newest binary from [Releases](https://github.com/k9yyy/ThreadCutter/releases);
2. pick an appropriate version and load it into your VST host, or use the stand-alone executable version.

### Basic operation

Train a new model:

1. Prepare audio clips in WAV format (`*.wav`) and place tics and non-tics audio samples in two separate folders respectively;
2. Load the plugin and open the GUI, or start the stand-alone version;
3. In the GUI, click the "Train new model" button;
4. Select the folders containing tics and non-tics samples, and then training will start;
5. After finished, use the model right away and/or save the model by click the "save model to file" button.

Use ThreadCutter to process audio:

1. Load the plugin and open the GUI, or start the standalone version;
2. Train a new model by abovementioned steps or load a model by clicking "load model from file" button;
3. Adjust the gate slider and the detection threshold slider (detailed documentation to be added; for now, refer to "how it works" below);
4. Feed audio in and it should work.

### Save and load

Saving and loading presets on the current version has not been implemented yet. Only the model can be saved and loaded as of now.

## How it works

(To be improved...)

* The gate value is compared to the ratio of current peak level and the running average of peak levels;
* Detection is done by using MFCC for feature extraction, and SVM for classifying;
* The detection threshold value is compared to the proportion of positive matches in a frame.

## Build from source

Visual Studio 2017 project files are under `Builds/VisualStudio2017/`. For other toolchains, project files may be generated by ProJucer using `ThreadCutter.jucer`.

To build VST2 target, VST2 SDK must be separately obtained first.

## External libraries

External libraries under `ExternalCode`:

- <https://github.com/Cwiiis/c_speech_features>
- <https://github.com/cjlin1/libsvm>

## To-do

* Add more adjustable parameters to GUI;
* Performance tuning;
* Clean-up code.