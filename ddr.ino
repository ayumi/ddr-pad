/*
  ika DDR pad test
*/

// Constants
// ===

const int analogInPin = 10;

// Average analog readings over this many readings, to smooth it out.
const int numReadings = 10;

// For initial sensor calibration, how many cycles across which to average.
const unsigned int setupCyclesTotal = 65535;


// Variables
// ===

int setupCycles = 0;

// Threshold after which KeyDown event is sent. Calibrated at setup.
float sensorThreshold = 0;
// Set once setup calibration is complete.
bool setupComplete = false;

int sensorValue = 0;


// TODO
//void setup() {
//  USBHID_begin_with_serial(HID_KEYBOARD);
//  Keyboard.begin(); // useful to detect host capslock state and LEDs
//  delay(1000);
//}
//
//void loop() {
//  Keyboard.println("Hello world");
//  delay(10000);
//}

void setup() {
  pinMode(analogInPin, INPUT_ANALOG);

  // Ignored by Maple. But needed by boards using Hardware serial via a USB to Serial Adaptor
  Serial.begin(115200);
}

// Setup tasks requiring multiple cycles, e.g. sensor calibration.
void setupLoop() {
  float sensorRA = 0;
  
  setupCycles += 1;
  sensorValue = analogRead(analogInPin);
  sensorThreshold = (sensorThreshold * (setupCycles - 1) + sensorValue)/(1.0*setupCycles);
  
  Serial.print("setupLoop calibration ");
  Serial.print(setupCycles);
  Serial.print("; reading: ");
  Serial.print(sensorValue);
  Serial.print("; threshold: ");
  Serial.print(sensorThreshold);
  Serial.print("\n");

  if (setupCycles < setupCyclesTotal) {
    return;
  }

  // Finalize
  setupComplete = true;
  Serial.print("Setup completed. Sensor threshold: ");
  Serial.print(sensorThreshold);
  Serial.print("\n");
}

void loop() {
  if (setupComplete != true) {
    setupLoop();
//    return;
  }

  sensorValue = analogRead(analogInPin);

  if (sensorValue > sensorThreshold) {
    Serial.print("sensor GET! value was: " );
    Serial.print(sensorValue);
    Serial.print(" (threshold: ");
    Serial.print(sensorThreshold);
    Serial.print(")\n");
  }
  
  // delay in between reads for stability
  // XXX: Is this necessary?
//  delay(1);
}
