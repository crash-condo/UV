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
const int READINGS_CBUFF_SIZE = 10;
const int PEAKS_CBUFF_SIZE = 5;
const float REF_AMP = 9.17;
const int REF_dB = 25;

// Initialize circular buffer array and loop variables

//float peaksCB[PEAKS_CBUFF_SIZE];        // Circular Buffer Array for peak threshold smoothing

float currentReading = 0;
float decibels = 0;
float sd;       //Standard Deviation
float sc;       //Sensitivity Constant



//Circular Buffer Struct for running averages
//FORMAT: CircBuff <INSTANCENAME>(SIZE), where size is the array size you want inside the instance.
struct CircBuff {
    float* array;   // Pointer to dynamically allocated array
    int arraySize;  // Size of the buffer
    int index = 0;  // Current index in the buffer
    float rTotal = 0;  // Running total
    float average = 0; // Running average

    // Constructor: Initialize the buffer with a given size
    CircBuff(int SIZE) {
      arraySize = SIZE;
      array = new float[arraySize];  // Dynamically allocate memory for the array
      for (int i = 0; i < arraySize; i++) {
        array[i] = 0;  // Initialize all elements to zero
    }
    }

    // // Destructor: Clean up dynamically allocated memory
    // ~CircBuff() {
    //     delete[] array;  // Free the memory allocated for the array
    // }
};


// Spike Struct (organization for nomenclation ie. can use peak.threshold)

struct AmpSpikes {

  float threshold;
  
  void calcPeakThreshold(float avg, float k, float stdd) {
    
    //return (avg + (k * stdd));

    threshold = avg + (k * stdd);
  }
  
  void calcDipThreshold(float avg, float k, float stdd) {
    
    //return (avg + (k * stdd));

    threshold = avg - (k * stdd);
  }

};

// Exponential Moving Average Struct
struct EMA {
  float alpha = 2.0 / (READINGS_CBUFF_SIZE + 1);   // alpha formula is 2 / N + 1, where N is number of units in average.
  float cEMA;          // Current EMA value
  float pEMA;          // Previous EMA value
  float deltaEMA;      // diff of cEMA and pEMA
  bool firstCall = true;  // Flag to track if it's the first call

  // Member function to calculate the EMA
  void calcEMA(float avg, float cr) {
    if (firstCall) {
      pEMA = avg;  // Use average for the first call
      firstCall = false;
    } else {
      pEMA = cEMA;  // Use previous EMA for subsequent calls
    }
    cEMA = (cr * alpha) + (pEMA * (1 - alpha));  // Calculate new EMA
    deltaEMA = cEMA - pEMA;                // Calc new deltaEMA
  }
};

// TOP STRUCT - ProcessEnvelope contains all variables, substructres and functions needed to
// process signal from envelope analog signal from SparkFun module on A0.
struct ProcessEnvelope {

};

// Struct Instances
AmpSpikes peak;
AmpSpikes dip;
CircBuff readings(READINGS_CBUFF_SIZE);
//CircBuff peak(PEAKS_CBUFF_SIZE)
EMA amplitude;

// Precall functions for availability
void procSound();
void printSoundStats();
float getStdDev();
float getSensitivityConstant();
float calcDecibels();

void setup() {
 
  Serial.begin(9600);

  // Initialize the sender
  IrSender.begin(IR_SEND_PIN);
  
}

void loop() {
 
  currentReading = analogRead(A0);
  procSound(currentReading);

  if ((currentReading > amplitude.cEMA) && (amplitude.deltaEMA > 0)) {

    //Serial.print("\n\nHIGH SPIKE\n\n");
    //printSoundStats();
    //IrSender.sendPanasonic(IR_PANASONIC_ADDRESS, IR_PANASONIC_VOLUME_DOWN, 0);

  }

  if ((currentReading < amplitude.cEMA) && (amplitude.deltaEMA < 0)) {
    
    //Serial.print("\n\nLOW SPIKE\n\n");
    //printSoundStats();
    //IrSender.sendPanasonic(IR_PANASONIC_ADDRESS, IR_PANASONIC_VOLUME_UP, 0);

  }  

  decibels = calcDecibels(currentReading);
  printSoundStats();
  delay(200);

}

void procSound(float cr) {
  // Proccess Sound - procSound()
  // Here's we'll implement the circular buffer using modulo math and leveraging the
  // Arduino forever loop we will cycle the circular buffer endlessly with no endless 
  // integer buildup for the integer.

  //Circular Buffer Readings for CircBuff readings struct instance
  readings.rTotal -= readings.array[readings.index];  // subtract old value from running total
  readings.array[readings.index] = cr;                // overwrite old value with new value at index
  readings.rTotal += cr;                              // add new value to running total
  readings.index = (readings.index + 1) % READINGS_CBUFF_SIZE;          // Wraps around when index reaches BUFFER_SIZE

  // Calculate running average for readings struct instance
  readings.average = readings.rTotal / (float)READINGS_CBUFF_SIZE;

  // get Exponential Moving Average
  amplitude.calcEMA(readings.average, cr);

  // Get Standard Deviation
  sd = getStdDev(readings.average);

  // Get Sensitivity constant
  sc = getSensitivityConstant();

  // Set Peak and Dip Threshold Determination
  peak.calcPeakThreshold(readings.average, sc, sd);
  dip.calcDipThreshold(readings.average, sc, sd);
  

  // //Circular Buffer Peaks
  // rTotalPeaks -= peaksCB[indexPeaks];
  // peaksCB[indexPeaks] = thresh.peak;
  // rTotalPeaks += thresh.peak;
  // indexPeaks = (indexPeaks + 1) % PEAKS_CBUFF_SIZE;

  // //Calculate running peaks average
  // averagePeaks = rTotalPeaks / (float)PEAKS_CBUFF_SIZE;
  
}



float getStdDev(float avg) {

  float variance = 0;
  float stddev = 0;

  //Calculate Variance
  for (int i = 0; i < READINGS_CBUFF_SIZE; i++) {
    variance += pow(readings.array[i] - avg, 2);
  }
  variance /= READINGS_CBUFF_SIZE;

  // Step 3: Calculate Standard Deviation
  stddev = sqrt(variance);

  return stddev;
}

float getSensitivityConstant() {
  if (abs(amplitude.deltaEMA) < 1.0) {
    return 1.2;  // More sensitive for small changes
  } else if (abs(amplitude.deltaEMA) < 5.0) {
    return 1.8;  // Default sensitivity for moderate changes
  } else {
    return 2.4;  // Less sensitive for large changes
  }
}

float calcDecibels(int cr) {
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
  Serial.print(sd);
  Serial.print("     SC: ");
  Serial.print(sc);
  Serial.print("     PT: ");
  Serial.print(peak.threshold);
  Serial.print("     DT: ");  
  Serial.print(dip.threshold);
  Serial.print("     cEMA: ");
  Serial.print(amplitude.cEMA);
  Serial.print("     pEMA: ");
  Serial.print(amplitude.pEMA);
  Serial.print("     deltaEMA: ");
  Serial.println(amplitude.deltaEMA);
  Serial.print("\r");

}