#ifndef IR_SENSOR_H
#define IR_SENSOR_H

#include <Arduino.h>

class IR_Sensor {
  private:
    int pin;
    int threshold;
    const int numReadings = 10; // Number of readings to average
    int readings[10];           // Array to store readings
    int readIndex = 0;          // Index of the current reading
    int total = 0;              // Sum of the readings
    int average = 0;            // Rolling average

    // Method to find the median of the readings
    int findMedian();
    
    // Method to despike the readings
    bool isSpiked(int newValue);

    // Method to sort an array (bubble sort)
    void bubbleSort(int arr[], int n);

  public:
    // Constructor
    IR_Sensor(int inputPin, int inputThreshold);

    // Method to read the sensor value and update the rolling average
    void update();

    // Method to check if the average value is below the threshold
    bool isBelowThreshold();

    // Method to get the current average value
    int getAverage();
};

#endif
