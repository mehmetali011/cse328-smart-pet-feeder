#include "arduino_secrets.h"
#include <ESP32Servo.h>
#include "HX711.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include "thingProperties.h"

// -------------------- OLED settings --------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SH1106G display = Adafruit_SH1106G(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

const int OLED_SDA_PIN = 21;
const int OLED_SCL_PIN = 22;

bool oledReady = false;

unsigned long previousOledMillis = 0;
const unsigned long oledInterval = 500;

// -------------------- Servo settings --------------------
Servo feederServo;

const int SERVO_PIN = 13;

// Current project logic:
// 75 degrees  = door open
// 180 degrees = door closed
const int SERVO_OPEN_ANGLE = 75;
const int SERVO_CLOSED_ANGLE = 180;

// Feed time
const unsigned long FEED_OPEN_TIME = 500; // ms

unsigned long lastFeedMillis = 0;
bool hasFedBefore = false;

// -------------------- PIR settings --------------------
const int PIR_PIN = 27;

// Pet is detected for 5 seconds after last motion
const unsigned long PET_TIMEOUT = 5000;

unsigned long lastMotionTime = 0;

bool localPetDetected = false;
bool lastLocalPetDetected = false;

int lastRawPirState = -1;

// -------------------- HX711 settings --------------------
const int HX711_DOUT_PIN = 19;  // DT / DOUT
const int HX711_SCK_PIN  = 18;  // SCK / CLK

HX711 scale;

// Stable calibration value
float calibration_factor = -399.0;

// Load cell read time
unsigned long previousScaleMillis = 0;
const unsigned long scaleInterval = 700;

// Noise filter
float lastPrintedWeight = 0.0;
const float weightChangeThreshold = 3.0;

// Accept values close to zero as 0
const float zeroDeadband = 3.0;

float currentDisplayWeight = 0.0;

// Low food warning limit
const float LOW_FOOD_THRESHOLD = 30.0; // grams

// -------------------- Helper functions --------------------
String getLastFeedText() {
  if (!hasFedBefore) {
    return "--";
  }

  unsigned long elapsedSeconds = (millis() - lastFeedMillis) / 1000;

  if (elapsedSeconds < 60) {
    return String(elapsedSeconds) + "s ago";
  }

  unsigned long minutes = elapsedSeconds / 60;
  unsigned long seconds = elapsedSeconds % 60;

  return String(minutes) + "m " + String(seconds) + "s ago";
}

