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
#define pwmOut 4
#define SINE 0
#define SQUARE 1
#define SAW_DESC 2
#define SAW_ASC 3
#define NOISE 4

char* waveLabels[] = {"SINE", "SQUARE", "SAW_ASC", "SAW_DESC", "NOISE"};
char* modeLabels[] = {"FREQ", "AMP", "WAVE", "DUTY"};
volatile int encMode = 0;
volatile boolean shouldUpdate = true;

float minFreq = 0.016 * 1000.0; // 0.0125 is too low for some people to feel, trying 0.016 now
float maxFreq = 0.4 * 1000.0;

float minDuty = 0.3;
float maxDuty = 0.95;
float dutyCycle = 0.75;

float minAmpl = 0.0;
float maxAmpl = 2000.0;
float DACamplitude = 1000.0;

int waveType = 0; // sine, square, saw descending, saw ascending, noise
#define waveMax 4
#define waveMin 0

float phase = 0.0;
float twopi = 3.14159 * 2;
float phaseOffset = 0.05;

long lastFreq = 200; // middle-to-low end of the 0-1023 range
long lastWave = 0; // sine
long lastAmp = 1023; // full amplitude
long lastDuty = 10; // low end of duty cycle
long newFreq = 200;
long newWave = 0;
long newAmp = 1023;
long newDuty = 10;

long newPosition = 0;
long lastPosition = 0;

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
  long newPosition = myEnc.read();
  long delta = newPosition - lastPosition;

  if (abs((delta)) != 0) {
    // calculate delta, toggle updateDisplay
    // increment the appropriate mode-position by the delta?
    switch (encMode) {
      case 0: // FREQ
        newFreq += delta;
        lastFreq = newFreq;
        phaseOffset = constrain(floatmap(newFreq, 0, 1023, minFreq, maxFreq), minFreq, maxFreq);
        break;
      case 1: // AMP
        newAmp += delta;
        lastAmp = newAmp;
        DACamplitude = constrain(floatmap(newAmp, 0.0, 1023.0, minAmpl, maxAmpl), minAmpl, maxAmpl);
        break;
      case 2: // WAVE
        newWave += (int(delta / 4) % 4); // wnat this to change every encoder detent, which is 4 counts... still not working great
        lastWave = newWave;
        waveType = constrain(map(newWave, 0, 3, waveMin, waveMax), waveMin, waveMax);
        break;
      case 3: // DUTY
        newDuty += delta;
        lastDuty = newDuty;
        dutyCycle = constrain(floatmap(newDuty, 0.0, 1023.0, minDuty, maxDuty), minDuty, maxDuty);
        break;
      default:
        newFreq += delta;
        lastFreq = newFreq;
        phaseOffset = constrain(floatmap(newFreq, 0, 1023, minFreq, maxFreq), minFreq, maxFreq);
        break;
    }
    shouldUpdate = true;
    lastPosition = newPosition;
  }

  analogWrite(pwmOut, int(4096 * dutyCycle)); // duty cycle should have been dynamically calculated before here

  calculateFeedback(); // this doesn't exist yet

  float DACval = 0;
  phase = phase + (phaseOffset / 1000.0);
  if (phase >= twopi) {
    phase = 0;
  }

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
    shouldUpdate = false;
  }
  
  shouldUpdate = false;
}

void encButtonPress() {
  encMode++;
  encMode %= 4;
  shouldUpdate = true;
}

