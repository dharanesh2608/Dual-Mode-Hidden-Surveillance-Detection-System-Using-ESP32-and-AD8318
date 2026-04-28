# Dual-Mode Hidden Surveillance Detection System  
Using ESP32, AD8318 (RF) and TSOP1838 (IR)

## Overview  
This project is a dual-mode hidden camera detection system that uses both radio frequency (RF) and infrared (IR) sensing. It is designed to identify hidden surveillance devices such as wireless cameras and night-vision cameras in indoor environments.

The system is built using an ESP32 microcontroller, which processes signals from RF and IR modules and provides alerts through a display and buzzer.

---

## Device Images  

### Hardware Setup  
![Hardware Setup](images/devicepic1.jpg)

### Working Device  
![Device](images/devicepic2.jpg)

---

## Features  
- Detects wireless cameras using RF signals (1–2.4 GHz)  
- Detects night-vision cameras using IR sensing  
- Displays RF signal strength on OLED  
- Shows alerts on LCD screen  
- Provides buzzer alert when detection occurs  
- Fast response time (within 2 sec)  
- Detection range of approximately 0.5 to 1.5 meters  

---

## Components Used  
- ESP32 Development Board  
- AD8318 RF Detector  
- TSOP1838 IR Receiver  
- 2.4 GHz Antenna  
- 16x2 LCD (I2C)  
- SSD1306 OLED Display  
- Active Buzzer  
- Power Supply Module  

---

## Working Principle  
The RF module detects signals from wireless transmitting devices and converts them into voltage. The ESP32 reads this voltage and estimates signal strength.

The IR module detects infrared signals emitted by night-vision cameras. Based on the input from both sensors, the system determines whether a hidden camera may be present.

The system operates in different states such as idle, RF detection, IR detection, and combined detection.

---

## Applications  
- Hotel room inspection  
- Office and meeting room security  
- Examination halls  
- Rental houses  
- General privacy protection  

---

## Limitations  
- Cannot detect cameras that are powered but not transmitting  
- Limited to RF range of 1–2.4 GHz  
- Does not provide exact location of the device  

---

## Future Improvements  
- Direction detection using antenna arrays  
- Support for wider frequency ranges  
- Mobile app integration  
- Improved filtering using machine learning

  ---
  ## Note  
This project was developed as part of an academic final-year project.
