//include the library for lcd screen use
#include <LiquidCrystal.h>

// LCD pin connections
const int rs = 5, en = 6, d4 = 7, d5 = 8, d6 = 9, d7 = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//sensor pins
const int switchItPin = 4;     // Switch pin (also used as start trigger)
const int bendItPin = A1;      // Flex sensor pin
const int tapItPin = A3;       // FSR pin
const int encoderPinA = 12;    // Rotary encoder pin A
const int encoderPinB = 11;    // Rotary encoder pin B

// Game state variables
long action;
bool isHardMode = false;  // Default to easy mode

// Rotary encoder variables
int lastEncoded = 0;
int encoderValue = 0;
int lastEncoderValue = 0;
int encoderState1 = 0;
int encoderState2 = 0;

//reference variables for sensor states
int switchItState1;
int switchItState2;
int bendItState1;
int bendItState2;
int tapItState1;
int tapItState2;
int tapCount = 0;  // Counter for tap actions

// time user has to complete action in milliseconds
unsigned long gameRate = 6000;  // Base time for easy mode
unsigned long timeOfPrompt;
unsigned long timeElapsed;

// Game control variables
int keepPlaying = 0;
int actionCompleted = 0;
int score = 0;
int finalScore;

// Track last switch state for start detection
int lastStartSwitchState;

void setup() {
  // Initialize LCD
  lcd.begin(16, 2);
  
  // Initialize serial monitor
  Serial.begin(9600);
  delay(500);

  randomSeed(analogRead(A2));

  // Setting pin modes
  pinMode(switchItPin, INPUT_PULLUP);
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);
  // bendItPin and tapItPin automatically set as inputs due to analog nature

  // Initialize encoder
  encoderState1 = digitalRead(encoderPinA);
  encoderState2 = digitalRead(encoderPinB);
  lastEncoded = (encoderState1 << 1) | encoderState2;

  // Initialize switch state for start detection
  lastStartSwitchState = digitalRead(switchItPin);

  // Display mode selection
  selectDifficulty();
}

void selectDifficulty() {
  bool modeSelected = false;
  int touchValue;
  int switchValue;
  int lastSwitchValue = digitalRead(switchItPin);
  int touchThreshold = 100;  // Threshold for FSR

  lcd.clear();
  lcd.print("Select Mode:");
  delay(1000);

  while (!modeSelected) {
    lcd.clear();
    lcd.print("Touch : Easy");
    lcd.setCursor(0, 1);
    lcd.print("Switch : Hard");

    touchValue = analogRead(tapItPin);
    switchValue = digitalRead(switchItPin);

    if (touchValue > touchThreshold) {
      isHardMode = false;
      modeSelected = true;
      lcd.clear(); lcd.print("Easy Mode!");
      Serial.println("Easy Mode Selected");
      delay(1000);
    }
    if (switchValue != lastSwitchValue) {
      isHardMode = true;
      modeSelected = true;
      lcd.clear(); lcd.print("Hard Mode!");
      Serial.println("Hard Mode Selected");
      delay(1000);
    }

    lastSwitchValue = switchValue;
    delay(100);
  }

  gameRate = isHardMode ? 4000 : 6000;
}

void loop() {
  // Display start prompt
  lcd.clear();
  lcd.print("Flip Switch to");
  lcd.setCursor(0,1);
  lcd.print("Start Game");

  // Read switch for start detection
  int startSwitchState = digitalRead(switchItPin);
  Serial.print("Start Switch: "); Serial.println(startSwitchState);

  // Trigger start on any switch state change
  if (startSwitchState != lastStartSwitchState) {
    keepPlaying = 1;

    while (keepPlaying) {
      // Pick a random action based on mode
      if (isHardMode) {
        action = random(1, 5);  // 4 actions: 1=Switch, 2=Bend, 3=Tap, 4=Turn
      } else {
        action = random(1, 4);  // 3 actions: 1=Bend, 2=Tap, 3=Turn
      }
      actionCompleted = 0;
      tapCount = 0;
      Serial.print("Action: "); Serial.println(action);

      // SWITCH It action (Hard mode only)
      if (action == 1 && isHardMode) {
        performSwitchIt();
      }
      // BEND It
      else if (action == (isHardMode ? 2 : 1)) {
        performBendIt();
      }
      // TAP It
      else if (action == (isHardMode ? 3 : 2)) {
        performTapIt();
      }
      // TURN It
      else {
        performTurnIt();
      }
    }

    // Reset game
    gameRate = isHardMode ? 4000 : 6000;
    score = 0;
    Serial.println("Game ended. Selecting difficulty again.");
    selectDifficulty();
  }
  // Update last switch state
  lastStartSwitchState = startSwitchState;
}

