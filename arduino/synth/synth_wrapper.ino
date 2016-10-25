#define FILTER_SIZE 10
#define POT_SAMPLE_RATE 10
#define POT_PIN A10

//Change these values based on minimum and maximum resistance of control potentiometer
#define POT_MIN 6600
#define POT_MAX 65536

#define FREQ_MIN 20
#define FREQ_MAX 12500

//potentiometer values details (w/ 1k pulldown):
//with test setup: minimum reading = 6600; maximum reading: 65536 (range: 58936)

//constant multiplier maps potentiometer position to frequency range
const int pot_freq_ratio = (POT_MAX-POT_MIN) / (FREQ_MAX-FREQ_MIN);

//constant offset for lowest possible potentiometer value
const int pot_offset = POT_MIN * pot_freq_ratio - FREQ_MIN;
static int playing = 0;

//ring buffer containing samples in moving average
static int avgs[FILTER_SIZE];
//pointer offset of oldest buffer contents
static int avg_ind = 0;

//time between cycles
static int dt;

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
  envelope1.attack(9.2);
  envelope1.hold(2.1);
  envelope1.decay(31.4);
  envelope1.sustain(0.6);
  envelope1.release(84.5);
}

float pot_to_freq(){
  int in = 0;
  for (int n = 0; n < FILTER_SIZE-1; n++){
    in += avgs[n];
  }
  in /= FILTER_SIZE;
  float freq = in * pot_freq_ratio - pot_offset;
  return freq;
}

void loop(){
  dt = millis();
  if (playing==1){
    if (analogRead(A10) < 200){
      //button released
      envelope1.noteOff();
      playing = 0;
    }else{  
      waveform1.frequency(pot_to_freq());
    }
  }else{
    //button pressed
    if (analogRead(A10) > 200){
        AudioNoInterrupts();
        waveform1.begin(0.2, pot_to_freq(), WAVEFORM_SINE);
        envelope1.noteOn();
        AudioInterrupts();
        playing = 1;
    }
  }
  avgs[avg_ind] = analogRead(POT_PIN);
  avg_ind = (avg_ind + 1) % FILTER_SIZE;
  while (millis() - dt < POT_SAMPLE_RATE) {}
}

