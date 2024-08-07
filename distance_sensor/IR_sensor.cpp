#include "IR_Sensor.h"

// Constructor
IR_Sensor::IR_Sensor(int inputPin, int inputThreshold) {
  pin = inputPin;
  threshold = inputThreshold;
  pinMode(pin, INPUT); // Initialize each sensor's pin
  for (int i = 0; i < numReadings; i++) {
    readings[i] = 0; // Initialize readings array to 0
  }
}

// Method to read the sensor value and update the rolling average
void IR_Sensor::update() {
  // Read the new value
  int newValue = analogRead(pin);
  
  // Check if the new value is a spike
//  if (isSpiked(newValue)) {
//    return; // Ignore the spiked value
//  }
  
  // Subtract the last reading
  total = total - readings[readIndex];
  // Add the new reading to the array
  readings[readIndex] = newValue;
  // Add the new reading to the total
  total = total + readings[readIndex];
  // Advance to the next position in the array
  readIndex = (readIndex + 1) % numReadings;
  // Calculate the average
  average = total / numReadings;
}

// Method to find the median of the readings
int IR_Sensor::findMedian() {
  int sortedReadings[numReadings];
  memcpy(sortedReadings, readings, sizeof(readings));
  bubbleSort(sortedReadings, numReadings);
  return sortedReadings[numReadings / 2];
}

// Method to despike the readings
bool IR_Sensor::isSpiked(int newValue) {
  int median = findMedian();
  int threshold = 100; // Set an appropriate threshold for despiking
  return abs(newValue - median) > threshold;
}

// Method to check if the average value is below the threshold
bool IR_Sensor::isBelowThreshold() {
  return (average > threshold);
}

// Method to get the current average value
int IR_Sensor::getAverage() {
  return average;
}

// Method to sort an array (bubble sort)
void IR_Sensor::bubbleSort(int arr[], int n) {
  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - i - 1; j++) {
      if (arr[j] > arr[j + 1]) {
        int temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
    }
  }
}
