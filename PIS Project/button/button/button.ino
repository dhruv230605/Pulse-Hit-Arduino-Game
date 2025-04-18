const int buttonPin = 2;  // Pin connected to button
int lastButtonState = digitalRead(buttonPin);  // Previous state of the button

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);  // Use internal pull-up resistor
  Serial.begin(9600);                // Start serial communication
  Serial.println("Button test ready...");
}

void loop() {
  int currentButtonState = digitalRead(buttonPin);
  // Detect falling edge: HIGH -> LOW
  if (currentButtonState != lastButtonState) {
    Serial.println("Button Pressed");
  }

  lastButtonState = currentButtonState;  // Save state for next loop
}
