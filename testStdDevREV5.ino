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


const int READINGS_CBUFF_SIZE = 10;
const int PEAKS_CBUFF_SIZE = 5;
const float REF_AMP = 9.17;
const int REF_dB = 25;

// Initialize circular buffer array and loop variables

int readingsCB[READINGS_CBUFF_SIZE];  // Circular Buffer Array for analog readings
float peaksCB[PEAKS_CBUFF_SIZE];        // Circular Buffer Array for peak threshold smoothing

float rTotal = 0;   //running total of readingsCB array
float rTotalPeaks = 0;
float average = 0;
float averagePeaks = 0;
float currentReading = 0;
float decibels = 0;
float sd = 0;       //Standard Deviation
float sc = 0;       //Sensitivity Constant
float pt = 0;       //Peak Threshold
int index = 0;
int indexPeaks = 0;
//Threshold Data Struct
struct Threshold {
  float peak;
  float dip;
  float peakAvg;
  float dipAvg;
};
//Exponential Moving Average Struct
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
// struct instancing
EMA amplitude;
Threshold thresh;

// Precall functions for availability
void procSound();
void printSoundStats();
Threshold peakThresholdDetect();
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

    Serial.print("\n\nHIGH SPIKE\n\n");
    //printSoundStats();
    IrSender.sendPanasonic(IR_PANASONIC_ADDRESS, IR_PANASONIC_VOLUME_DOWN, 0);

  }

  if ((currentReading < amplitude.cEMA) && (amplitude.deltaEMA < 0)) {
    
    Serial.print("\n\nLOW SPIKE\n\n");
    //printSoundStats();
    IrSender.sendPanasonic(IR_PANASONIC_ADDRESS, IR_PANASONIC_VOLUME_UP, 0);

  }  

  decibels = calcDecibels(currentReading);
  //printSoundStats();
  delay(200);

}

void procSound(float cr) {
  // Proccess Sound - procSound()
  // Here's we'll implement the circular buffer using modulo math and leveraging the
  // Arduino forever loop we will cycle the circular buffer endlessly with no endless 
  // integer buildup for the integer.

  //Circular Buffer Readings
  rTotal -= readingsCB[index];// subtract old value from running total
  readingsCB[index] = cr;// overwrite old value with new value at index
  rTotal += cr;// add new value to running total
  index = (index + 1) % READINGS_CBUFF_SIZE;// Wraps around when index reaches BUFFER_SIZE

  // Calculate running average
  average = rTotal / (float)READINGS_CBUFF_SIZE;

  // get Exponential Moving Average
  amplitude.calcEMA(average, cr);

  // Get Standard Deviation
  sd = getStdDev(average);

  // Get Sensitivity constant
  sc = getSensitivityConstant();

  // Get Peak/Dip Threshold Determination
  thresh = peakThresholdDetect(average, sc, sd);

  //Circular Buffer Peaks
  rTotalPeaks -= peaksCB[indexPeaks];
  peaksCB[indexPeaks] = thresh.peak;
  rTotalPeaks += thresh.peak;
  indexPeaks = (indexPeaks + 1) % PEAKS_CBUFF_SIZE;

  //Calculate running peaks average
  averagePeaks = rTotalPeaks / (float)PEAKS_CBUFF_SIZE;
  
}

Threshold peakThresholdDetect(float avg, float k, float stdd) {
    
  //return (avg + (k * stdd));

  Threshold t;
  float kxs = (k * stdd);
  t.peak = (avg + kxs);
  t.dip = (avg - kxs);
  t.peakAvg = 0;
  t.dipAvg = 0;

  return t;

}

float getStdDev(float avg) {

  float variance = 0;
  float stddev = 0;

  //Calculate Variance
  for (int i = 0; i < READINGS_CBUFF_SIZE; i++) {
    variance += pow(readingsCB[i] - avg, 2);
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
  Serial.print("     AVG: ");
  Serial.print(average);
  Serial.print("     StdDev: ");
  Serial.print(sd);
  Serial.print("     SC: ");
  Serial.print(sc);
  Serial.print("     PT: ");
  Serial.print(thresh.peak);
  Serial.print("     AVG pt: ");
  Serial.print(averagePeaks);
  Serial.print("     DT: ");  
  Serial.print(thresh.dip);
  Serial.print("     cEMA: ");
  Serial.print(amplitude.cEMA);
  Serial.print("     pEMA: ");
  Serial.print(amplitude.pEMA);
  Serial.print("     deltaEMA: ");
  Serial.println(amplitude.deltaEMA);
  Serial.print("\r");

}