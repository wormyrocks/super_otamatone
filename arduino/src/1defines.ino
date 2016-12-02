#include <MenuSystem.h>
#include <Adafruit_SSD1306.h>
#include <Bounce2.h>
#include <math.h>

//pin values
#define OLED_RESET 4              //unused
#define POT_PIN A10               //neck potentiometer reading
#define VOLUME_PIN 15             //volume potentiometer reading
#define BUTTON_LEFT 0             //left button
#define BUTTON_RIGHT 1            //right button

#define RMIN 23                   // minimum neck resistance, in kOhms
#define RMAX 190                  // maximum neck resistance, in kOhms
#define PULL 62                   // size of pull resistor, in kOhms
#define POLL_NECK_MICROS 6000     // interval between resistor polls (us)
#define POLL_VOLUME_MICROS 50000  // interval between volume pot polls (us)
#define EVENT_LOOP_DT 10          // time between loop() calls (ms)
#define FILTER_SIZE 8             // size of moving average buffer

//minimum and maximum frequencies in default range
#define FREQ_MIN_INIT 160.0 
#define FREQ_MAX_INIT 1450.0

//analogRead resolution
#define ADC_RES 16

#define MAX_OCT 2
#define MIN_OCT -2

//values pertaining to ADC resolution
static const int max_adc = pow(2,ADC_RES);
static const int thresh_adc = max_adc/10;

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

//neck scaling coefficients
static int neckscale = max_adc * PULL;
static int neckscale_2 = max_adc / (RMAX-RMIN);

//frequency coefficients
static float a_coeff;
static float b_coeff;
  
AudioControlSGTL5000 codec;
IntervalTimer neck_read_int;
IntervalTimer vol_read_int;

Bounce debounce_1 = Bounce();
Bounce debounce_2 = Bounce();

//function prototypes

int oto_tune(int freq);
int pot_to_freq();
void read_volume();
void disp_setup();
void disp_update_non_menu();
void change_volume();
void change_octave(int oct);
int oto_tune(int f_orig);

//function prototypes for callbacks
void button_1_pressed();
void button_1_released();
void button_2_pressed();
void button_2_released();
void read_scale_neck();
