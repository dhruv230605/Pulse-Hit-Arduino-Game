const int fsrPin = A3;  // FSR connected to analog pin A3
int initialFsrReading = 0;

void setup() {
  Serial.begin(9600);  // Start serial communication
  pinMode(fsrPin, INPUT);
  Serial.println("FSR Change from Initial Value Detection...");

  delay(500);  // Let the FSR stabilize
  initialFsrReading = analogRead(fsrPin);  // Store initial value
  Serial.print("Initial FSR Value: ");
  Serial.println(initialFsrReading);
}

void loop() {
  int currentFsrReading = analogRead(fsrPin);

  if (currentFsrReading != initialFsrReading) {
    Serial.print("FSR Changed! New Value: ");
    Serial.println(currentFsrReading);
  }

  delay(100);  // Avoid spamming
}
