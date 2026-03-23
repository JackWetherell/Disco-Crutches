# Crutch LED System

I broke my foot and decided my crutches needed an upgrade. They are painted blue with LED strips along the structure. The Arduino-controlled LEDs twinkle like stars when idle, and with each step a blue shooting star soars around crutch.

<p align="center">
  <img src="images/demo.gif" width="300">
</p>

This is the code and wiring for the interactive LED system for crutches that react to walking motion using an IMU sensor.

---

## Features

- **Idle Mode**
  Soft white twinkling LEDs.

- **Step Detection**
  Detects motion using IMU acceleration data.

- **Dynamic Ripple Effect**
  On each step, a ripple travels up the LED strip.

- **Color Cycling**
  White, Light Blue, Dark Blue, repeat.

- **Queued Animation**
  Steps during an active ripple trigger the next ripple seamlessly.

---

## Hardware Requirements

- Arduino Nano (LAFVIN or compatible)
- WS2812B LED strip (100 LEDs recommended)
- BNO055 IMU sensor
- 470Ω resistor (data line protection)
- External 5V power supply (I use a USB power bank, ≥1800 mA recommended)
- Jumper wires (I used perfboard)

---

## Required Libraries

Install via **Arduino IDE → Tools → Manage Libraries**:

| Library | Author | Notes |
|---------|--------|-------|
| FastLED | FastLED | WS2812B LED control |
| Adafruit BNO055 | Adafruit | IMU driver |
| Adafruit Unified Sensor | Adafruit | Required by BNO055 |

All three are available directly in the Arduino Library Manager by searching their names.

---

## Wiring Diagram

<p align="center">
  <img src="images/schematic.png" width="700">
</p>

---

## Installation

1. Install the required libraries listed above.
2. Open `crutches.ino` in the Arduino IDE.
3. If your LED strip has a **different number of LEDs**, change `#define NUM_LEDS 100` to match and recompile.
4. Select **Board: Arduino Nano** and the correct serial port.
5. Upload the sketch.

On first power-on the LEDs will briefly flash red during IMU initialisation (up to 5 retries). Once ready, the strip enters the idle twinkle animation.

---

## Calibration

The step detection thresholds may need tuning depending on how the IMU is mounted and how you walk. The key parameters are near the top of `crutches.ino`:

| Parameter | Default | Effect |
|-----------|---------|--------|
| `stepHigh` | `1.8` | Activity level required to trigger a step. **Increase** if false triggers occur; **decrease** if steps are missed. |
| `stepLow` | `0.7` | Activity must drop below this before the detector re-arms. Keep below `stepHigh`. |
| `lockoutMs` | `300` | Minimum milliseconds between two triggers. Increase to suppress double-counts. |
| `quietMsRequired` | `400` | How long activity must stay below `stepLow` before re-arming early. |
| `baseAlpha` | `0.98` | Baseline smoothing. Higher = slower to adapt to new movement patterns. |
| `actAlpha` | `0.90` | Activity smoothing. Lower = reacts faster but noisier. |

**Tuning workflow:**
Upload, walk normally, and watch the ripple triggers. If ripples fire when you are standing still, raise `stepHigh`. If ripples do not fire when walking, lower `stepHigh`.

---

## Power Consumption

The sketch caps LED power draw with `FastLED.setMaxPowerInVoltsAndMilliamps(5, 1800)`, so total draw never exceeds **~9 W** (5 V × 1800 mA). In practice, average draw is significantly lower because LEDs are mostly off or dimmed.

A **2000 mAh USB power bank** provides roughly 4–6 hours of use depending on activity level.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---------|-------------|-----|
| All LEDs blink red continuously | IMU not detected after 5 retries | Check I2C wiring (SDA→A4, SCL→A5) and that BNO055 is powered |
| LEDs flicker or show wrong colors | Loose data wire or missing 470Ω resistor | Re-seat connections; verify resistor on D10 |
| Steps not detected | Thresholds too high for your walking style | Lower `stepHigh` (try 1.4–1.6) |
| Too many false triggers | Thresholds too low or IMU loosely mounted | Raise `stepHigh`; secure the IMU |
| Strip only partially lights up | `NUM_LEDS` does not match actual strip length | Update `#define NUM_LEDS` and recompile |
| Power bank shuts off immediately | Power bank requires minimum load; the brief delay at startup may not satisfy it | Add a small dummy load, or use a power bank that does not have auto-off |
