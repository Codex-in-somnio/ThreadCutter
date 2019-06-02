# ThreadCutter

A VST plugin that automatically detects and cuts unwanted sound.

## How to use

### Installation

Get the newest binary from [Releases](https://github.com/k9yyy/ThreadCutter/releases), pick an appropriate version, and load it into your VST host.

### Basic operation

After plugin is enabled:

1. Select an  approximately 100-millisecond long noise sample, and play it in looped mode. While the sample is playing, click "Capture sample *N*" button to capture it into the plugin.

2. Check the "Enabled" checkbox to enable the captured sample for sound detection.

3. Adjust the threshold slider so that only close-enough matches triggers mute.

Note: Currently it is only best optimized for 44100 Hz sample rate.

### Save and load

In addition to saving and loading as presets in your VST host, ThreadCutter also allows you to save to and load from JSON formatted files. Therefore you can save and load across different VST hosts.

## How it works

(To be improved...)

* Matching score is calculated based on MFCC distance and spectrum power product sum.

* When multiple samples are used, largest score will be the final score.

* When the final score surpasses the threshold set by user, mute is triggered. 

## Build from source

Toolchain: Microsoft Visual C++ 2017 (with Visual Studio)

Framework: JUCE (Version 5.4.3)

External libraries used:

* `picojson.h`: <https://github.com/kazuho/picojson>
* `spline.h`: <https://kluge.in-chemnitz.de/opensource/spline/>

* `FftComplex.hpp`, `FftComplex.cpp`, `FftRealPair.hpp`,  `FftRealPair.cpp`: <https://www.nayuki.io/page/free-small-fft-in-multiple-languages>

* `FastDctFft.hpp`, `FastDctFft.cpp`: <https://www.nayuki.io/page/fast-discrete-cosine-transform-algorithms>

  (These goes into `ExternalCode/`)

How to build: Generate the Visual Studio project from `ThreadCutter.jucer` using ProJucer, get the external libraries, and build.

## To-do

* Add more adjustable parameters to GUI;
* Performance tuning;
* Clean-up code.