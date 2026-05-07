# smart_Brownout_Protection

An industrial-grade, Arduino-based AC power monitoring and protection system. This project continuously measures mains AC voltage using a ZMPT101B sensor, calculates the True RMS voltage to eliminate harmonic noise, and automatically disconnects connected appliances during dangerous brownouts or power surges.

## ✨ Features

* **True RMS Calculation:** Uses a custom 32-bit processing algorithm to sample the AC sine wave and calculate the Root Mean Square. This completely eliminates random electrical noise and floating ground spikes.
* **Under/Over Voltage Protection:** Automatically triggers a relay to cut power if the voltage drops below 180V (Brownout) or spikes above 250V (Surge).
* **"Dead Grid" Detection:** If the voltage falls below 90V, the system enters a silent "NO INPUT" standby mode without sounding alarms.
* **Fault Simulation Mode:** Includes a physical hardware interrupt button to simulate a sudden 160V brownout for testing and presentation purposes.
* **Smart UI & Alarms:** Features a live OLED dashboard and a passive buzzer that triggers audio warnings during active faults.

## 🛠️ Hardware Requirements

* 1x **Arduino Nano** (or Uno)
* 1x **ZMPT101B** AC Voltage Sensor Module
* 1x **SSD1306 OLED Display** (128x64, I2C)
* 1x **5V Relay Module** (Active-Low)
* 1x **Passive Buzzer**
* 1x **Push Button** (For fault simulation)
* Jumper Wires & 5V Power Supply

## 🔌 Pin Connections

| Component | Pin Name | Arduino Nano Pin | Notes |
| :--- | :--- | :--- | :--- |
| **ZMPT101B Sensor** | OUT | **A2** | Analog Signal |
| | VCC / GND | 5V / GND | Power |
| **OLED (I2C)** | SDA | **A4** | I2C Data |
| | SCL | **A5** | I2C Clock |
| **Relay Module** | IN | **D2** | Active-LOW Trigger |
| **Passive Buzzer** | Positive (+) | **D5** | PWM Tone Output |
| **Sim Button** | Terminal 1 | **D7** | Uses `INPUT_PULLUP` |
| | Terminal 2 | **GND** | Pulls D7 LOW when pressed |

> **Note:** The simulation button relies on the Arduino's internal pull-up resistor. Wire the button directly between D7 and GND. No external resistors are required.

## 💻 Software Setup

This project does not rely on external "black-box" sensor libraries for voltage calculation, ensuring full control over the signal processing.

You will only need to install the following display libraries via the Arduino Library Manager:
1. `Adafruit GFX Library`
2. `Adafruit SSD1306`

## ⚙️ Calibration Guide

To ensure high accuracy, this system must be calibrated in two steps:

### 1. Hardware Calibration (Preventing Wave Clipping)
If the ZMPT101B potentiometer is turned too high, the AC sine wave will hit the 5V limit of the Arduino, resulting in a squared-off wave and massive math errors (e.g., jumping to 400V+).
* While monitoring the voltage, slowly turn the blue potentiometer on the ZMPT101B **counter-clockwise** until the reading stops fluctuating wildly and stabilizes into a clean, lower number. 

### 2. Software Calibration
Once the hardware wave is clean, scale the math to match your actual wall voltage.
1. Use a trusted Multimeter to measure your actual wall AC voltage.
2. Note the stable voltage currently showing on the OLED screen.
3. Divide the **Multimeter Voltage** by the **OLED Voltage** to find your ratio. 
4. Multiply your current `calibration_factor` in the code by that ratio.
5. Update line 15 in the code: `float calibration_factor = [YOUR_NEW_NUMBER];`

## 📊 System States

The system features an exponential moving average filter to stabilize the OLED display, which operates in one of three states:

1. **STATUS: HEALTHY (180V - 250V):** Relay is LOW (Load is ON). Buzzer is silent.
2. **STATUS: TRIP! (< 180V or > 250V):** Relay is HIGH (Load is OFF). Buzzer sounds continuous 1000Hz alarm.
3. **STATUS: NO INPUT (< 90V):** Relay is HIGH (Load is OFF). Buzzer is silent.

*If the D7 Simulation button is held, the screen updates to "SIM: FAULT DETECTED" and forces a 160V trip sequence, displaying "PROTECTING LOAD...".*

---
*Developed for robust electrical protection and precise digital signal processing.*
