#include <UVD.h>

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
    // Check if the button has been held for at least BUTTON_HOLD_TIME milliseconds
    // If so, enter config mode
    if (millis() - pressStartTime >= BUTTON_HOLD_TIME) {

      enterConfigMode();
      
      isButtonPressed = false; // Reset flag to avoid retriggering until button is released
    }
  } else { // Button is not pressed (released)
    isButtonPressed = false; // Reset flag when button is released
  }
}