// Action functions
void performSwitchIt() {
  Serial.println("Switch It");
  switchItState1 = digitalRead(switchItPin);
  lcd.clear(); lcd.print("Switch It!");
  timeOfPrompt = millis();

  while ((millis() - timeOfPrompt) < gameRate && !actionCompleted) {
    switchItState2 = digitalRead(switchItPin);
    if (switchItState2 != switchItState1) {
      actionCompleted = 1; score++; adjustRate();
    }
  }
  handleResult();
}

void performBendIt() {
  Serial.println("Bend It");
  bendItState1 = analogRead(bendItPin);
  lcd.clear(); lcd.print("Bend It!");
  timeOfPrompt = millis();

  while ((millis() - timeOfPrompt) < gameRate && !actionCompleted) {
    bendItState2 = analogRead(bendItPin);
    if (abs(bendItState2 - bendItState1) >= 5) {
      actionCompleted = 1; score++; adjustRate();
    }
  }
  handleResult();
}

void performTapIt() {
  Serial.println("Tap It");
  tapItState1 = analogRead(tapItPin);
  lcd.clear(); lcd.print("Tap It!");
  timeOfPrompt = millis();

  while ((millis() - timeOfPrompt) < gameRate && !actionCompleted) {
    tapItState2 = analogRead(tapItPin);
    if (abs(tapItState2 - tapItState1) >= 100) {
      tapCount++; tapItState1 = tapItState2;
      if (tapCount >= 2) { actionCompleted = 1; score++; adjustRate(); }
    }
  }
  handleResult();
}

void performTurnIt() {
  Serial.println("Twist It");
  lcd.clear(); lcd.print("Twist It!");
  timeOfPrompt = millis();

  encoderState1 = digitalRead(encoderPinA);
  encoderState2 = digitalRead(encoderPinB);
  lastEncoded = (encoderState1 << 1) | encoderState2;
  lastEncoderValue = 0; encoderValue = 0;

  while ((millis() - timeOfPrompt) < gameRate && !actionCompleted) {
    encoderState1 = digitalRead(encoderPinA);
    encoderState2 = digitalRead(encoderPinB);
    int encoded = (encoderState1 << 1) | encoderState2;
    int sum = (lastEncoded << 2) | encoded;
    if (sum==0b1101||sum==0b0100||sum==0b0010||sum==0b1011) encoderValue++;
    if (sum==0b1110||sum==0b0111||sum==0b0001||sum==0b1000) encoderValue--;
    lastEncoded = encoded;
    if (encoderValue != lastEncoderValue) { actionCompleted = 1; score++; adjustRate(); }
  }
  handleResult();
}

// Adjust game rate for hard mode
void adjustRate() {
  if (isHardMode) gameRate -= (gameRate * 0.05);
}

// Handle action result (success/fail)
void handleResult() {
  if (actionCompleted) {
    playSuccessTone();
    actionCompleted = 0;
  } else {
    playFailTone();
    finalScore = score;
    keepPlaying = 0;
    lcd.clear(); lcd.print("GAME OVER!");
    lcd.setCursor(0,1);
    lcd.print("Score: "); lcd.print(finalScore);
    delay(2000);
  }
}

// Sound feedback
void playSuccessTone() {
  tone(3, 880, 100);
  delay(100);
  noTone(3);
}

void playFailTone() {
  tone(3, 800, 300); delay(300);
  tone(3, 600, 300); delay(300);
  tone(3, 400, 400); delay(400);
  tone(3, 200, 600); delay(600);
  noTone(3);
}
