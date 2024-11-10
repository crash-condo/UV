//Math Concept: Peak Detection
//In mathematics, detecting spikes or outliers is often referred to as peak detection or outlier detection. 
//A simple way to implement this is by comparing each new reading against a dynamically adjusted threshold based on the running average:
// Peak Threshold=μ+k×σ


const int CB_SIZE = 10;


int readingsCB[CB_SIZE];  // Circular Buffer Array for analog readings



void ProcSound();
float peakThresholdDetect();
float getStdDev();
float getSensitivityConstant();


void setup() {
  // put your setup code here, to run once:

  
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:

  procSound();

}


void procSound() {

    float total = 0;
    float average = 0;
    float sd = 0; //Standard Deviation
    float sc = 0; //Sensitivity Constant
    float pt = 0; //Peak Threshold


    for (int i = 0; i < CB_SIZE; i++) {
    //Fill Circular Buffer
        readingsCB[i] = analogRead(A0);
        total += readingsCB[i];
    }

    // Calculate running average
    average = total / (float)CB_SIZE;

    // Get Standard Deviation

    sd = getStdDev(average);

    // Get Sensitivity constant

    sc = getSensitivityConstant(sd);

    // Get Peak/Spkie Threshold Determination

    pt = peakThresholdDetect(average, sc, sd);


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


float peakThresholdDetect(float avg, float k, float stdd) {
    
    return (avg + (k + stdd));

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
