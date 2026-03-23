/*
  ============================================================
  CRUTCH LED SYSTEM
  ------------------------------------------------------------
  Features:
  - Idle: soft white twinkling LEDs
  - Step detected: fast ripple up the strip
  - Ripple cycles through 3 colors:
      white → light blue → dark blue → repeat

  Hardware:
  - Arduino Nano (LAFVIN)
  - WS2812B LED strip DATA on D10 (through 470Ω resistor)
  - BNO055 IMU using I2C on A4 (SDA) and A5 (SCL)

  Libraries required:
  - FastLED
  - Adafruit BNO055
  - Adafruit Unified Sensor

  Pin layout:

  Arduino Nano    →    BNO055
  ----------------------------
  5V              →    VIN
  GND             →    GND
  A4              →    SDA
  A5              →    SCL


  Arduino Nano    →    LED Strip
  -------------------------------
  D10             →    DIN (through 470Ω resistor)
  5V              →    +5V
  GND             →    GND

  ============================================================
*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <FastLED.h>

// LED config
// NOTE: Change NUM_LEDS to match your strip length and recompile.
#define LED_PIN 10
#define NUM_LEDS 100
#define BRIGHTNESS 50
CRGB leds[NUM_LEDS];
const uint16_t MAX_MA = 1800;

// IMU config
Adafruit_BNO055 bno = Adafruit_BNO055(55);

// Baseline activity smoothing
//
// We track two exponential moving averages (EMAs):
//   baseMag  — slow-moving average of raw acceleration magnitude,
//              representing the "background" level (gravity + gentle movement).
//   activity — fast-moving average of |mag - baseMag|, representing
//              how "spiky" or energetic the current motion is.
//
// EMA update rule:  value = alpha * value + (1 - alpha) * sample
// Higher alpha → smoother / slower to react.
float baseMag = 9.8f;                 // baseline acceleration magnitude (approx g)
float activity = 0.0f;                // smoothed "spikiness"
const float baseAlpha = 0.98f;        // baseline smoothing (0.97–0.995); high = very slow drift
const float actAlpha  = 0.90f;        // activity smoothing (0.85–0.95); lower = reacts faster

// Step detection parameters
//
// Hysteresis thresholds prevent rapid re-triggering:
//   activity must rise above stepHigh  → trigger candidate
//   activity must fall below stepLow   → sensor considered "quiet" again
// lockoutMs   — minimum gap between any two triggers (debounce).
// quietMsRequired — how long activity must stay below stepLow before
//                   the detector re-arms (prevents double-counting one step).
float stepHigh = 1.8f;                // trigger threshold
float stepLow  = 0.7f;                // must drop below this to be considered "quiet"
const uint16_t lockoutMs = 300;       // minimum time between triggers (ms)
const uint16_t quietMsRequired = 400; // must be quiet this long before re-arming (ms)

// Initialisation
unsigned long lastStep = 0;
unsigned long quietSince = 0;
bool stepArmed = true;
float prevActivity = 0.0f;

// Ripple config
bool rippleActive = false;
float ripplePos = 0.0f;
float rippleSpeed = 5.0f;            // LEDs per frame in LEDs/second
uint8_t rippleWidth = 4;             // ripple thickness in LEDs
bool rippleQueued = false;           // if a step happens mid-ripple, play another after

// Ripple colors (cycle)
const CRGB rippleColors[] = {
  CRGB::White,                       // white
  CRGB(40, 100, 255),                // light blue
  CRGB(0, 40, 255)                   // dark blue
};
const uint8_t NUM_COLORS = sizeof(rippleColors) / sizeof(CRGB);
uint8_t currentColorIndex = 0;
CRGB currentColor;

// Twinkle config
#define TWINKLE_CHANCE 40            // lower=fewer sparkles (20–60)
#define TWINKLE_BRIGHT 80            // sparkle intensity (50–120)


void startRipple() {
  rippleActive = true;
  ripplePos = 0.0f;
  currentColor = rippleColors[currentColorIndex];
  currentColorIndex = (currentColorIndex + 1) % NUM_COLORS;
}


void updateRipple() {
  fadeToBlackBy(leds, NUM_LEDS, 80);
  int center = (int)ripplePos;

  for (int i = -(int)rippleWidth; i <= (int)rippleWidth; i++) {
    int p = center + i;
    if (p < 0 || p >= NUM_LEDS) continue;
    // Linear brightness falloff from center to edge of ripple:
    // center pixel (i=0) → 255, edge pixel (i=±rippleWidth) → ~0.
    int b = 255 - abs(i) * (255 / (rippleWidth + 1));
    b = constrain(b, 0, 255);
    CRGB scaled = currentColor;
    scaled.nscale8((uint8_t)b);
    leds[p] += scaled;
  }

  ripplePos += rippleSpeed;

  if (ripplePos >= NUM_LEDS + rippleWidth) {
    rippleActive = false;
    if (rippleQueued) {
      rippleQueued = false;
      startRipple();
    }
  }
}


void updateTwinkle() {
  fadeToBlackBy(leds, NUM_LEDS, 10);
  if (random8() < TWINKLE_CHANCE) {
    int pos = random16(NUM_LEDS);
    leds[pos] += CRGB(TWINKLE_BRIGHT, TWINKLE_BRIGHT, TWINKLE_BRIGHT);
  }
}


bool detectStep() {
  sensors_event_t e;
  // Request accelerometer vector; returns false on I2C failure.
  if (!bno.getEvent(&e, Adafruit_BNO055::VECTOR_ACCELEROMETER)) {
    return false;  // skip this sample if sensor read fails
  }

  float ax = e.acceleration.x;
  float ay = e.acceleration.y;
  float az = e.acceleration.z;

  float mag = sqrt(ax*ax + ay*ay + az*az);

  // Update slow baseline (tracks gravity + low-frequency drift).
  baseMag = baseAlpha * baseMag + (1.0f - baseAlpha) * mag;
  // Update fast activity (tracks short bursts above baseline).
  float delta = fabs(mag - baseMag);
  activity = actAlpha * activity + (1.0f - actAlpha) * delta;

  unsigned long now = millis();

  // Re-arm after lockout period regardless of activity level.
  if (!stepArmed && (now - lastStep) >= lockoutMs) {
    stepArmed = true;
    quietSince = 0;
  }

  // Track how long the sensor has been quiet; re-arm early if quiet long enough.
  if (activity < stepLow) {
    if (quietSince == 0) quietSince = now;
    if (!stepArmed && (now - quietSince) >= quietMsRequired) {
      stepArmed = true;
    }
  } else {
    quietSince = 0;
  }

  // Detect rising edge: activity just crossed above stepHigh.
  bool risingCross = (prevActivity <= stepHigh && activity > stepHigh);
  prevActivity = activity;

  if (stepArmed && risingCross && (now - lastStep) > lockoutMs) {
    lastStep = now;
    stepArmed = false;
    return true;
  }

  return false;
}


void setup() {
  delay(200);

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_MA);

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  Wire.begin();

  // Attempt IMU initialisation with up to 5 retries.
  // Blinks red while retrying; continues if eventually successful.
  // If all retries fail, blinks red indefinitely to signal the fault.
  const uint8_t MAX_IMU_RETRIES = 5;
  bool imuOk = false;
  for (uint8_t attempt = 0; attempt < MAX_IMU_RETRIES; attempt++) {
    if (bno.begin()) {
      imuOk = true;
      break;
    }
    // Flash red once per failed attempt to give visual feedback.
    fill_solid(leds, NUM_LEDS, CRGB::Red);
    FastLED.show(); delay(200);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show(); delay(300);
  }

  if (!imuOk) {
    // All retries exhausted — blink red indefinitely to signal fault.
    while (true) {
      fill_solid(leds, NUM_LEDS, CRGB::Red);
      FastLED.show(); delay(200);
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show(); delay(200);
    }
  }

  bno.setExtCrystalUse(true);
}


void loop() {
  if (detectStep()) {
    if (rippleActive) rippleQueued = true;
    else startRipple();
  }

  if (rippleActive) updateRipple();
  else updateTwinkle();

  FastLED.show();
  delay(20);
}
