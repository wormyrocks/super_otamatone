#include "MenuSystem.h"
#include "Synth.h"
#include "images.h"
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 oled(4);
// pin values
#define POT_PIN 20 // neck potentiometer reading
#define VOLUME_PIN 23 // volume potentiometer reading
#define BUTTON_LEFT 22 // left button
#define BUTTON_RIGHT 21 // right button
#define POLL_NECK_MICROS 6000 // interval between resistor polls (us)
#define POLL_VOLUME_MICROS 50000 // interval between volume pot polls (us)
#define EVENT_LOOP_DT 10 // time between loop() calls (ms)
// analogRead resolution
#define ADC_RES 16
#define RMIN 17 // minimum neck resistance, in kOhms
#define RMAX 195 // maximum neck resistance, in kOhms
#define PULL 62 // size of pull resistor, in kOhms
// values pertaining to ADC resolution
static const uint16_t max_adc = pow(2, ADC_RES) - 1;
static const uint16_t thresh_adc = max_adc / 10;
static const uint32_t neckscale = max_adc * PULL;
static const uint16_t neckscale_2 = max_adc / (RMAX - RMIN);
static uint16_t dt;
volatile uint16_t _pot_val;
static uint16_t pot_val;
static bool oto_tune_on = false;
static bool scrolling = false;
static bool select_on_release = false;
static bool edit_mode = false;
static bool new_vol = false;
// GUItool: begin automatically generated code
AudioSynthWaveform waveform1; // xy=464.2857360839844,727.8571605682373
AudioEffectEnvelope envelope1; // xy=701.2857360839844,635.8571605682373
AudioMixer4 mixer1; // xy=883,801
AudioOutputAnalog dac1; // xy=1017.1428642272949,667.1428813934326
AudioConnection patchCord1(waveform1, envelope1);
AudioConnection patchCord2(envelope1, 0, mixer1, 0);
AudioConnection patchCord3(mixer1, dac1);
// GUItool: end automatically generated code
Synth synth1(max_adc, & mixer1, & waveform1, & envelope1);
IntervalTimer neck_read_int;
IntervalTimer vol_read_int;
Bounce debounce_1 = Bounce();
Bounce debounce_2 = Bounce();
#include "mymenu.h"
void read_volume()
{
}

void button_1_pressed()
{
  if (edit_mode)
  {
    ms.display();
  }
}

void button_1_released()
{
  if (!edit_mode)
  {
    if (!synth1.playing)
    {
      synth1.change_octave(-1);
      disp_update_non_menu();
    }
  } else {
    ms.back();
    ms.display();
  }
}

void button_2_pressed()
{
  if (edit_mode)
  {
    synth1.mute(true);
    scrolling = true;
    select_on_release = true;
  }
}

void button_2_released()
{
  if (edit_mode)
  {
    synth1.mute(false);
    scrolling = false;
    if (select_on_release)
    {
      ms.select();
      ms.display();
    }
  }
  else
  {
    if (!synth1.playing)
    {
      synth1.change_octave(1);
      disp_update_non_menu();
    }
  }
}

void read_scale_neck()
{
  // resistance of neck, accounting for voltage division: PULL * (max_adc-N)/N
  _pot_val = analogRead(POT_PIN);
  if (_pot_val < thresh_adc)
    _pot_val = 0;
  else
  {
    _pot_val = (neckscale / _pot_val - PULL - RMIN) * neckscale_2;
  }
}

void setup()
{
  AudioMemory(18);
  pinMode(BUTTON_LEFT, INPUT);
  pinMode(BUTTON_RIGHT, INPUT);
  pinMode(POT_PIN, INPUT);
  pinMode(VOLUME_PIN, INPUT);
  // enter edit mode if both buttons are down at startup
  if (digitalRead(BUTTON_LEFT) == 0 && digitalRead(BUTTON_RIGHT) == 0)
  {
    edit_mode = 1;
  }
  edit_mode = 1;
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.clearDisplay();
  dac1.analogReference(INTERNAL);
  delay(50); // time for DAC voltage stable
  // pinMode(5, OUTPUT);
  // digitalWrite(5, HIGH); // turn on the amplifier
  // delay(10); // allow time to wake up
  debounce_1.attach(BUTTON_LEFT);
  debounce_2.attach(BUTTON_RIGHT);
  debounce_1.interval(5);
  debounce_2.interval(5);
  analogReadResolution(ADC_RES);
  neck_read_int.begin(read_scale_neck, POLL_NECK_MICROS);
  vol_read_int.begin(read_volume, POLL_VOLUME_MICROS);
  synth1.begin();
  presets[0].en = true;
  preset_menu_shown = presets[0].en;
  populate_menus();
  if (edit_mode)
  {
    ms.display();
  }
  else
  {
    oled.drawBitmap(0, 0, otamatone, 128, 64, 1);
    oled.display();
    delay(2000);
    disp_setup();
    disp_update_non_menu();
  }
}

void loop()
{
  debounce_1.update();
  debounce_2.update();
  dt = millis();
  noInterrupts();
  pot_val = _pot_val;
  interrupts();
  synth1.update(pot_val);
  // if (synth1.playing) Serial.println(synth1.get_freq());
  if (debounce_1.risingEdge())
    button_1_released();
  else if (debounce_1.fallingEdge())
    button_1_pressed();
  if (debounce_2.risingEdge())
    button_2_released();
  else if (debounce_2.fallingEdge())
    button_2_pressed();
  update_menu();
  while (millis() - dt < EVENT_LOOP_DT)
  {
  }
}
