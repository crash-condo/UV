#include <IRremote.hpp>  // Ensure you're using the latest version of IRremote

const int IR_SEND_PIN = 2;      // Pin for sending IR signals
const int GREEN_LED = 3;        // Green LED connected to pin 3
const int BUTTON_PIN = 5;       // Push button connected to digital pin 5
const int IR_RECEIVE_PIN = 11;  // Pin for receiving IR signals

unsigned long input_code;
uint8_t bits;
decode_type_t protocolNumber;
uint16_t irAddress;
String protocol;
uint16_t keyCommand;

void blinkGREEN(int times);
void getCode();
void sendCode();

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Set button pin as INPUT_PULLUP
  
  // Initialize the receiver
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  // Initialize the sender
  IrSender.begin(IR_SEND_PIN);
}

void loop() {
  
  getCode();

  // Wait for button press to retransmit
  if (digitalRead(BUTTON_PIN) == LOW) {

    sendCode();

  }
}

// Function to blink LED a specified number of times
void blinkGREEN(int times) {
  for(int i = 0; i < times; i++)
  {
    digitalWrite(GREEN_LED, HIGH);   // Turn LED on
    delay(50);                    // Wait for 200ms
    digitalWrite(GREEN_LED, LOW);    // Turn LED off
    delay(50);                    // Wait for 200ms
  }
}

void getCode() {
  // Check if an IR signal is received
  if (IrReceiver.decode()) {
    Serial.println("IR signal received!");
    blinkGREEN(5);

    // Print out human-readable format for debugging (optional)
    IrReceiver.printIRResultShort(&Serial);
    IrReceiver.printIRSendUsage(&Serial);

    // Check which protocol was used and store relevant input_code
    input_code = IrReceiver.decodedIRData.decodedRawData;
    keyCommand = IrReceiver.decodedIRData.command;
    bits = IrReceiver.decodedIRData.numberOfBits;
    protocolNumber = IrReceiver.decodedIRData.protocol;
    irAddress = IrReceiver.decodedIRData.address;
    protocol = getProtocolString(protocolNumber);

    Serial.println(" ");
    
    // Fixing the printing issues by separating each part
    Serial.print("Protocol Number: ");
    Serial.println((int)protocolNumber);  // Cast enum to int for printing
    delay(50);
    
    Serial.print("Protocol Name: ");
    Serial.println(protocol);  // No need to concatenate, just print
    delay(50);

    Serial.print("Key Command: ");
    Serial.println(keyCommand, HEX);
    delay(50);
    
    Serial.print("IR Address: ");
    Serial.println(irAddress, HEX);  // Print address in hexadecimal format
    delay(50);
    
    Serial.print("Input Code: ");
    Serial.println(input_code, HEX);  // Print input code in hexadecimal format
    delay(50);
    
    Serial.print("Bits: ");
    Serial.println(bits);  // Print number of bits directly
    delay(50);

    // Resume receiving next signal
    IrReceiver.resume();
    delay(250);
    Serial.print("IR Receiver Resumed Properly");
    return;
  }
}

  void sendCode() {
      
    Serial.println("Retransmitting...");

    // Retransmit based on detected protocol
    switch (protocolNumber) {
        case NEC:
            // Disable the receiver temporarily
            IrReceiver.stop();
            Serial.println("NEC - Pausing IR RECV");
            delay(250);
            
            IrSender.sendNEC(irAddress, keyCommand, 0);
            delay(250);

            // Re-enable the receiver after sending
            IrReceiver.start();
            Serial.println("NEC - Resuming IR RECV");
            break;
        case SONY:
            // Disable the receiver temporarily
            IrReceiver.stop();
            Serial.println("Sony - Pausing IR RECV");
            delay(250);
            
            IrSender.sendSony(irAddress, keyCommand, 0, bits);
            delay(250);

            // Re-enable the receiver after sending
            IrReceiver.start();
            Serial.println("Sony - Resuming IR RECV");
            break;
        case RC5:
            IrSender.sendRC5(input_code, bits);
            break;
        case RC6:
            IrSender.sendRC6((uint32_t)input_code, bits);
            break;
        case JVC:
            IrSender.sendJVC((uint8_t)irAddress, (uint8_t)input_code, 0);  
            break;
        case SAMSUNG:
            IrSender.sendSamsung(irAddress, (uint8_t)input_code, 0);  
            break;
        case LG:
            IrSender.sendLG(input_code, bits);
            break;
        case WHYNTER:
            IrSender.sendWhynter(input_code, bits);
            break;
        case PANASONIC:
            // Disable the receiver temporarily
            IrReceiver.stop();
            Serial.println("Panasonic - Pausing IR RECV");
            delay(250);
            
            // Send the Panasonic signal
            IrSender.sendPanasonic(irAddress, keyCommand, 0);
            delay(250);
            
            // Re-enable the receiver after sending
            IrReceiver.start();
            Serial.println("Panasonic - Resuming IR RECV");
            break;
        case DENON:
            IrSender.sendDenon(input_code, bits);
            break;
        case BOSEWAVE:
            IrSender.sendBoseWave(input_code, bits);
            break;
        case LEGO_PF:
            IrSender.sendLegoPowerFunctions(input_code, bits);
            break;
        default:
            Serial.println("Unknown protocol or raw input code. Cannot retransmit.");
            break;
    }
    delay(200); // Debounce delay for button press
    Serial.println("Switch Cases left, loop resarting");

  }

  void getSoundLevel
  
