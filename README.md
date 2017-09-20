# arduino-midi-drums
Self-set task to achieve the cheapest possible working DYI electronic drumset for home recording.

Code based on evankale's [project](https://github.com/evankale/ArduinoMidiDrums) with added support for the arduino MIDI library
and the optically triggered hi-hat.

Software used for mapping the sound: LMMS running under Ubuntu 16.04.3 with EZDrummer2 VST (loaded trought VeSTige)

Notes: current version uses [this](https://www.amazon.co.uk/Proster-MIDI-Interface-Cable-Adapter-2M-USB/dp/B00EYYWGRI/ref=sr_1_3?ie=UTF8&qid=1505928062&sr=8-3&keywords=midi+interface) MIDI interface which proved to result in latency (5.5ms) that even with a linux real-time kernel is not very pleasant.
