const int switchPin = 4;     // Switch connected to digital pin 4
int switchState = HIGH;      // Current state of the switch
int lastSwitchState = HIGH;  // Previous state of the switch
unsigned long lastDebounceTime = 0;  // Last time the output pin was toggled
unsigned long debounceDelay = 50;    // Debounce time in milliseconds

void setup() {
  pinMode(switchPin, INPUT_PULLUP); // Enable internal pull-up resistor
  Serial.begin(9600);
}

void loop() {
  // Read the state of the switch
  int reading = digitalRead(switchPin);

  // Check if the switch state changed (due to noise or pressing)
  if (reading != lastSwitchState) {
    // Reset the debouncing timer
    lastDebounceTime = millis();
  }

  // If the reading has been stable for longer than the debounce delay
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the switch state changed
    if (reading != switchState) {
      switchState = reading;

      // Only act if the switch was pressed (LOW because we're using pull-up)
      if (switchState == LOW) {
        Serial.println("Switch pressed!");
        // Add your action here
      }
      else {
        Serial.println("Switch released!");
        // Add your release action here if needed
      }
    }
  }

  // Save the current reading for next loop
  lastSwitchState = reading;
}