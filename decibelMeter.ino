const int envelopePin = A0; // Pin connected to the envelope output
const float referenceAmplitude = 7.5; // Envelope reading for 25 dB SPL
const float referenceDb = 25.0; // Reference dB level for quiet room

void setup() {
  Serial.begin(9600); // Start serial communication for debugging
}

void loop() {
  float envelopeReading = analogRead(envelopePin); // Read envelope value from SparkFun Sound Detector
  
  // Prevent division by zero or negative values
  if (envelopeReading > 0) {
    // Calculate decibel level using the formula
    float decibels = referenceDb + 20 * log10(float(envelopeReading) / referenceAmplitude);

    // Output the results to the Serial Monitor
    Serial.print("Envelope Reading: ");
    Serial.print(envelopeReading);
    Serial.print(" | Decibels: ");
    Serial.println(decibels);
  } else {
    Serial.println("No sound detected or very low sound.");
  }

  delay(100); // Wait for a second before taking another reading
}