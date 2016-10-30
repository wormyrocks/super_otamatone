#define POT_PIN A10

#define RMIN 23     // minimum neck resistance, in kOhms
#define RMAX 190    // maximum neck resistance, in kOhms
#define PULL 62     // size of pull resistor, in kOhms
#define POLL_NECK_MICROS 6000 // interval between resistor polls (us)
#define EVENT_LOOP_DT 10 // time between loop() calls (ms)
#define FILTER_SIZE 16  // size of moving average buffer

//minimum and maximum osc frequencies
#define FREQ_MIN 160.0
#define FREQ_MAX 1447.0

static int playing = 0;

//ring buffer containing samples in moving average
static int avgs[FILTER_SIZE];

//pointer offset of oldest buffer contents
static int avg_ind = 0;

//time between cycles
static int dt;

//most recent value from potentiometer
static int pot_val;

//coefficient for neck scaling
static int neckscale = 65536 * PULL;
static int neckscale_2 = 65536 / (RMAX-RMIN);

AudioControlSGTL5000 codec;
IntervalTimer pot_read_int;

void setup() {
  pinMode(POT_PIN, INPUT);
  pot_read_int.begin(read_scale_neck, POLL_NECK_MICROS);
  Serial.begin(115200);
  analogReadResolution(16);
  AudioMemory(18);
  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();
  codec.enable();
  codec.volume(0.45);
  envelope1.attack(0);
  envelope1.hold(0);
  envelope1.decay(0);
  envelope1.sustain(1);
  envelope1.release(1);
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
  int freq = (1.0-in/65536.0) * (FREQ_MAX - FREQ_MIN) + FREQ_MIN;
//  Serial.printf("%d %d\n",in,freq);
  return freq;
}

void read_scale_neck() {
  // resistance of neck, accounting for voltage division: PULL * (65536-N)/N
  int a = analogRead(A10);
  if (a < 6000) a = 0;
  else a = (neckscale/a-PULL-RMIN)*neckscale_2;
  pot_val = a;
  avgs[avg_ind] = pot_val;
  avg_ind = (avg_ind + 1) % FILTER_SIZE;
}

void loop() {
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
    } else {
      //if button is held down, update frequency with moving average
      waveform1.frequency(pot_to_freq());
    }
  } else {
    //button pressed
    if (pv != 0) {
      AudioNoInterrupts(); 
      waveform1.begin(0.2, pot_to_freq(), WAVEFORM_SAWTOOTH);
      waveform1.pulseWidth(0.3);
      envelope1.noteOn();
      AudioInterrupts();
      playing = 1;
    }
  }
  while (millis() - dt < EVENT_LOOP_DT) {}
}
