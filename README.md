# Crutch LED System

I broke my foot and I wanted the crutches to look cool. They are painted blue with strips of LEDS along the structure. The Arduino controlled LEDs twinkle white like starts, and when I step a blue shooting star fires through the sky.

This is th4 code and wiring for the interactive LED system for mobility aids that reacts to walking motion using an IMU sensor.

---

## Features

- **Idle Mode**  
  Soft white twinkling LEDs.

- **Step Detection**  
  Detects motion using IMU acceleration data.

- **Dynamic Ripple Effect**  
  On each step, a ripple travels up the LED strip.

- **Color Cycling**
  White → Light Blue → Dark Blue → repeat.

- **Queued Animation**  
  Steps during an active ripple trigger the next ripple seamlessly.

---

## Hardware Requirements

- Arduino Nano (LAFVIN or compatible)
- WS2812B LED strip (100 LEDs recommended)
- BNO055 IMU sensor
- 470Ω resistor (data line protection)
- External 5V power supply (I use a power pank)
- Jumper wires

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

###  Power Notes

- Do NOT power LED strip directly from Arduino
- Use external 5V supply
- Ensure all grounds are connected together

---

##  Getting Started

### 1. Install Arduino IDE
https://www.arduino.cc/en/software

---

### 2. Install Libraries

In Arduino IDE:
Sketch → Include Library → Manage Libraries

Search and install:
- FastLED
- Adafruit BNO055
- Adafruit Unified Sensor

---

### 3. Upload Code

1. Connect Arduino Nano via USB  
2. Select:
   - Tools → Board → Arduino Nano  
   - Processor → ATmega328P (Old Bootloader if needed)  
3. Click Upload  

---

### 4. Verify IMU

- If initialization fails:
  - LEDs blink red
  - Check wiring

---