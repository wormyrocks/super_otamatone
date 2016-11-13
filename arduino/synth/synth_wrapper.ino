#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Bounce2.h>

#define OLED_RESET 4

#define POT_PIN A10
#define BUTTON_LEFT 0
#define BUTTON_RIGHT 1

#define RMIN 23     // minimum neck resistance, in kOhms
#define RMAX 190    // maximum neck resistance, in kOhms
#define PULL 62     // size of pull resistor, in kOhms
#define POLL_NECK_MICROS 8000 // interval between resistor polls (us)
#define EVENT_LOOP_DT 10 // time between loop() calls (ms)
#define FILTER_SIZE 8  // size of moving average buffer

//minimum and maximum osc frequencies
#define FREQ_MIN_INIT 160.0
#define FREQ_MAX_INIT 1447.0

static int playing = 0;

//ring buffer containing samples in moving average
static int avgs[FILTER_SIZE];

//pointer offset of oldest buffer contents
static int avg_ind = 0;

//time between cycles
static int dt;

//most recent value from potentiometer
static int pot_val;

//minimum and maximum frequencies
static float freq_min = FREQ_MIN_INIT;
static float freq_max = FREQ_MAX_INIT;

//current octave
static int octave = 0;
#define MAX_OCT 3
#define MIN_OCT -3
static const int min_octave = -2;
static const int max_octave = 2;

//neck scaling coefficients
static int neckscale = 65536 * PULL;
static int neckscale_2 = 65536 / (RMAX-RMIN);

//frequency coefficients
static float a_coeff;
static float b_coeff;
  
Adafruit_SSD1306 display(OLED_RESET);
AudioControlSGTL5000 codec;
IntervalTimer pot_read_int;

Bounce debounce_1 = Bounce();
Bounce debounce_2 = Bounce();

void setup() {
  pinMode(BUTTON_LEFT, INPUT);
  pinMode(BUTTON_RIGHT, INPUT);
  debounce_1.attach(BUTTON_LEFT);
  debounce_2.attach(BUTTON_RIGHT);
  debounce_1.interval(5);
  debounce_2.interval(5);
  pinMode(POT_PIN, INPUT);
  pot_read_int.begin(read_scale_neck, POLL_NECK_MICROS);
  Serial.begin(115200);
  analogReadResolution(16);
  AudioMemory(18);
  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();
  codec.enable();
  codec.volume(0.5);
  envelope1.attack(0);
  envelope1.hold(0);
  envelope1.decay(0);
  envelope1.sustain(1);
  envelope1.release(1);
  mixer1.gain(1,2);
  mixer1.gain(2,.3);
  
  mixer2.gain(1,2);
  mixer2.gain(2,.3);
  change_octave(octave);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  disp_update();
}

void change_octave(int oct){
  octave += oct;
  if (octave < MAX_OCT && octave > MIN_OCT){
    freq_min *= (pow(2,oct));
    freq_max *= (pow(2,oct));
    //coefficient for inverse freq scaling
    a_coeff = 1.0/freq_min - 1.0/freq_max;
    b_coeff = 1.0/freq_max;
  }else octave -= oct;
}
int pot_to_freq() {
  int in = 0;
  int good = FILTER_SIZE;
//  noInterrupts();
  for (int n = 0; n < FILTER_SIZE; n++) {
    if (avgs[n] == 0) {
      good -= 1;
    } else in += avgs[n];
  }
//  interrupts();
  in /= good;
  int freq = 1.0/(a_coeff * in / 65536.0 + b_coeff);
  
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

void disp_update(){
  display.clearDisplay();
  display.setTextSize(.5);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.setTextSize(2);
  display.print("Octave: ");
  if (octave >= 0) display.print("+");
  display.println(octave);
  display.display();
}

void loop() {
  debounce_1.update();
  debounce_2.update();
  
  dt = millis();
//  noInterrupts();
  int pv = pot_val;
//  interrupts();
  //if button is currently pressed
  if (playing == 1) {
    if (pv == 0) {
      //if button is released
      envelope1.noteOff();
      playing = 0;
    } else {
      //if button is held down, update frequency with moving average
      waveform1.frequency(pot_to_freq());
    }
  } else {
      if (debounce_1.fell()){
        change_octave(-1);
        disp_update();
      }
      if (debounce_2.fell()){
        change_octave(1);
        disp_update();
      }
    //button pressed
    if (pv != 0) {
//      AudioNoInterrupts(); 
      waveform1.begin(0.4, pot_to_freq(), WAVEFORM_SAWTOOTH);
      waveform1.pulseWidth(0.3);
      envelope1.noteOn();
//      AudioInterrupts();
      playing = 1;
    }
  }
  while (millis() - dt < EVENT_LOOP_DT) {}
}
