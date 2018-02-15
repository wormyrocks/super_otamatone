#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <math.h>
#include <Bounce2.h>

#define FILTER_SIZE 7 // size of moving average buffer

//minimum and maximum frequencies in default range
#define FREQ_MIN_INIT 160.0 
#define FREQ_MAX_INIT 1450.0

#define MAX_OCT 2
#define MIN_OCT -2

class Synth;

class Synth{

public:
	Synth(uint16_t _max_adc, AudioMixer4 *_mixer1, AudioSynthWaveform *_waveform1, AudioEffectEnvelope *_envelope1);
	void update(uint16_t sample);
	void change_octave(uint8_t _oct);
	void toggle_oto_tune();
	void begin();
	void stop();
	void mute(bool do_mute);
	uint16_t get_freq();
	uint16_t get_octave();

	//whether the note is currently playing 
	bool playing;

private:
	void pot_to_freq();

	AudioSynthWaveform *waveform1;
	AudioEffectEnvelope *envelope1; 
	AudioMixer4* mixer1;

	uint16_t oto_tune(uint16_t f_orig);

	float mixer1_gain = 0.03;

	//ring buffer containing samples in moving average
	uint16_t avgs[FILTER_SIZE];

	//pointer offset of oldest buffer contents
	uint16_t avg_ind;

	//time between cycles
	uint16_t dt;

	//maximum value read from ADC
	uint16_t max_adc;

	uint16_t thresh_adc;

	//most recent value from potentiometer
	uint16_t pot_val;

	//current volume
	uint16_t vol;

	//current frequency
	uint16_t freq;

	//minimum and maximum frequencies
	float freq_min;
	float freq_max;

	//constant for equal temperament
	float a_exp;
	float log_a_exp;
	float a_tune;

	//is oto_tune enabled
	bool oto_tune_on;

	//current octave
	uint16_t octave;

	//frequency coefficients
	float a_coeff;
	float b_coeff;

};