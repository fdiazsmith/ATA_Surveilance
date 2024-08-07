#include <Keyboard.h>
#include "IR_Sensor.h"

// Define the number of sensors
const int numSensors = 1;

// Create an array of IR_Sensor objects
IR_Sensor sensors[numSensors] = {
  IR_Sensor(A0, 200),
//  IR_Sensor(A1, 200),
//  IR_Sensor(A2, 200)
};

bool anyBelowThreshold = false;
bool previousAnyBelowThreshold = false;

void setup() {
  Serial.begin(9600);
  Keyboard.begin();
}

void loop() {
  anyBelowThreshold = false;

  for (int i = 0; i < numSensors; i++) {
    sensors[i].update();
    Serial.print("Sensor ");
    Serial.print(i);
    Serial.print(" Average Value: ");
    Serial.println(sensors[i].getAverage());

    if (sensors[i].isBelowThreshold()) {
      anyBelowThreshold = true;
    }
  }

  if (anyBelowThreshold != previousAnyBelowThreshold) {
    if (anyBelowThreshold) {
      Serial.println("State changed: At least one sensor is below threshold! Pressing 'a'");
      // Press 'a' if any sensor is below the threshold
//      Keyboard.press('a');
//      delay(100); // Hold the key for 100 milliseconds
//      Keyboard.release('a');
    } else {
      Serial.println("State changed: All sensors are above threshold! Pressing 'd'");
      // Press 'd' if all sensors are above the threshold
//      Keyboard.press('d');
//      delay(100); // Hold the key for 100 milliseconds
//      Keyboard.release('d');
    }
    // Update the previous state
    previousAnyBelowThreshold = anyBelowThreshold;
  }

  delay(50); // Wait for a short period before the next loop
}
