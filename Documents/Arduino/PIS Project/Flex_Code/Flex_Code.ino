const int flexPin = A1;  // Flex sensor connected to analog pin A1
int baselineReading = 0;

void setup() {
  Serial.begin(9600);
  pinMode(flexPin, INPUT);

  delay(500);  // Allow time for stabilization
  baselineReading = analogRead(flexPin);  // Capture baseline
}

void loop() {
  int flexReading = analogRead(flexPin);

  if (abs(flexReading - baselineReading) > 5) {
    Serial.println("Bended");
  }

  delay(200);
}
