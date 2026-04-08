# Smart Pet Feeder - CSE328 Term Project

An IoT-based smart pet feeder developed for the CSE328 course at Akdeniz University. This system automates pet feeding by monitoring food weight, detecting pet presence, and allowing remote dispensing via a cloud dashboard.

## Overview

The project uses an ESP32 microcontroller to collect data from high-precision sensors and communicate with a cloud platform over MQTT. By replacing traditional distance sensors with a **weight-based system**, it provides more accurate monitoring of pet nutrition and habits.

## Key Features

*   **Precision Weight Monitoring:** Uses an **HX711 Load Cell** to measure the exact weight of food. The system converts raw weight data into a **percentage (%)** based on container capacity for easy monitoring.
*   **Pet Detection:** A PIR motion sensor logs and tracks exactly when the pet approaches the feeding area.
*   **Remote Control:** Trigger the dispensing mechanism manually from anywhere in the world using the Cloud Dashboard.
*   **Local Status Display:** A 128x64 OLED screen provides real-time local feedback, including Wi-Fi status, food percentage, and the "Last Fed" timestamp.
*   **Smart Alerts:** The system processes weight data to send "Low Food" notifications when the container level drops below a specific threshold.

## Hardware Components

*   **Microcontroller:** ESP32 Development Board
*   **Weight Sensor:** HX711 Load Cell Amplifier & 5kg Load Cell
*   **Motion Sensor:** PIR Motion Sensor
*   **Actuator:** SG90 Micro Servo Motor
*   **Display:** 128x64 I2C OLED Display

## How It Works (Software Logic)

The ESP32 reads the analog signal from the Load Cell via the HX711 amplifier. Through software calibration, the empty bowl weight is "tared" and the current weight is mapped to a 0-100% range. This data is then synced with the **Arduino Cloud** (or similar MQTT broker). When the "Feed" command is received from the cloud, the ESP32 rotates the SG90 servo motor to dispense food.

## Tech Stack

*   **Language:** C++
*   **Framework:** Arduino IDE / PlatformIO
*   **Protocols:** Wi-Fi, MQTT, I2C, PWM
