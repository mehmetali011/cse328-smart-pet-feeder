# Smart Pet Feeder - CSE328 Term Project

An IoT-based smart pet feeder developed for the CSE328 course. This system automates pet feeding by monitoring food levels, detecting pet presence, and allowing remote dispensing via a cloud dashboard.

## Overview

The project uses an ESP32 microcontroller to collect data from sensors and communicate with a cloud platform over MQTT. It solves the issue of portion control without requiring complex mechanical rotary parts by using a software-based time calculation for the servo motor.

## Key Features

* **Food Level Monitoring:** HC-SR04 ultrasonic sensor calculates the remaining food percentage.
* **Pet Detection:** PIR motion sensor logs when the pet is at the bowl.
* **Remote Control:** Dispense food manually from anywhere using the Cloud Dashboard.
* **Local Display:** 128x64 OLED shows Wi-Fi status, food level, and last feeding time.
* **Software Portion Control:** Algorithmic calculation adjusts the servo open-time based on the current food level to ensure consistent portions.

## Hardware Components

* ESP32 Development Board
* HC-SR04 Ultrasonic Sensor
* PIR Motion Sensor
* SG90 Micro Servo Motor
* 128x64 I2C OLED Display

## Tech Stack

* **Language:** C++
* **Framework:** Arduino IDE / PlatformIO
* **Protocols:** Wi-Fi, MQTT, I2C
