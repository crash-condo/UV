//Math Concept: Peak Detection
//In mathematics, detecting spikes or outliers is often referred to as peak detection
// or outlier detection. A simple way to implement this is by comparing each new reading
// against a dynamically adjusted threshold based on the running average:
//
// Peak Threshold=μ+k×σ

#include <IRremote.hpp>

// IR codes for Panasonic TV (as captured)
#define IR_PANASONIC_ADDRESS 0x8
#define IR_PANASONIC_VOLUME_UP 0x20
#define IR_PANASONIC_VOLUME_DOWN 0x21
#define IR_SEND_PIN 2

const int REQ_CONSEC_COUNT = 5;
const int COOLOFF = 5000;
const int CBUFF_SIZE_10 = 10;
const int CBUFF_SIZE_5 = 5;
const float REF_AMP = 8.00;
const int REF_dB = 25;
const float K = 1.5; //sensitivity constant tweak as needed.


float currentReading = 0;
float decibels = 0;


// AmpSpikes Struct (organization for nomenclation ie. can use peak.threshold)
struct Threshold {

  float threshold;
  
  void calcPeakT(float avg, float k, float stdd) {
    
    //return (avg + (k * stdd));

    threshold = avg + (k * stdd);
  }
  
  void calcDipT(float avg, float k, float stdd) {
    
    //return (avg + (k * stdd));

    threshold = avg - (k * stdd);
  }

};

struct StandardDeviation {

  float current;

  void calc(float avg, float *array) {

    float variance = 0;
    float stddev = 0;

    //Calculate Variance
    for (int i = 0; i < CBUFF_SIZE_10; i++) {
      variance += pow(array[i] - avg, 2);
    }
    variance /= CBUFF_SIZE_10;
    
    current = sqrt(variance);
  }
};

// Exponential Moving Average Struct
struct ExponentialMovingAverage {
  float alpha = 2.0 / (CBUFF_SIZE_10 + 1);   // alpha formula is 2 / N + 1, where N is number of units in average.
  float current;          // Current EMA value
  float previous;          // Previous EMA value
  float delta;      // diff of current and previous
  bool firstCall = true;  // Flag to track if it's the first call

  // Member function to calculate the EMA
  void calc(float avg, float cr) {
    if (firstCall) {
      previous = avg;  // Use average for the first call
      firstCall = false;
    } else {
      previous = current;  // Use previous EMA for subsequent calls
    }
    current = (cr * alpha) + (previous * (1 - alpha));  // Calculate new EMA
    delta = current - previous;                // Calc new delta
  }
};

//Circular Buffer Struct for running averages
//FORMAT: CircBuff <INSTANCENAME>(SIZE), where size is the array size you want inside the instance.
struct CircBuff {
    float *array;   // Pointer to dynamically allocated array
    int arraySize;  // Size of the buffer
    int index = 0;  // Current index in the buffer
    float rTotal = 0;  // Running total
    float average = 0; // Running average
    struct StandardDeviation stdDev;
    struct ExponentialMovingAverage EMA;

    // Constructor: Initialize the buffer with a given size
    CircBuff(int SIZE) {
      arraySize = SIZE;
      array = new float[arraySize];  // Dynamically allocate memory for the array
      for (int i = 0; i < arraySize; i++) {
        array[i] = 0;  // Initialize all elements to zero
      }
    }

    void cycleBuffer(float cr) {
      //Circular Buffer Readings for CircBuff readings struct instance
      rTotal -= array[index];                     // subtract old value from running total
      array[index] = cr;                          // overwrite old value with new value at index
      rTotal += cr;                               // add new value to running total
      index = (index + 1) % CBUFF_SIZE_10;        // Wraps around when index reaches BUFFER_SIZE
      average = rTotal / (float)CBUFF_SIZE_10;    // calc average

      EMA.calc(average, cr);
      stdDev.calc(average, array);
    }
};

//Struct Instantiation
struct Threshold peak;
struct Threshold dip;
struct CircBuff readings(CBUFF_SIZE_10);
// TODO: struct CircBuff decibels(CBUFF_SIZE_10);

// Precall functions for availability
void procSound();
void printSoundStats();
float calcDecibels();

void setup() {
 
  Serial.begin(9600);

  // Initialize the sender
  IrSender.begin(IR_SEND_PIN);
  
}

void loop() {
 
  currentReading = analogRead(A0);
  procSound(currentReading);
  decibels = calcDecibels(currentReading);

  // if ((currentReading > amplitude.current) && (amplitude.delta > 0.2)) {

  //   Serial.print("\n\nHIGH SPIKE\n\n");
  //   printSoundStats();
  //   IrSender.sendPanasonic(IR_PANASONIC_ADDRESS, IR_PANASONIC_VOLUME_DOWN, 0);

  // }

  // if ((currentReading < amplitude.current) && (amplitude.delta < -0.2)) {
    
  //   Serial.print("\n\nLOW SPIKE\n\n");
  //   printSoundStats();
  //   IrSender.sendPanasonic(IR_PANASONIC_ADDRESS, IR_PANASONIC_VOLUME_UP, 0);

  // }  

  printSoundStats();
  delay(200);
}

void procSound(float cr) {

  readings.cycleBuffer(cr);

  // Set Peak and Dip Threshold Determination
  peak.calcPeakT(readings.average, K, readings.stdDev.current);
  dip.calcDipT(readings.average, K, readings.stdDev.current);
  
}

float calcDecibels(float cr) {
  return REF_dB + 20 * log10(float(cr) / REF_AMP);
}

void printSoundStats() {

  Serial.print("dB: ");
  Serial.print(decibels);
  Serial.print("     CR: ");
  Serial.print(currentReading);
  Serial.print("     CR_AVG: ");
  Serial.print(readings.average);
  Serial.print("     StdDev: ");
  Serial.print(readings.stdDev.current);
  Serial.print("     PT: ");
  Serial.print(peak.threshold);
  Serial.print("     DT: ");  
  Serial.print(dip.threshold);
  Serial.print("     cEMA: ");
  Serial.print(readings.EMA.current);
  Serial.print("     pEMA: ");
  Serial.print(readings.EMA.previous);
  Serial.print("     deltaEMA: ");
  Serial.println(readings.EMA.delta);
  Serial.print("\r");

}