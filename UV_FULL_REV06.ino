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

const int POTPIN = A5; // potentiometer
const int REQ_CONSEC_COUNT = 5;
const int COOLOFF = 5000;
const int CBUFF_SIZE_10 = 10;
const int CBUFF_SIZE_5 = 5;
const float REF_AMP = 8.75;
const int REF_dB = 25;
float K = 1.5; //sensitivity for peak dip determination constant tweak as needed.


float currentReading = 0;

struct Timer {
    unsigned long startTime = 0;   // Time when the timer was started or last reset
    unsigned long duration = 0;    // Duration for the timer (in milliseconds)
    bool isRunning = false;        // Flag to indicate if the timer is active
    bool autoReset = false;        // Flag to indicate if the timer should auto-reset after elapsing

    // Constructor with optional auto-reset
    Timer(bool autoReset = false) 
        : autoReset(autoReset) {}

    // Start or reset the timer with a specific duration (in milliseconds)
    void start(unsigned long duration) {
        this->duration = duration;
        this->startTime = millis();
        this->isRunning = true;
    }

    // Stop the timer
    void stop() {
        isRunning = false;
    }

    // Reset the timer without changing the duration
    void reset() {
        startTime = millis();
        isRunning = true;
    }

    // Check if the timer has elapsed
    bool hasElapsed() {
        if (!isRunning) return false;
        
        if (millis() - startTime >= duration) {
            if (autoReset) reset();  // Auto-reset if enabled
            else stop();  // Stop the timer if not auto-resetting
            
            return true;
        }
        
        return false;
    }

    // Check how much time has passed since the timer started
    unsigned long timePassed() const {
        return millis() - startTime;
    }

    // Check how much time is left until the timer elapses
    unsigned long timeLeft() const {
        if (!isRunning || hasElapsed()) return 0;
        return duration - (millis() - startTime);
    }

    // Set whether the timer should automatically reset after elapsing
    void setAutoReset(bool autoReset) {
        this->autoReset = autoReset;
    }

    // Start a timer with a duration specified in seconds
    void startSeconds(unsigned long seconds) {
        start(seconds * 1000);  // Convert seconds to milliseconds
    }

    // Start a timer with a duration specified in microseconds
    void startMicroseconds(unsigned long microseconds) {
        start(microseconds / 1000);  // Convert microseconds to milliseconds
    }
};

struct Sensitivity {
    int potPin;      // Analog pin connected to the potentiometer
    float minK;    // Minimum value for sensitivity (K)
    float maxK;    // Maximum value for sensitivity (K)
    float K;           // Current sensitivity value
    bool invertDial;
    float scale;    // Scale factor for finer control over K
    int pMin = 0;       // potentiometer minimum reading
    int pMax = 1024;       // potentiometer maximum reading

    // Constructor to initialize the struct with pin and sensitivity range
    Sensitivity(int potPin, float minK, float maxK, bool invertDial = false)
        : potPin(potPin), minK(minK), maxK(maxK), K(0), invertDial(invertDial) {}


    // Method to update the sensitivity value based on potentiometer reading and scale it to minK/maxK
    void update() {

      K = minK + ((float)(analogRead(potPin) - pMin) / (pMax - pMin)) * (maxK - minK);

      if (invertDial) {
        K = maxK - K;
      }

    }
    // Optional: Method to print current sensitivity value for debugging
    void printK() {
        Serial.print("Current Sensitivity (K): ");
        Serial.println(K);
    }
};

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
  float alpha = 2.0 / (CBUFF_SIZE_10 + 1);  // alpha formula is 2 / N + 1, where N is number of units in average.
  float current;                            // Current EMA value
  float previous;                           // Previous EMA value
  float delta;                              // diff of current and previous
  bool firstCall = true;                    // Flag to track if it's the first call

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

struct Decibels {

  float current = 0;
  
  void calc(float cr) {

    current = REF_dB + 20 * log10(cr/ REF_AMP);

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
    struct Decibels dB;

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
      dB.calc(cr);
    }
};

//Struct Instantiation
struct Threshold peak;
struct Threshold dip;
struct CircBuff readings(CBUFF_SIZE_10);
//struct CircBuff decibels(CBUFF_SIZE_10);
struct Timer volUpLimiter;
struct Timer volDownLimiter;
struct Sensitivity dial(POTPIN, 0.00, 5.00, true);

// Precall functions for availability
void procSound();
void printSoundStats();
float calcDecibels();

void setup() {
 
  Serial.begin(9600);

  // Initialize the sender
  IrSender.begin(IR_SEND_PIN);
  
  volUpLimiter.startSeconds(2);
  volDownLimiter.startSeconds(2);
}

void loop() {

  currentReading = analogRead(A0);
  
  dial.update();
  dial.printK();
  readings.dB.calc(currentReading);

  procSound(readings.dB.current); // sending decibels into readings CircBuff, whatever is sent into procSound gets placed in CircBuff for readings array

  if ((readings.dB.current > peak.threshold) && (readings.EMA.delta > dial.K) && (volDownLimiter.hasElapsed())) {

    Serial.print("\n\nHIGH SPIKE\n\n");
    printSoundStats();
    IrSender.sendPanasonic(IR_PANASONIC_ADDRESS, IR_PANASONIC_VOLUME_DOWN, 0);
    volDownLimiter.reset();

  }

  if ((readings.dB.current < dip.threshold) && (readings.EMA.delta < -dial.K) && (volUpLimiter.hasElapsed())) {
    
    Serial.print("\n\nLOW SPIKE\n\n");
    printSoundStats();
    IrSender.sendPanasonic(IR_PANASONIC_ADDRESS, IR_PANASONIC_VOLUME_UP, 0);
    volUpLimiter.reset();

  }  

  printSoundStats();
  delay(200);
}

void procSound(float cr) {

  readings.cycleBuffer(cr);

  // Set Peak and Dip Threshold Determination
  peak.calcPeakT(readings.average, dial.K, readings.stdDev.current);
  dip.calcDipT(readings.average, K, readings.stdDev.current);
  
}

void printSoundStats() {

  Serial.print("dB: ");
  Serial.print(readings.dB.current);
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