#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=237,463
AudioEffectEnvelope      envelope1;      //xy=474,371
AudioInputI2S            i2s2;           //xy=499,248
AudioMixer4              mixer2;         //xy=663,403
AudioMixer4              mixer1;         //xy=667,263
AudioOutputI2S           i2s1;           //xy=881,339
AudioConnection          patchCord1(waveform1, envelope1);
AudioConnection          patchCord2(envelope1, 0, mixer2, 0);
AudioConnection          patchCord3(envelope1, 0, mixer1, 0);
AudioConnection          patchCord4(i2s2, 0, mixer1, 1);
AudioConnection          patchCord5(i2s2, 1, mixer2, 1);
AudioConnection          patchCord6(mixer2, 0, i2s1, 1);
AudioConnection          patchCord7(mixer1, 0, i2s1, 0);
// GUItool: end automatically generated code

