#define FILTER_SIZE 16
#define POT_SAMPLE_RATE 4
#define POT_PIN A10

//Change these values based on minimum and maximum resistance of control potentiometer
#define POT_MIN 6600.0
#define POT_MAX 65536.0

//ADC reading below which potentiometer circuit is probably open
#define POT_ON_THRESHOLD 200

//minimum and maximum osc frequencies
#define FREQ_MIN 200.0
#define FREQ_MAX 5000.0

//potentiometer values details (w/ 1k pulldown):
//with test setup: minimum reading = 6600; maximum reading: 65536 (range: 58936)

//constant multiplier maps potentiometer position to frequency range
const float pot_freq_ratio = (FREQ_MAX-FREQ_MIN) / (POT_MAX-POT_MIN);

//constant offset for lowest possible potentiometer value
const int pot_offset = POT_MIN * pot_freq_ratio - FREQ_MIN;
static int playing = 0;

//ring buffer containing samples in moving average
static int avgs[FILTER_SIZE];
//pointer offset of oldest buffer contents
static int avg_ind = 0;

//time between cycles
static int dt;

//most recent value from potentiometer
static int pot_val;

AudioControlSGTL5000 codec;

void setup(){
  pinMode(POT_PIN, INPUT);
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

float pot_to_freq(){
  int in = 0;
  int good = FILTER_SIZE;
  for (int n = 0; n < FILTER_SIZE; n++){
    if (avgs[n] < POT_ON_THRESHOLD) {
      good-=1;
    }else in += avgs[n];
  }
  in /= good;

  float freq = in * pot_freq_ratio - pot_offset;
  return freq;
}

void loop(){
  dt = millis();
  //if button is currently pressed
  if (playing == 1){
    if (pot_val < POT_ON_THRESHOLD){
      //if button is released
      envelope1.noteOff();
      playing = 0;
    }else{
      //if button is held down, update frequency with moving average
      waveform1.frequency(pot_to_freq());
    }
  }else{
    //button pressed
    if (pot_val > POT_ON_THRESHOLD){
        AudioNoInterrupts();
        waveform1.begin(0.2, pot_to_freq(), WAVEFORM_PULSE);
        waveform1.pulseWidth(0.3);
        envelope1.noteOn();
        AudioInterrupts();
        playing = 1;
    }
  }
  pot_val = analogRead(POT_PIN);
  avgs[avg_ind] = pot_val;
  avg_ind = (avg_ind + 1) % FILTER_SIZE;
  while (millis() - dt < POT_SAMPLE_RATE) {}
}
