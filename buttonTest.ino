const int BUTTON_PIN = 5;       // Push button connected to digital pin 5

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Set button pin as INPUT_PULLUP

}

void loop() {
    int buttonState = digitalRead(BUTTON_PIN);
    Serial.print("Button state: ");
    Serial.println(buttonState);  // This will print 0 when pressed, 1 when not pressed

//    if (buttonState == LOW) {
//        Serial.println("Retransmitting...");
//        // Your retransmit code here
//    }
}
