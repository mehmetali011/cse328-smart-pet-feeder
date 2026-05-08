# Smart Pet Feeder - CSE328 Term Project

An IoT-based smart pet feeder developed for the CSE328 course at Akdeniz University. This system automates pet feeding by monitoring food weight, detecting pet presence, and allowing remote dispensing via the Arduino Cloud dashboard.

## Overview

The project uses an ESP32 microcontroller to collect data from high-precision sensors and communicate with the Arduino IoT Cloud. By relying on a weight-based load cell system rather than traditional distance sensors, the feeder provides highly accurate, real-time monitoring of pet nutrition and container capacity.

## Key Features

* **Precision Weight Monitoring:** Uses an HX711 Load Cell to measure the exact weight of the food in **grams**. The system averages multiple readings for stability and includes a zero-deadband filter to ignore minor sensor noise.
* **Pet Detection:** A PIR motion sensor tracks when the pet approaches the feeding area, utilizing a 5-second timeout window to maintain a stable detection state.
* **Remote Control & Synchronization:** Fully integrated with Arduino Cloud. Users can trigger the servo-driven dispensing mechanism manually from the cloud dashboard using the `feedNow` variable.
* **Local Status Display:** A 128x64 OLED screen updates every 500ms to provide real-time local feedback, including Cloud connection status, current food weight (g), pet presence, and a dynamically updating "Last Fed" timer.
* **Smart Alerts:** The system processes weight data locally and triggers a "Low Food Alert" to the cloud when the container level drops below 30 grams.

## Hardware Components

* **Microcontroller:** ESP32 Development Board
* **Weight Sensor:** HX711 Load Cell Amplifier & 5kg Load Cell
* **Motion Sensor:** PIR Motion Sensor
* **Actuator:** SG90 Micro Servo Motor
* **Display:** 128x64 I2C OLED Display (Adafruit SH1106G)

## How It Works (Software Logic)

1.  **Initialization:** On startup, the ESP32 initializes the I2C OLED display, sets the servo to the closed position (180 degrees), allows the PIR sensor 30 seconds to stabilize, and tares the HX711 load cell (setting the empty platform weight to 0.00g).
2.  **Main Loop:** The system continuously updates the Arduino Cloud connection. It reads the PIR sensor state, filtering the raw input to determine if a pet is actively present. 
3.  **Weight Processing:** Every 700ms, the ESP32 takes an average of 20 readings from the load cell. If the weight drops below the defined threshold (30g), the `lowFoodAlert` flag is pushed to the cloud.
4.  **Actuation:** When the `feedNow` command is received from the cloud (or via local Serial commands for testing), the ESP32 rotates the servo motor to 75 degrees for 500 milliseconds to dispense food, then returns to the closed position. The system then records the timestamp to update the "Last Fed" metric.

## Tech Stack

* **Language:** C++
* **Framework:** Arduino IDE / PlatformIO
* **Cloud Platform:** Arduino IoT Cloud
* **Protocols & Interfaces:** Wi-Fi, I2C (OLED), PWM (Servo)
