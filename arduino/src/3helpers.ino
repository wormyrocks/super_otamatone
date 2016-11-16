int pot_to_freq() {
  int in = 0;
  int good = FILTER_SIZE;
  noInterrupts();
  for (int n = 0; n < FILTER_SIZE; n++) {
    if (avgs[n] == 0) {
      good -= 1;
    } else in += avgs[n];
  }
  interrupts();
  in /= good;
  int freq = 1.0/(in / (1.0*max_adc) * a_coeff + b_coeff);
  if (oto_tune_on) return oto_tune(freq);
  return freq;
}


void read_volume(){
  vol = analogRead(VOLUME_PIN);
  new_vol = true;
}

void disp_setup(){
  display.setTextSize(2);
  display.setTextColor(WHITE);
}

void disp_update_non_menu(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Octave: ");
  if (octave >= 0) display.print("+");
  display.println(octave);
  display.display();
}

void change_volume(){
  float a = ((vol & 0xff00) / (1.0*(max_adc & 0xff00)));
  codec.dacVolume(a);
  new_vol = false;
}

void change_octave(int oct){
  octave += oct;
  if (octave <= MAX_OCT && octave >= MIN_OCT){
    freq_min *= (pow(2,oct));
    freq_max *= (pow(2,oct));
    //coefficient for inverse freq scaling
    a_coeff = 1.0/freq_min - 1.0/freq_max;
    b_coeff = 1.0/freq_max;
  }else octave -= oct;
}

int oto_tune(int f_orig){
  int freq;
  freq = round(log(f_orig/a_tune)/log_a_exp);
  freq = a_tune * exp(freq * log_a_exp);
  return freq;
}