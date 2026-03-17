# Crutch LED System

I broke my foot and I wanted the crutches to look cool. They are painted blue with strips of LEDS along the structure. The Arduino controlled LEDs twinkle white like stars, and when I step a blue shooting star fires through the sky.

This is the code and wiring for the interactive LED system for mobility aids that reacts to walking motion using an IMU sensor.

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
- External 5V power supply (I use a power pank)
- Jumper wires (I used perfboard)

---

## Required Libraries

Install via Arduino Library Manager:

- FastLED  
- Adafruit BNO055  
- Adafruit Unified Sensor  

---

## Wiring Diagram

### Arduino Nano ↔ BNO055 (IMU)

Arduino Nano → BNO055

5V → VIN
GND → GND
A4 → SDA
A5 → SCL

---

### Arduino Nano ↔ WS2812B LED Strip

Arduino Nano → LED Strip

D10 → DIN (through 470Ω resistor)
5V → +5V
GND → GND


---