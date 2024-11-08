#include <IRremote.hpp>  // Ensure you're using the latest version of IRremote

// Define pin numbers
const int IR_SEND_PIN = 2;      // Pin for sending IR signals
const int GREEN_LED = 3;        // Green LED connected to pin 3
const int BUTTON_PIN = 5;       // Push button connected to digital pin 5
const int IR_RECEIVE_PIN = 11;  // Pin for receiving IR signals
const int CB_SIZE = 10;         // Circular Buffer size for Analog Sound readings
const int BUTTON_HOLD_TIME = 5000; // Button hold time to trigger action

// Variables to track button state and timing
bool isButtonPressed = false;
unsigned long pressStartTime = 0;
int buttonState = 0;

// Define a struct to hold the data for each IR input
struct irRecvData {
  unsigned long input_code;      // Store the input code
  uint8_t bits;                  // Number of bits in the IR signal
  decode_type_t protocolNumber;  // Protocol number (from IRremote)
  uint16_t irAddress;            // Address of the IR device
  String protocol;               // Protocol name (dynamic, using String)
  uint16_t keyCommand;           // Command code from the remote
};

// Create a global array of two elements (2x6 structure)
irRecvData volControl[2];

void enterConfigMode();
void blinkGREEN();
void recvIrCode();
void printArray();


void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Set button pin as input with internal pull-up resistor
  pinMode(GREEN_LED, OUTPUT);           // Set LED pin as output
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // initialize IR receiver
}

void loop() {
  // Read the current state of the button (LOW when pressed, HIGH when not pressed)
  buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW) { // Button is pressed
    if (isButtonPressed == false) {   // If this is the first time we detect the press
      isButtonPressed = true;
      pressStartTime = millis(); // Record when the press started
    }
    // Check if the button has been held for at least 5 seconds
    if (millis() - pressStartTime >= BUTTON_HOLD_TIME) {

      enterConfigMode();
      
      isButtonPressed = false; // Reset flag to avoid retriggering until button is released
    }
  } else { // Button is not pressed (released)
    isButtonPressed = false; // Reset flag when button is released
  }
}

// Function to blink green LED a specified number of times with a 'd' delay optional parameter, default delay is 75ms
void blinkGREEN(int times, int d = 75) {
  for(int i = 0; i < times; i++)
  {
    digitalWrite(GREEN_LED, HIGH);   // Turn LED on
    delay(d);
    digitalWrite(GREEN_LED, LOW);    // Turn LED off
    delay(d);
  }
}

void recvIrCode(int vcc) {
  //vcc input represents which column in the array we're storing the codes
  //column 1 is for volume up
  //column 2 is for volume down
  Serial.println("IR signal received!");
  blinkGREEN(5);

  // Print out human-readable format for debugging (optional)
  IrReceiver.printIRResultShort(&Serial);
  IrReceiver.printIRSendUsage(&Serial);

  // Check which protocol was used and store relevant input_code to proper volControl 2x6 array column
  volControl[vcc].protocol = getProtocolString(IrReceiver.decodedIRData.protocol);
  volControl[vcc].protocolNumber = IrReceiver.decodedIRData.protocol;
  volControl[vcc].keyCommand = IrReceiver.decodedIRData.command;
  volControl[vcc].input_code = IrReceiver.decodedIRData.decodedRawData;
  volControl[vcc].irAddress = IrReceiver.decodedIRData.address;
  volControl[vcc].bits = IrReceiver.decodedIRData.numberOfBits;

  // Resume receiving next signal
  IrReceiver.resume();
  delay(200);
  Serial.println("Input Recorded");
  Serial.println("");
  return;

}

// Function to enter config mode and wait for IR input
void enterConfigMode() {
  Serial.println("Entering config mode...");
  blinkGREEN(5);
  delay(250);
  digitalWrite(GREEN_LED, HIGH);                 // Turn on LED to indicate config mode
  bool waitingForInput = true;
  int arrayColumn = 0;

  for (int i = 0; i < 2; i++) {
    while (waitingForInput) {
      if (IrReceiver.decode()) {
    
        recvIrCode(arrayColumn);
        arrayColumn += 1;        
        break;
      }
    }

    blinkGREEN(3, 200);
    delay(250);

    if (arrayColumn == 2) {
      digitalWrite(GREEN_LED, LOW);
      Serial.print("Configuration complete, exiting config mode...\n\n");
      printArray();
      return;
    }
    else {
      digitalWrite(GREEN_LED, HIGH);
    }
  }
  //printArray();
  
}

void printArray() {

  for (int i = 0; i < 2; i++) {
    Serial.println("IR Input " + String(i + 1));
    Serial.println("Input Code: " + String(volControl[i].input_code, HEX));
    Serial.println("Bits: " + String(volControl[i].bits));
    Serial.println("Protocol Number: " + String(volControl[i].protocolNumber));
    Serial.println("IR Address: " + String(volControl[i].irAddress, HEX));
    Serial.println("Protocol: " + volControl[i].protocol);   // Using String directly
    Serial.println("Key Command: " + String(volControl[i].keyCommand, HEX));
    Serial.println();
  }

}