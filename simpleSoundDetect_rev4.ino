  
  #include <IRremote.hpp>
  
  // IR codes for Panasonic TV (as captured)
  #define IR_PANASONIC_ADDRESS 0x8
  #define IR_PANASONIC_VOLUME_UP 0x20
  #define IR_PANASONIC_VOLUME_DOWN 0x21
  #define IR_SEND_PIN 2
  
  const int numReadings = 25; // Size of the circular buffer
  int readings[numReadings];  // Array to store the readings
  int index = 0;              // Current index in the circular buffer
  long total = 0;             // Sum of the readings
  float average = 0;          // Calculated average
  
  void setup() {
    Serial.begin(9600);
    IrSender.begin(IR_SEND_PIN);
  }
  
  void loop() {
    // Subtract the oldest reading from the total
    total -= readings[index];
  
    // Read a new value from the analog input
    readings[index] = analogRead(A5);
  
    // Add the new reading to the total
    total += readings[index];
  
    // Move to the next position in the buffer
    index = (index + 1) % numReadings;
  
    // Calculate the average
    average = total / (float)numReadings;
  
  
    // Control Volume
    if (average < 7) {
      IrSender.sendPanasonic(IR_PANASONIC_ADDRESS, IR_PANASONIC_VOLUME_UP, 0);
      Serial.println("Too Quiet, Sent Volume Up");
    }
  
    
    // Send volume down command
    if (average >= 7.9) {
      IrSender.sendPanasonic(IR_PANASONIC_ADDRESS, IR_PANASONIC_VOLUME_DOWN, 0);
      Serial.println("Too Loud! Sent Volume Down");
      
    }
    
    
    // Print sound level to Serial Monitor
    Serial.println(average);
  
    delay(250);
  
  }
  
  //void volumeDown(
