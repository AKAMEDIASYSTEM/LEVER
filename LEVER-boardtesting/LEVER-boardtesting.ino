/* PCB BOARD TESTING VERSION OF THIS FIRMWARE - ONLY HAS ONE POT */

// LEVER driver for implementation of REVEL and TeslaTouch papers by Disney Research
// for teensy 3.1
// Digital pin 4 sends hi-freq PWM (altered to send at 375kHz) to a boost converter circuit
// Analog pin 14 (it's a real DAC on the teensy 3.1) sends variable-freq sine wave to a hi-voltage transistor circuit
// that modulates the boosted supply voltage
// freqPot on pin A7 sets the signal frequency
// SDA on pin A4
// SCL on pin A5
// OLED_RESET on digital pin 5

// poll freqPot pin to see what freq should be
// map 0-1023 analog in to 0.0125 (about 37.5 Hz) to 0.4 (about 1000Hz), add phase
// update OLED to show approx freq and bars representing texture
// 1000hz maps to a full bar every other row (ie offset 1)
// 30 hz maps to 64 on bars, 64 off bars (ie offset 64)

// TODO: implement goal-seeking feedback to keep constant current despite changing impedances
// TODO: figure out where in teh circuit the current-sensing occurs (pretty sure we're measuring voltage drop across precision resistor?)
// TODO: figure out how an RF interface might be used, maybe RFduino serial link or BlueSmiRF?

// #include <Adafruit_MCP4725.h> // not yet used, for external DAC if necessary
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 20
Adafruit_SSD1306 display(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

//#define amplPot 20 // A6
#define freqPot 21 // A7
#define dutyPot 22 // A8
#define wavePot 23 // A9
#define pwmOut 4
#define SINE 0
#define SQUARE 1
#define SAW_DESC 2
#define SAW_ASC 3
#define NOISE 4

char* waveLabels[] = {"SINE", "SQUARE", "SAW_DESC", "SAW_ASC", "NOISE"};

float minFreq = 0.016 * 1000.0; // 0.0125 is too low for some people to feel, trying 0.016 now
float maxFreq = 0.4 * 1000.0;

float minDuty = 0.3;
float maxDuty = 0.98;
float dutyCycle = 0.75;

float minAmpl = 0.0; // not tested
float maxAmpl = 2000.0; // not tested
float DACamplitude = 1000.0;

int waveType = 0; // sine, square, saw descending, saw ascending, triangle
#define waveMax 4
#define waveMin 0

float phase = 0.0;
float twopi = 3.14159 * 2;
float phaseOffset = 0.05;

int count = 0;
int newFreqPot = 0;
int lastFreqPot = 0; // remember last loop's reading, only update screen if freqPot is moved
int freqPotDeltaThreshold = 50; // read resolution is 10-bit (0-1023) so this is about 5%

void setup() {
  digitalWrite(13,HIGH);
  // 0 - 4095 pwm values if res set to 12-bit
  analogWriteResolution(12);
  analogWriteFrequency(pwmOut, 375000);
  pinMode(pwmOut, OUTPUT);
  pinMode(freqPot, INPUT); // frequency adjust pot
//  pinMode(dutyPot, INPUT); // eventually this will be replaced by calculateFeedback()
//  pinMode(amplPot, INPUT); // manually adjust output DAC amplitude, to see if this should be calculateFeedback() modulated
//  pinMode(wavePot, INPUT); // select between waveforms (how to display current waveform?)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.display();
digitalWrite(13,LOW);
}

void loop() {

  //  dutyCycle = constrain(floatmap(analogRead(dutyPot), 0.0, 1023.0, minDuty, maxDuty), minDuty, maxDuty);
  dutyCycle = 0.5; // BOARDTESTING: hardcode duty cycle, yields about 180v from boost circuit
  analogWrite(pwmOut, int(4096 * dutyCycle)); // duty cycle should have been dynamically calculated before here

  //  DACamplitude = constrain(floatmap(analogRead(amplPot), 0.0, 1023.0, minAmpl, maxAmpl), minAmpl, maxAmpl);
  DACamplitude = 2000.0; // BOARDTESTING: hardcode max amplitude

  newFreqPot = analogRead(freqPot);
  if ( abs(newFreqPot - lastFreqPot) > freqPotDeltaThreshold) {
    lastFreqPot = newFreqPot;
    updateDisplay();
  }
  // calculate and update the phase accumulator
  phaseOffset = (4 * phaseOffset + constrain(map(newFreqPot, 0, 1023, minFreq, maxFreq), minFreq, maxFreq)) / 5;
  phase = phase + (phaseOffset / 1000.0);
  if (phase >= twopi) {
    phase = 0;
  }
  //  phase = phase + 0.2; // about 600Hz
  //  phase = phase + 0.05; // about 150Hz
  //  phase = phase + 0.025; // about 75Hz

  calculateFeedback(); // this doesn't exist yet
  /* I think: we measure the current going across a precision resistor (at high voltage, so: how?! isolation amp?)
   * Then we see how far under/over target we are, and proportionally adjust duty cycle
   * (higher duty cycle, i think, means higher voltage but make sure it's not going into saturation)
   *
   */

  //  waveType = constrain(map(analogRead(wavePot), 0, 1023, waveMin, waveMax), waveMin, waveMax);
  waveType = 1; // BOARDTESTING: hardcode for square now
  float DACval = 0;
  switch (waveType) {
    case SINE:
//      DACval = sin(phase) * DACamplitude + 2050.0; // amplitude adjustment should occur here
      //  float sineVal = sin(phase) * 2000.0 + 2050.0; // amplitude adjustment should occur here
      DACval = sin(phase) * 2000.0 + 2050.0; // amplitude adjustment should occur here
      // 2050.0 is DC offset?
      // 2000.0 is the amplitude? Test this today 9/14
      break;
    case SQUARE:
      // if phase > pi then 1 else 0
      (phase > twopi / 2) ? (DACval = (DACamplitude / maxAmpl) * 4095.0) : (DACval = 0.0);
      break;
    case SAW_DESC:
      // phase itself is linearly ramping
      DACval = floatmap(phase, 0, twopi, 1.0, 0.0) * (DACamplitude / maxAmpl) * 4095.0;
      break;
    case SAW_ASC:
      // phase itself is linearly ramping
      DACval = floatmap(phase, 0, twopi, 0.0, 1.0) * (DACamplitude / maxAmpl) * 4095.0;
      break;
    case NOISE:
    // check this on the scope!
      (random() > 0.5) ? (DACval = (DACamplitude / maxAmpl) * 4095.0) : (DACval = 0.0);
      break;
    default:
      DACval = sin(phase) * DACamplitude + 2050.0; // amplitude adjustment should occur here
      break;
  }
  analogWrite(A14, (int)DACval);
}
