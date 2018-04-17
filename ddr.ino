/*
  ika DDR pad test
*/

// Lib
// ===

#include <USBComposite.h>
#include "RunningAverage.h"


// Constants
// ===

#define SENSOR_COUNT 1
// Average analog readings over this many readings, to smooth it out.
#define SMOOTH_CYCLES 100

// Analog pins for each sensor
const byte sensorInPins[SENSOR_COUNT] = {10};

// Keycodes to emit when respective sensors are pressed
const byte sensorKeycodes[SENSOR_COUNT] = {88}; // X


// Variables
// ===

// Threshold after which KeyDown event is sent.
// TODO: Runtime calibration
int sensorThresholds[SENSOR_COUNT];

int sensorReadings[SENSOR_COUNT];

// Smooth over several readings to compensate for noise.
RunningAverage *smoothReadings [SENSOR_COUNT];

// Store if sensors were down the last reading cycle,
// to allow us to send key Press and Release events.
bool sensorLastDown[SENSOR_COUNT];


void setup() {
  for (byte n = 0; n < SENSOR_COUNT; n++) {
    pinMode(sensorInPins[n], INPUT_ANALOG);
    sensorReadings[n] = 0;
    sensorLastDown[n] = false;
    // TODO: Runtime calibration
    sensorThresholds[n] = 150;
    smoothReadings[n] = new RunningAverage(SMOOTH_CYCLES);
  }

  // Ignored by Maple. But needed by boards using Hardware serial via a USB to Serial Adaptor
  Serial.begin(115200);

  USBHID_begin_with_serial(HID_KEYBOARD);
  Keyboard.begin(); // useful to detect host capslock state and LEDs

  // Delay because I dunno
  delay(1000);
}

void loop() {
  for (byte n = 0; n < SENSOR_COUNT; n++) {
    loopSensor(n);
  }
}

void loopSensor(int sensorId) {
  int sensorValue = analogRead( sensorInPins[sensorId] );
  smoothReadings[sensorId]->addValue(sensorValue);
  int smoothAverage = int(smoothReadings[sensorId]->getAverage());

  if (smoothAverage > sensorThresholds[sensorId]) {
    if (sensorLastDown[sensorId] != true) {
      Keyboard.press(sensorKeycodes[sensorId]);
      
      CompositeSerial.print("Sensor ");
      CompositeSerial.print(sensorId);
      CompositeSerial.print(": Press\n");
    }
    sensorLastDown[sensorId] = true;

  } else {
    if (sensorLastDown[sensorId] != false) {
      Keyboard.release(sensorKeycodes[sensorId]);

      CompositeSerial.print("Sensor ");
      CompositeSerial.print(sensorId);
      CompositeSerial.print(": Release\n");
    }
    sensorLastDown[sensorId] = false;
  }

  CompositeSerial.print("Sensor ");
  CompositeSerial.print(sensorId);
  CompositeSerial.print(", Smooth reading: ");
  CompositeSerial.print(smoothAverage);
  CompositeSerial.print(")\n");
}

