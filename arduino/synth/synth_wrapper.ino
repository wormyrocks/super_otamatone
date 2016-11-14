#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Bounce2.h>

#define OLED_RESET 4

#define POT_PIN A10
#define BUTTON_LEFT 0
#define BUTTON_RIGHT 1

#define RMIN 23     // minimum neck resistance, in kOhms
#define RMAX 190    // maximum neck resistance, in kOhms
#define PULL 62     // size of pull resistor, in kOhms
#define POLL_NECK_MICROS 6000 // interval between resistor polls (us)
#define EVENT_LOOP_DT 10 // time between loop() calls (ms)
#define FILTER_SIZE 12  // size of moving average buffer

//minimum and maximum osc frequencies
#define FREQ_MIN_INIT 160.0
#define FREQ_MAX_INIT 1447.0

static int playing = 0;

//hold both cheeks during startup to go to patch edit mode
static uint8_t edit_mode = 0;

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
#define MAX_OCT 2
#define MIN_OCT -2

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

  //enter edit mode if both buttons are down at startup
  if (digitalRead(BUTTON_LEFT) == 1 && digitalRead(BUTTON_RIGHT) == 1){
    edit_mode = 1;
  }
  
  debounce_1.attach(BUTTON_LEFT);
  debounce_2.attach(BUTTON_RIGHT);
  debounce_1.interval(50);
  debounce_2.interval(50);
  pinMode(POT_PIN, INPUT);
  pot_read_int.begin(read_scale_neck, POLL_NECK_MICROS);
  analogReadResolution(16);
  AudioMemory(20);
  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();
  codec.enable();
  codec.volume(1);
  codec.lineOutLevel(31);
  codec.lineInLevel(0);
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
  disp_setup();
  disp_update();
  waveform1.begin(1, pot_to_freq(), WAVEFORM_SQUARE);
  waveform1.pulseWidth(0.5);
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

void disp_setup(){
  display.setTextSize(2);
  display.setTextColor(WHITE);
}

void disp_update(){
  display.clearDisplay();
  display.setCursor(0,0);
  if (edit_mode){
    display.print("Edit Mode");
  }else{
    display.print("Octave: ");
    if (octave >= 0) display.print("+");
    display.println(octave);
  }
  display.display();
}

void loop() {
  debounce_1.update();
  debounce_2.update();
  
  dt = millis();
  noInterrupts();
  int pv = pot_val;
  interrupts();
  //if button is currently pressed
  if (playing == 1) {
    if (pv == 0) {
      //if button is released
      envelope1.noteOff();
      playing = 0;
      noInterrupts();
      interrupts();
    } else {
      //if button is held down, update frequency with moving average
      waveform1.frequency(pot_to_freq());
    }
  } else {
      if (!edit_mode){
        if (debounce_1.fell()){
          change_octave(-1);
          disp_update();
        }
        if (debounce_2.fell()){
          change_octave(1);
          disp_update();
        }
      }
    //button pressed
    if (pv != 0) {
      AudioNoInterrupts(); 
      waveform1.frequency(pot_to_freq());
      envelope1.noteOn();
      AudioInterrupts();
      playing = 1;
    }
  }
  while (millis() - dt < EVENT_LOOP_DT) {}
}
