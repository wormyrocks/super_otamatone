#include "Synth.h"

Synth::Synth(uint16_t _max_adc, AudioMixer4 *_mixer1, AudioSynthWaveform * _waveform1, AudioEffectEnvelope * _envelope1)
{
  max_adc = _max_adc;
  thresh_adc = max_adc/10;
  avg_ind = 0;
  freq_min = FREQ_MIN_INIT;
  freq_max = FREQ_MAX_INIT;
  a_exp = pow(2, 1/12.0);
  log_a_exp = log(2) /12.0;
  a_tune = 440;
  octave = 0;
  freq = 0;
  oto_tune_on = 0;
  playing = 0;
  envelope1 = _envelope1;
  waveform1 = _waveform1;
  mixer1 = _mixer1;
}

void Synth::begin()
{
  mixer1 -> gain(1, mixer1_gain);
  envelope1 -> attack(9.2);
  envelope1 -> hold(2.1);
  envelope1 -> decay(31.4);
  envelope1 -> sustain(.6);
  envelope1 -> release(84.5);
  waveform1 -> pulseWidth(.5);
  waveform1 -> begin(.8, 1000, WAVEFORM_SQUARE);
  // envelope1->noteOn();
  change_octave(0);
}

void Synth::change_octave(int _oct)
{
  octave += _oct;
  if (octave <= MAX_OCT && octave >= MIN_OCT)
  {
    freq_min *= (pow(2, _oct));
    freq_max *= (pow(2, _oct));
    // coefficient for inverse freq scaling
    a_coeff = 1.0/freq_min - 1.0/freq_max;
    b_coeff = 1.0/freq_max;
  }
  else
    octave -= _oct;
}

void Synth::mute(bool do_mute){
  if (do_mute) mixer1 -> gain(1, 0);
  else mixer1 -> gain(1, mixer1_gain);
}

void Synth::stop(){
  // if neck is released
  envelope1 -> noteOff();
  playing = 0;
}

void Synth::update(uint16_t sample)
{
  avgs[avg_ind] = sample;
  avg_ind = (avg_ind + 1) % FILTER_SIZE;
  // if neck is currently pressed
  if (playing)
  {
    if (sample == 0)
    {
      stop();
    }
    else
    {
      // if neck is held down, update frequency with moving average
      pot_to_freq();
      waveform1 -> frequency(freq);
    }
  }
  else
  {
    // neck pressed
    if (sample != 0)
    {
      pot_to_freq();
      AudioNoInterrupts();
      waveform1 -> frequency(freq);
      envelope1 -> noteOn();
      AudioInterrupts();
      playing = 1;
    }
  }
}

uint16_t Synth::get_freq()
{
  return freq;
}

int Synth::get_octave()
{
  return octave;
}

// TODO: fix rounding issues, convert to median filter
void Synth::pot_to_freq()
{
  uint32_t in = 0;
  uint8_t good = FILTER_SIZE;
  noInterrupts();
  for (uint16_t n = 0; n < FILTER_SIZE; n++)
  {
    if (avgs[n] == 0)
    {
      good -= 1;
    }
    else
      in += avgs[n];
  }
  interrupts();
  in /= good;
  freq = max_adc / (in * a_coeff + max_adc * b_coeff);
  // if (oto_tune_on) freq = oto_tune(freq);
}

void Synth::toggle_oto_tune()
{
  oto_tune_on = !oto_tune_on;
}

uint16_t Synth::oto_tune(uint16_t f_orig)
{
  uint16_t freq;
  freq = round(log(f_orig/a_tune) /log_a_exp);
  freq = a_tune * exp(freq * log_a_exp);
  return freq;
}
