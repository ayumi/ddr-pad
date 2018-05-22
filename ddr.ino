/*
  DDR pad IKA
  Version: 1.1
  By Ayumi Yu 2018
  License: CC-BY 4.0 https://creativecommons.org/licenses/by/4.0/
*/

// Lib
// ===

#include <USBComposite.h>
#include "HX711.h"
#include "RunningAverage.h"


// Constants
// ===

#define SENSOR_COUNT 4

// Remember analog readings over this many readings, to calculate rate of change.
#define SENSOR_DERIVATIVE_READINGS 3

// Analog Input pins receiving the HX711 readings. 1 per step.
const byte sensorInPins[SENSOR_COUNT] = {4, 5, 6, 7};

// PVM Output pins for communicating with HX711.
const byte sensorOutPins[SENSOR_COUNT] = {8, 9, 10, 11};

// Keycodes to emit when respective sensors are pressed
// Right: 215, Left: 216, Down: 217, Up: 218
const byte sensorKeycodes[SENSOR_COUNT] = {215, 216, 217, 218}; // R L D U

// Derivative strategy measures rate of change of sensors instead of single values.
// It's not perfected yet, so only turn it on as a fallback!
const bool sensorShouldUseDerivative[SENSOR_COUNT] = {false, false, false, false}; // R L D U

// Calibration factor for sensor to measure accurately 1 kg.
const float sensorCalibrations[SENSOR_COUNT] = {-22000, -22000, -22000, -22000};

// Threshold after which key event is sent
const float sensorThresholds[SENSOR_COUNT] = {3.0, 3.0, 3.0, 3.0};

// Derivative threshold after which key event is sent.
const float sensorDerivativeThresholds[SENSOR_COUNT] = {1.0, 1.0, 1.0, 1.0};


// Variables
// ===

float sensorReadings[SENSOR_COUNT];

HX711 *sensors [SENSOR_COUNT];

// Remember previous readings to compensate for sticky sensor readings.
RunningAverage *sensorChangeHistory [SENSOR_COUNT];

// Store if sensors were down the last reading cycle,
// to allow us to send key Press and Release events.
bool sensorLastDown[SENSOR_COUNT];


void setup() {
  // Delay because I dunno
  delay(1000);
  
  // Ignored by Maple. But needed by boards using Hardware serial via a USB to Serial Adaptor
  Serial.begin(115200);
  USBHID_begin_with_serial(HID_KEYBOARD);
  // useful to detect host capslock state and LEDs
  Keyboard.begin();

  for (byte n = 0; n < SENSOR_COUNT; n++) {
    CompositeSerial.print("setup() sensor ");
    CompositeSerial.print(n);
    CompositeSerial.print("; ");
    
    sensorReadings[n] = 0;
    sensorLastDown[n] = false;
    sensors[n] = new HX711(sensorInPins[n], sensorOutPins[n], 128);
    
    sensors[n]->set_scale();
    sensors[n]->tare(); // Reset scale to 0

    // Get baseline reading
    long zero_factor = sensors[n]->read_average();
    CompositeSerial.print("Zero factor: ");
    CompositeSerial.println(zero_factor);
    
    sensors[n]->set_scale(sensorCalibrations[n]);
 
    sensorChangeHistory[n] = new RunningAverage(SENSOR_DERIVATIVE_READINGS);
  }

  // Delay because I dunno
  delay(1000);
}

void loop() {
  for (byte n = 0; n < SENSOR_COUNT; n++) {
    loopSensor(n);
  }

  // Formatting
  CompositeSerial.print("\n");
}

void loopSensor(int sensorId) {
  float rawValue = sensors[sensorId]->get_units();
  float previousValue = sensorReadings[sensorId];
  float change = rawValue - previousValue;
  sensorReadings[sensorId] = rawValue;

  sensorChangeHistory[sensorId]->addValue(change);
  float derivative = sensorChangeHistory[sensorId]->getAverage();

  CompositeSerial.print("S");
  CompositeSerial.print(sensorId);
  CompositeSerial.print(": ");
  CompositeSerial.print(rawValue);
  CompositeSerial.print("(d ");
  CompositeSerial.print(derivative);
  CompositeSerial.print("); ");

  bool pressed = false;
  if (sensorShouldUseDerivative[sensorId]) {
    pressed = (derivative > sensorDerivativeThresholds[sensorId]);
  } else {
    pressed = (rawValue > sensorThresholds[sensorId]);
  }
  if (pressed) {
    if (sensorLastDown[sensorId] != true) {
      Keyboard.press(sensorKeycodes[sensorId]);
      
      CompositeSerial.print("Sensor ");
      CompositeSerial.print(sensorId);
      CompositeSerial.print(": PRESS; ");
      CompositeSerial.print(rawValue);
      CompositeSerial.print("\n");
    }
    sensorLastDown[sensorId] = true;
    return;
  }

  bool unpressed = false;
  if (sensorShouldUseDerivative[sensorId]) {
    unpressed = (derivative < (-1 * sensorDerivativeThresholds[sensorId]));
  } else {
    unpressed = !pressed;
  }
  if (unpressed) {
    if (sensorLastDown[sensorId] != false) {
      Keyboard.release(sensorKeycodes[sensorId]);

      CompositeSerial.print("Sensor ");
      CompositeSerial.print(sensorId);
      CompositeSerial.print(": RELEASE; ");
      CompositeSerial.print(rawValue);
      CompositeSerial.print("\n");
    }
    sensorLastDown[sensorId] = false;
    return;
  }
}