void updateOLED() {
  if (!oledReady) {
    return;
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("Smart Pet Feeder");

  display.drawLine(0, 10, 127, 10, SH110X_WHITE);

  display.setCursor(0, 14);
  display.print("Cloud: ");
  if (ArduinoCloud.connected()) {
    display.println("Online");
  } else {
    display.println("Offline");
  }

  display.setCursor(0, 26);
  display.print("Food: ");
  display.print(currentDisplayWeight, 1);
  display.println(" g");

  display.setCursor(0, 38);
  display.print("Pet: ");
  if (localPetDetected) {
    display.println("Detected");
  } else {
    display.println("Waiting");
  }

  display.setCursor(0, 50);
  display.print("Last: ");
  display.println(getLastFeedText());

  display.display();
}

void showOLEDMessage(String line1, String line2 = "", String line3 = "") {
  if (!oledReady) {
    return;
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.println(line1);

  if (line2.length() > 0) {
    display.println(line2);
  }

  if (line3.length() > 0) {
    display.println(line3);
  }

  display.display();
}

void doFeed() {
  Serial.println("FEED started.");

  feederServo.write(SERVO_OPEN_ANGLE);
  Serial.println("Servo: 75 degrees -> DOOR OPEN");

  delay(FEED_OPEN_TIME);

  feederServo.write(SERVO_CLOSED_ANGLE);
  Serial.println("Servo: 180 degrees -> DOOR CLOSED");

  lastFeedMillis = millis();
  hasFedBefore = true;

  lastFeedText = getLastFeedText();

  Serial.println("FEED finished.");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // -------------------- Start OLED --------------------
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);

  if (!display.begin(0x3C, true)) {
    oledReady = false;
    Serial.println("OLED not found. Address might not be 0x3C or connection error.");
  } else {
    oledReady = true;
    showOLEDMessage("Smart Pet Feeder", "OLED started.");
    delay(1000);
  }

  // -------------------- Start Servo --------------------
  feederServo.setPeriodHertz(50);
  feederServo.attach(SERVO_PIN, 500, 2400);

  feederServo.write(SERVO_CLOSED_ANGLE);
  Serial.println("Servo started: 180 degrees -> DOOR CLOSED");

  // -------------------- Start PIR --------------------
  pinMode(PIR_PIN, INPUT);

  Serial.println("PIR stabilizing. Waiting 30 seconds...");
  showOLEDMessage("PIR stabilizing", "Wait 30 sec...");
  delay(30000);

  // To prevent false Pet Detected on startup
  lastMotionTime = millis() - PET_TIMEOUT - 1;

  // -------------------- Start HX711 --------------------
  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  scale.set_scale(calibration_factor);

  Serial.println("Waiting for HX711...");
  showOLEDMessage("HX711", "Waiting...");

  while (!scale.is_ready()) {
    delay(100);
  }

  Serial.println("HX711 ready.");
  Serial.println("Taring. Platform must be empty.");
  showOLEDMessage("HX711 ready", "Taring...", "Platform empty");

  delay(2000);

  scale.tare();

  currentDisplayWeight = 0.0;
  lastPrintedWeight = 0.0;

  Serial.println("Tare complete.");
  Serial.println("First weight: 0.00 g");

  // -------------------- Start Arduino Cloud --------------------
  initProperties();

  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  // First cloud values
  foodWeight = 0.0;
  petDetectedCloud = false;
  lowFoodAlert = false;
  lastFeedText = "--";
  feedNow = false;

  Serial.println("--------------------------------");
  Serial.println("Smart Pet Feeder Cloud code started.");
  Serial.println("Servo will be controlled with feedNow from Dashboard.");
  Serial.println("--------------------------------");

  updateOLED();
}

void loop() {
  unsigned long currentMillis = millis();

  // Keep updating Cloud connection
  ArduinoCloud.update();

  // -------------------- Local servo test with Serial command --------------------
  if (Serial.available() > 0) {
    char command = Serial.read();

    if (command == 'o' || command == 'O') {
      feederServo.write(SERVO_OPEN_ANGLE);
      Serial.println("Command: DOOR OPEN -> 75 degrees");
    }

    else if (command == 'c' || command == 'C') {
      feederServo.write(SERVO_CLOSED_ANGLE);
      Serial.println("Command: DOOR CLOSED -> 180 degrees");
    }

    else if (command == 'f' || command == 'F') {
      Serial.println("Command: FEED TEST");
      doFeed();
    }
  }

  // -------------------- Read PIR --------------------
  int rawPirState = digitalRead(PIR_PIN);

  // Print raw PIR only when it changes
  if (rawPirState != lastRawPirState) {
    lastRawPirState = rawPirState;

    if (rawPirState == HIGH) {
      Serial.println("PIR RAW: HIGH");
    } else {
      Serial.println("PIR RAW: LOW");
    }
  }

  // Update time if motion detected
  if (rawPirState == HIGH) {
    lastMotionTime = currentMillis;
  }

  // Pet detected if motion in last 5 seconds
  if (currentMillis - lastMotionTime <= PET_TIMEOUT) {
    localPetDetected = true;
  } else {
    localPetDetected = false;
  }

  // Update filtered pet status only when changed
  if (localPetDetected != lastLocalPetDetected) {
    lastLocalPetDetected = localPetDetected;

    if (localPetDetected) {
      Serial.println("PET STATUS: Pet Detected");
    } else {
      Serial.println("PET STATUS: Waiting / No Pet");
    }

    petDetectedCloud = localPetDetected;
  }

  // -------------------- Read Load cell --------------------
  if (currentMillis - previousScaleMillis >= scaleInterval) {
    previousScaleMillis = currentMillis;

    // Skip if HX711 is not ready
    if (!scale.is_ready()) {
      return;
    }

    // Average of 20 readings for stable measure
    float currentWeight = scale.get_units(20);

    // Accept values close to zero as 0
    if (abs(currentWeight) <= zeroDeadband) {
      currentWeight = 0.0;
    }

    currentDisplayWeight = currentWeight;

    // Update cloud weight
    foodWeight = currentDisplayWeight;

    // Low food alert
    if (currentDisplayWeight <= LOW_FOOD_THRESHOLD) {
      lowFoodAlert = true;
    } else {
      lowFoodAlert = false;
    }

    // Print to Serial only if big change
    if (abs(currentWeight - lastPrintedWeight) >= weightChangeThreshold) {
      Serial.print("Weight changed: ");
      Serial.print(lastPrintedWeight, 2);
      Serial.print(" g -> ");
      Serial.print(currentWeight, 2);
      Serial.println(" g");

      lastPrintedWeight = currentWeight;
    }
  }

  // -------------------- Update last feed text --------------------
  if (hasFedBefore) {
    lastFeedText = getLastFeedText();
  }

  // -------------------- Update OLED --------------------
  if (currentMillis - previousOledMillis >= oledInterval) {
    previousOledMillis = currentMillis;
    updateOLED();
  }
}

// This runs when feedNow changes on Arduino Cloud
void onFeedNowChange() {
  if (feedNow == true) {
    Serial.println("Cloud command: feedNow = true");

    doFeed();

    // Make button false again so it can be pressed again
    feedNow = false;
  }
}