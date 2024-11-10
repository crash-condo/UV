//Math Concept: Peak Detection
//In mathematics, detecting spikes or outliers is often referred to as peak detection
// or outlier detection. A simple way to implement this is by comparing each new reading
// against a dynamically adjusted threshold based on the running average:
//
// Peak Threshold=μ+k×σ


const int CB_SIZE = 10;


// Initialize circular buffer array and loop variables

int readingsCB[CB_SIZE];  // Circular Buffer Array for analog readings

float rTotal = 0;   //running total of readingsCB array
float average = 0;
float currentReading = 0;
float sd = 0;       //Standard Deviation
float sc = 0;       //Sensitivity Constant
float pt = 0;       //Peak Threshold
int index = 0;

// Precall functions
void procSound();
void printSoundStats();
float peakThresholdDetect();
float getStdDev();
float getSensitivityConstant();


void setup() {
 
  Serial.begin(9600);
  
}

void loop() {
 
  currentReading = analogRead(A0);
  procSound(currentReading);

  // By association, this throttles the serial print to sample every CB_SIZE loops
  if (index == 8) {
    //Serial.print("TEST");
    delay (200);
    printSoundStats();
  }
  

}

void procSound(float cr) {
  // Proccess Sound - procSound()
  // Here's we'll implement the circular buffer using modulo math and leveraging the
  // Arduino forever loop we will cycle the circular buffer endlessly with no endless 
  // integer buildup for the integer.

  //Circular Buffer
  rTotal -= readingsCB[index];// subtract old value from running total
  readingsCB[index] = cr;// overwrite old value with new value at index
  rTotal += cr;// add new value to running total
  index = (index + 1) % CB_SIZE;// Wraps around when index reaches BUFFER_SIZE
  
  // Calculate running average
  average = rTotal / (float)CB_SIZE;

  // Get Standard Deviation
  sd = getStdDev(average);

  // Get Sensitivity constant
  sc = getSensitivityConstant(sd);

  // Get Peak/Spkie Threshold Determination
  pt = peakThresholdDetect(average, sc, sd);
  
}

float peakThresholdDetect(float avg, float k, float stdd) {
    
  return (avg + (k * stdd));

}

float getStdDev(float avg) {

  float variance = 0;
  float stddev = 0;

  //Calculate Variance
  for (int i = 0; i < CB_SIZE; i++) {
    variance += pow(readingsCB[i] - avg, 2);
  }
  variance /= CB_SIZE;

  // Step 3: Calculate Standard Deviation
  stddev = sqrt(variance);

  return stddev;
}

float getSensitivityConstant(float stddev) {
  if (stddev < 5) {
    return 1.5;  // Low variability, more sensitive
  } else if (stddev < 10) {
    return 2.0;  // Moderate variability
  } else {
    return 3.0;  // High variability, less sensitive
  }
}

void printSoundStats() {

  Serial.print("AVG: ");
  Serial.println(average);
  Serial.print("StdDev: ");
  Serial.println(sd);
  Serial.print("SC: ");
  Serial.println(sc);
  Serial.print("PT: ");
  Serial.println(pt);
  Serial.print("\r");

}
