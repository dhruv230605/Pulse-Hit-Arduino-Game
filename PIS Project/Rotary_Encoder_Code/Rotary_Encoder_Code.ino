const int encoderPinA = 12; // Connected to pin 1A
const int encoderPinB = 11; // Connected to pin 3B

int lastEncoded = 0;
int encoderValue = 0;
int lastEncoderValue = 0;

void setup() {
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);
  Serial.begin(9600);
}

void loop() {
  int MSB = digitalRead(encoderPinA);
  int LSB = digitalRead(encoderPinB);

  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue--;

  lastEncoded = encoded;

  // Detect change
  if (encoderValue != lastEncoderValue) {
    Serial.println("Rotated");
    lastEncoderValue = encoderValue;
  }

  delay(10);  // Light delay to smooth output
}
