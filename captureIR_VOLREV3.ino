// Arduino Project: Capture IR Commands for Volume Up and Volume Down

#include <IRremote.h>

// Pin Definitions
const int LED_PIN = 3;            // Red LED connected to digital pin 5
const int BUTTON_PIN = 5;         // Push button connected to digital pin 2
const int IR_RECEIVE_PIN = 11;    // IR Receiver connected to digital pin 11

// Variables to store IR codes
unsigned long VOLUP = 0;
unsigned long VOLDOWN = 0;

void blinkLED(int times);
unsigned long captureIRCommand();

void setup() 
{
  pinMode(LED_PIN, OUTPUT);          // Set LED pin as OUTPUT
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Set button pin as INPUT_PULLUP
  
  digitalWrite(LED_PIN, LOW);        // Ensure LED is off initially
  
  Serial.begin(9600);                // Initialize serial communication
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
}

void loop() 
{
  // Check if the button is pressed
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    blinkLED(1); // Blink LED once to acknowledge button press
    
    // Capture Volume Up command
    VOLUP = captureIRCommand();
    
    blinkLED(2); // Blink LED twice to acknowledge VOLUP capture
    
    // Capture Volume Down command
    VOLDOWN = captureIRCommand();
    
    blinkLED(3); // Blink LED three times to acknowledge VOLDOWN capture
    
    // Print captured codes
    Serial.print("Volume Up Code: ");
    Serial.println(VOLUP, HEX);     // Print as hexadecimal
    
    Serial.print("Volume Down Code: ");
    Serial.println(VOLDOWN, HEX);   // Print as hexadecimal
  }
}

// Function to blink LED a specified number of times
void blinkLED(int times)
{
  for(int i = 0; i < times; i++)
  {
    digitalWrite(LED_PIN, HIGH);   // Turn LED on
    delay(200);                    // Wait for 200ms
    digitalWrite(LED_PIN, LOW);    // Turn LED off
    delay(200);                    // Wait for 200ms
  }
}

// Function to capture IR command and return it as an unsigned long
unsigned long captureIRCommand()
{
  while (true)
  {
    Serial.print("Waiting for IR input...");
    
    for(int i = 0; i != -1; i++)
    {
      Serial.print(".");
      
  
      if (IrReceiver.decode()) {
        unsigned long code = IrReceiver.decodedIRData.decodedRawData; // Capture raw data
        IrReceiver.printIRSendUsage(&Serial); // Capture IRSend Usage
        Serial.println("");
        Serial.print("IR Command Received: ");
        Serial.println(code, HEX);   // Print received code in hexadecimal format
        Serial.println(irSendUsage) // Print IR Send Usage String
        
        IrReceiver.resume();         // Ready to receive the next IR code
        
        return code;    // Return the captured code as an unsigned long
      }

     delay(1000);
    }
  }
}
