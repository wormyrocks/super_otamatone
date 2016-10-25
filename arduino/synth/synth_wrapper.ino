#define FILTER_SIZE 10
#define POT_SAMPLE_RATE 10
#define INPUT_PIN A10

//potentiometer values details (w/ 1k pulldown):
//with test setup: minimum reading = 6600; maximum reading: 65536 (range: 58936)
//maximum frequency: 12500, minimum frequency: 20 (range: 12480)

int playing = 0;
int avgs[FILTER_SIZE];
int avg_ind = 0;

AudioControlSGTL5000 codec;

void setup(){
  pinMode(INPUT_PIN, INPUT);
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
  float freq = in*12480.0/58936.0-1378.0;
  return freq;
}

void loop(){
  if (playing==1){
    if (analogRead(A10) < 200){
      //button released
      envelope1.noteOff();
      playing = 0;
    }else{  
      waveform1.frequency(pot_to_freq());
    }
  }else{
    //button pushed
    if (analogRead(A10) > 200){
        AudioNoInterrupts();
        waveform1.begin(0.2, pot_to_freq(), WAVEFORM_SINE);
        envelope1.noteOn();
        AudioInterrupts();
        playing = 1;
    }else{
      
    }
  }
  delay(POT_SAMPLE_RATE);
  avgs[avg_ind] = analogRead(INPUT_PIN);
  avg_ind = (avg_ind + 1) % FILTER_SIZE; 
}

