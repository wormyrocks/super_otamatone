#include <Bounce2.h>
#include <math.h>

#define OLED_RESET 4

#define POT_PIN A10
#define VOLUME_PIN 15
#define BUTTON_LEFT 0
#define BUTTON_RIGHT 1

#define RMIN 23     // minimum neck resistance, in kOhms
#define RMAX 190    // maximum neck resistance, in kOhms
#define PULL 62     // size of pull resistor, in kOhms
#define POLL_NECK_MICROS 6000 // interval between resistor polls (us)
#define POLL_VOLUME_MICROS 50000 // interval between volume pot polls (us)
#define EVENT_LOOP_DT 10 // time between loop() calls (ms)
#define FILTER_SIZE 8  // size of moving average buffer

//minimum and maximum osc frequencies
#define FREQ_MIN_INIT 160.0
#define FREQ_MAX_INIT 1450.0

// ***** flags controlling overall state machine ***** //
static bool oto_tune_on = false;
static bool playing = false;
static bool scrolling = false;
static bool select_on_release = false;
static bool edit_mode = false;
static bool new_vol = false;

//ring buffer containing samples in moving average
static int avgs[FILTER_SIZE];

//pointer offset of oldest buffer contents
static int avg_ind = 0;

//time between cycles
static int dt;

//most recent value from potentiometer
static int pot_val;
//current volume
static int vol;

//minimum and maximum frequencies
static float freq_min = FREQ_MIN_INIT;
static float freq_max = FREQ_MAX_INIT;

//constant for equal temperament
static const float a_exp = pow(2, 1/12.0);
static const float log_a_exp = log(2)/12.0;
static const float a_tune = 440;

//current octave
static int octave = 0;
#define MAX_OCT 2
#define MIN_OCT -2

//neck scaling coefficients
static int neckscale = 65536 * PULL;
static int neckscale_2 = 65536 / (RMAX-RMIN);

//frequency coefficients
static float a_coeff;
static float b_coeff;
  
AudioControlSGTL5000 codec;
IntervalTimer neck_read_int;
IntervalTimer vol_read_int;

Bounce debounce_1 = Bounce();
Bounce debounce_2 = Bounce();

int oto_tune(int f_orig){
  int freq;
  freq = round(log(f_orig/a_tune)/log_a_exp);
  freq = a_tune * exp(freq * log_a_exp);
  return freq;
}

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
  int freq = 1.0/(a_coeff * in / 65536.0 + b_coeff);
  if (oto_tune_on) return oto_tune(freq);
  return freq;
}

void read_scale_neck() {
  // resistance of neck, accounting for voltage division: PULL * (65536-N)/N
  pot_val = analogRead(A10);
  if (pot_val < 6000) pot_val = 0;
  else {
    pot_val = (neckscale/pot_val-PULL-RMIN)*neckscale_2;
  } 
  avgs[avg_ind] = pot_val;
  avg_ind = (avg_ind + 1) % FILTER_SIZE; 
}

void read_volume(){
  vol = analogRead(VOLUME_PIN);
  new_vol = true;
}

void change_volume(){
  float a = (vol & 0xff00)/65280.0;
  codec.dacVolume(a);
  new_vol = false;
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

void setup() {
  pinMode(BUTTON_LEFT, INPUT);
  pinMode(BUTTON_RIGHT, INPUT);
  pinMode(POT_PIN, INPUT);
  pinMode(VOLUME_PIN, INPUT);
  
  //enter edit mode if both buttons are down at startup
  if (digitalRead(BUTTON_LEFT) == 1 && digitalRead(BUTTON_RIGHT) == 1){
    edit_mode = 1;
  }
  debounce_1.attach(BUTTON_LEFT);
  debounce_2.attach(BUTTON_RIGHT);
  debounce_1.interval(5);
  debounce_2.interval(5);

  analogReadResolution(16);

  AudioMemory(20);
  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();
  codec.enable();
  neck_read_int.begin(read_scale_neck, POLL_NECK_MICROS);
  vol_read_int.begin(read_volume, POLL_VOLUME_MICROS);
  codec.lineInLevel(0);
  codec.lineOutLevel(31);
  envelope1.attack(5);
  envelope1.hold(30);
  envelope1.decay(30);
  envelope1.sustain(.5);
  envelope1.release(50);
  mixer1.gain(1,.5);
  mixer1.gain(2,.5);
  mixer2.gain(1,.5);
  mixer2.gain(2,.5);
  
  change_octave(octave);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  ms.get_root_menu().add_item(&mm_mi1);
  ms.get_root_menu().add_item(&mm_mi2);
  ms.get_root_menu().add_menu(&mu1);
  mu1.add_item(&mu1_mi1);
  if (edit_mode){
    ms.display();
  }else{
    disp_setup();
    disp_update_non_menu();
  }
  
  waveform1.begin(1, pot_to_freq(), WAVEFORM_SQUARE);
  waveform1.pulseWidth(0.5);
}

void button_1_pressed(){
  if (edit_mode){
    ms.back();
    ms.display();
  }
}

void button_1_released(){
  if (edit_mode){
    
  }else{
    if (!playing){
      change_octave(-1);
      disp_update_non_menu();
    }
  }
}

void button_2_pressed(){
  if (edit_mode){
    scrolling=true;
    select_on_release=true;
  }
}

void button_2_released(){
  if (edit_mode){
    scrolling=false;
    if (select_on_release){
      ms.select();
      ms.display();
    }
  }else{
    if (!playing){
      change_octave(1);
      disp_update_non_menu();
    }
  }
}

void loop() {
  debounce_1.update();
  debounce_2.update();
  
  dt = millis();
  noInterrupts();
  int pv = pot_val;
  interrupts();
  
  if (scrolling){
    if (playing){
      envelope1.noteOff();
      playing = false;
    }
    if (pv != 0){
      Menu const* m = ms.get_current_menu();
      int index = (65536-pv) * m->get_num_components() / 65536;
      if (index > m->get_current_component_num()){
        ms.next();
        ms.display();
      }else if (index < m->get_current_component_num()){
        ms.prev();
        ms.display();
      }
      select_on_release = false;
    } 
  }
  //if neck is currently pressed
  else{
    if (playing) {
      if (pv == 0) {
        //if neck is released
        envelope1.noteOff();
        playing=false;
      } else {
        //if neck is held down, update frequency with moving average
        waveform1.frequency(pot_to_freq());
      }
    } else {
      //neck pressed
      if (pv != 0) {
        AudioNoInterrupts(); 
        waveform1.frequency(pot_to_freq());
        envelope1.noteOn();
        AudioInterrupts();
        playing=true;;
      }
    }
  }

  if (debounce_1.risingEdge()) button_1_pressed();
  else if (debounce_1.fallingEdge()) button_1_released();
  if (debounce_2.risingEdge()) button_2_pressed();
  else if (debounce_2.fallingEdge()) button_2_released();
  
  if (new_vol) change_volume();
  
  while (millis() - dt < EVENT_LOOP_DT) {}
}
