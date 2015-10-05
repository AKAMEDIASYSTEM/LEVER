// ENCODER VERSION
/*
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

//  phase = phase + 0.2; // about 600Hz
//  phase = phase + 0.05; // about 150Hz
//  phase = phase + 0.025; // about 75Hz
*/
// #include <Adafruit_MCP4725.h> // not yet used, for external DAC if necessary
#include <Encoder.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 5
Adafruit_SSD1306 display(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
#define encA A2
#define encB A3
#define encButton A1
#define amplPot 20 // A6
#define freqPot 21 // A7
#define dutyPot 22 // A8
#define wavePot 23 // A9
#define pwmOut 4
#define SINE 0
#define SQUARE 1
#define SAW_DESC 2
#define SAW_ASC 3
#define NOISE 4

char* waveLabels[] = {"SINE", "SQUARE", "SAW_ASC", "SAW_DESC", "NOISE"};
char* modeLabels[] = {"FREQ", "AMP", "WAVE", "DUTY"};
volatile int encMode = 0;

float minFreq = 0.016 * 1000.0; // 0.0125 is too low for some people to feel, trying 0.016 now
float maxFreq = 0.4 * 1000.0;

float minDuty = 0.3;
float maxDuty = 0.95;
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

int potDeltaThreshold = 50; // read resolution is 10-bit (0-1023) so 50 is about 5%

long lastFreq = 0;
long lastWave = 0;
long lastAmp = 0;
long lastDuty = 0;
long newFreq = 0;
long newWave = 0;
long newAmp = 0;
long newDuty = 0;

long newPosition = 0;
long lastPosition = 0;
boolean shouldUpdate = true;


Encoder myEnc(encA, encB);

void setup() {
  // 0 - 4095 pwm values if res set to 12-bit
  analogWriteResolution(12);
  analogWriteFrequency(pwmOut, 375000);
  pinMode(pwmOut, OUTPUT);
  pinMode(encButton, INPUT_PULLUP); // MODE switch
  attachInterrupt(encButton, encButtonPress, RISING);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.display();
}

void loop() {
  shouldUpdate = false;
  // TODO, read teh encoder once per loop and then switch on the result
  // we are trying to resolve an issue where there's a discontinuity between the master encoder Position
  // and each mode's position
  // so: calc delta of encoder, then look at mode and increment the mode position by that delta?
  long newPosition = myEnc.read();
  if(abs((newPosition-oldPosition)) != 0){
    // calculate delta, toggle updateDisplay
    // increment the appropriate mode-position by the delta?
  }
  
  switch (encMode) {
    case 0: // FREQ
      newFreq = myEnc.read();
      newFreq = constrain(newFreq, 0, 1023);
      if (newFreq != lastFreq) {
        lastFreq = newFreq;
        shouldUpdate = true;
        phaseOffset = constrain(floatmap(newFreq, 0, 1023, minFreq, maxFreq), minFreq, maxFreq); // smoother - is this still needed
        phase = phase + (phaseOffset / 1000.0);
        if (phase >= twopi) {
          phase = 0;
        }
      }
      break;
    case 1: // AMP
      newAmp = myEnc.read();
      newAmp = constrain(newAmp, 0, 1023);
      if (newAmp != lastAmp) {
        lastAmp = newAmp;
        shouldUpdate = true;
        DACamplitude = constrain(floatmap(newAmp, 0.0, 1023.0, minAmpl, maxAmpl), minAmpl, maxAmpl);
      }
      break;
    case 2: // WAVE
      newWave = myEnc.read();
      newWave = constrain(newWave, 0, 1023);
      if (newWave != lastWave) {
        lastWave = newWave;
        shouldUpdate = true;
        waveType = constrain(map(newWave, 0, 1023, waveMin, waveMax), waveMin, waveMax);
      }
      break;
    case 3: // DUTY
      newDuty = myEnc.read();
      newDuty = constrain(newDuty, 0, 1023);
      if (newDuty != lastDuty) {
        lastDuty = newDuty;
        shouldUpdate = true;
        dutyCycle = constrain(floatmap(newDuty, 0.0, 1023.0, minDuty, maxDuty), minDuty, maxDuty);
      }
      break;
    default:
      newFreq = myEnc.read();
      newFreq = constrain(newFreq, 0, 1023);
      if (newFreq != lastFreq) {
        lastFreq = newFreq;
        shouldUpdate = true;
        phaseOffset = constrain(floatmap(newFreq, 0, 1023, minFreq, maxFreq), minFreq, maxFreq); // smoother - is this still needed
        phase = phase + (phaseOffset / 1000.0);
        if (phase >= twopi) {
          phase = 0;
        }
      }
      break;
  }



  analogWrite(pwmOut, int(4096 * dutyCycle)); // duty cycle should have been dynamically calculated before here

  calculateFeedback(); // this doesn't exist yet

  float DACval = 0;

  // working to invert this whole part, as currently the DAC amplitude control is reversed - it biases towards always-on and not always-off
  switch (waveType) {
    case SINE:
      DACval = sin(phase) * DACamplitude + 2050.0;
      // 2050.0 is DC offset?
      break;
    case SQUARE:
      // if phase > pi then 1 else 0
      (phase > twopi / 2) ? (DACval = (DACamplitude / maxAmpl) * 4095.0) : (DACval = 0.0);
      break;
    case SAW_DESC:
      // phase itself is linearly ramping
      DACval = floatmap(phase, 0, twopi, 0.0, 1.0) * (DACamplitude / maxAmpl) * 4095.0;
      break;
    case SAW_ASC:
      // phase itself is linearly ramping
      DACval = floatmap(phase, 0, twopi, 1.0, 0.0) * (DACamplitude / maxAmpl) * 4095.0;
      break;
    case NOISE:
      (random(0, 9) > 4.5) ? (DACval = (DACamplitude / maxAmpl) * 4095.0) : (DACval = 0.0);
      break;
    default:
      DACval = sin(phase) * DACamplitude + 2050.0;
      break;
  }

  DACval = 4095.0 - DACval;
  analogWrite(A14, (int)DACval);

  if (shouldUpdate) {
    updateDisplay();
  }

}

void encButtonPress() {
  encMode++;
  encMode %= 4;
}

