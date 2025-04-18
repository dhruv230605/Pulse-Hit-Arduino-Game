//include the library for lcd screen use
#include <LiquidCrystal.h>

// LCD pin connections
const int rs = 5, en = 6, d4 = 7, d5 = 8, d6 = 9, d7 = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//sensor pins
const int switchItPin = 4;     // Switch pin
const int pushItPin = 2;       // Button pin (also used as start button)
const int bendItPin = A1;      // Flex sensor pin
const int tapItPin = A3;       // FSR pin
const int encoderPinA = 12;    // Rotary encoder pin A
const int encoderPinB = 11;    // Rotary encoder pin B

// Game state variables
int startButtonState;
int lastStartButtonState = HIGH;  // Previous state of start button
int lastPushButtonState = HIGH;   // Previous state of push button
long action;
long unmappedAction;
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

void setup() {
  // Initialize LCD
  lcd.begin(16, 2);
  
  // Initialize serial monitor
  Serial.begin(9600);
  delay(500);

  randomSeed(analogRead(A2));

  // Setting pin modes
  pinMode(switchItPin, INPUT_PULLUP);
  pinMode(pushItPin, INPUT_PULLUP);
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);
  // bendItPin and tapItPin automatically set as inputs due to their analog nature

  // Set initial button states
  startButtonState = digitalRead(pushItPin);
  lastStartButtonState = startButtonState;
  lastPushButtonState = digitalRead(pushItPin);

  // Initialize encoder
  encoderState1 = digitalRead(encoderPinA);
  encoderState2 = digitalRead(encoderPinB);
  lastEncoded = (encoderState1 << 1) | encoderState2;

  // Display mode selection
  selectDifficulty();
}

void selectDifficulty() {
  bool modeSelected = false;
  int touchValue;
  int switchValue;
  int lastSwitchValue = digitalRead(switchItPin);  // Track last switch state
  int touchThreshold = 100;  // Threshold for FSR

  // Show "Select Mode" message once
  lcd.clear();
  lcd.print("Select Mode:");
  delay(1000);

  // Then loop until mode is selected
  while (!modeSelected) {
    lcd.clear();
    lcd.print("Touch:Easy");
    lcd.setCursor(0, 1);
    lcd.print("Switch:Hard");

    // Read sensor values
    touchValue = analogRead(tapItPin);
    switchValue = digitalRead(switchItPin);

    // Debug prints
    Serial.print("Touch Value: ");
    Serial.println(touchValue);
    Serial.print("Switch Value: ");
    Serial.println(switchValue);

    // Check for FSR (Easy mode)
    if (touchValue > touchThreshold) {
      isHardMode = false;
      modeSelected = true;
      lcd.clear();
      lcd.print("Easy Mode!");
      Serial.println("Easy Mode Selected");
      delay(1000);
    }

    // Check for switch change (Hard mode)
    if (switchValue != lastSwitchValue) {
      isHardMode = true;
      modeSelected = true;
      lcd.clear();
      lcd.print("Hard Mode!");
      Serial.println("Hard Mode Selected");
      delay(1000);
    }

    lastSwitchValue = switchValue;
    delay(100);
  }

  // Set game rate based on mode
  gameRate = isHardMode ? 4000 : 6000;
}


void loop() {
  // Read start button to see if pressed
  startButtonState = digitalRead(pushItPin);
  
  // Debug prints
  Serial.print("Start Button: ");
  Serial.println(startButtonState);
  Serial.print("Bend value: ");
  Serial.println(analogRead(bendItPin));
  Serial.print("Tap value: ");
  Serial.println(analogRead(tapItPin));

  // Display game start message
  lcd.clear();
  lcd.print("Double Tap Start");

  // Check for falling edge on start button (HIGH -> LOW)
  if (startButtonState == LOW && lastStartButtonState == HIGH) {
    keepPlaying = 1;
    
    while (keepPlaying != 0) {
      // Pick a random action based on mode
      if (isHardMode) {
        action = random(1, 6);  // All 5 actions
      } else {
        action = random(1, 4);  // Only 3 actions (1: Turn, 2: Bend, 3: Tap)
      }
      
      actionCompleted = 0;
      tapCount = 0;  // Reset tap counter
      Serial.print("Action: ");
      Serial.println(action);

      // Switch It action (Hard mode only)
      if (action == 1 && isHardMode) {
        Serial.println("Switch It");
        switchItState1 = digitalRead(switchItPin);
        switchItState2 = switchItState1;

        lcd.clear();
        lcd.print("Switch It!");

        timeOfPrompt = millis();
        timeElapsed = millis() - timeOfPrompt;

        while (timeElapsed < gameRate && actionCompleted == 0) {
          switchItState2 = digitalRead(switchItPin);
          
          if (switchItState2 != switchItState1) {
            actionCompleted = 1;
            score++;
            if (isHardMode) {
              gameRate = gameRate - (gameRate * 0.05);
            }
          }
          
          timeElapsed = millis() - timeOfPrompt;
        }

        handleActionCompletion(action);
      }

      // Bend It action
      else if (action == (isHardMode ? 2 : 1)) {
        Serial.println("Bend It");
        bendItState1 = analogRead(bendItPin);
        
        lcd.clear();
        lcd.print("Bend It!");

        timeOfPrompt = millis();
        timeElapsed = millis() - timeOfPrompt;

        while (timeElapsed < gameRate && actionCompleted == 0) {
          bendItState2 = analogRead(bendItPin);
          
          // Check if flex sensor value changed by 5 or more
          if (abs(bendItState2 - bendItState1) >= 5) {
            actionCompleted = 1;
            score++;
            if (isHardMode) {
              gameRate = gameRate - (gameRate * 0.05);
            }
          }
          
          timeElapsed = millis() - timeOfPrompt;
        }

        handleActionCompletion(action);
      }

      // Push It action (Hard mode only)
      else if (action == 3 && isHardMode) {
        Serial.println("Double Push!");
        
        lcd.clear();
        lcd.print("Double Push!");

        timeOfPrompt = millis();
        timeElapsed = millis() - timeOfPrompt;

        while (timeElapsed < gameRate && actionCompleted == 0) {
          int currentPushButtonState = digitalRead(pushItPin);
          
          // Check for falling edge (HIGH -> LOW)
          if (currentPushButtonState == LOW && lastPushButtonState == HIGH) {
            actionCompleted = 1;
            score++;
            if (isHardMode) {
              gameRate = gameRate - (gameRate * 0.05);
            }
          }
          
          lastPushButtonState = currentPushButtonState;
          timeElapsed = millis() - timeOfPrompt;
        }

        handleActionCompletion(action);
      }

      // Tap It action
      else if (action == (isHardMode ? 4 : 2)) {
        Serial.println("Tap It!");
        tapItState1 = analogRead(tapItPin);
        
        lcd.clear();
        lcd.print("Tap It!");

        timeOfPrompt = millis();
        timeElapsed = millis() - timeOfPrompt;

        while (timeElapsed < gameRate && actionCompleted == 0) {
          tapItState2 = analogRead(tapItPin);
          
          // Check if FSR value changed significantly
          if (abs(tapItState2 - tapItState1) >= 100) {
            tapCount++;
            tapItState1 = tapItState2;  // Reset baseline after first tap
            if (tapCount >= 2) {  // Require two taps
              actionCompleted = 1;
              score++;
              if (isHardMode) {
                gameRate = gameRate - (gameRate * 0.05);
              }
            }
          }
          
          timeElapsed = millis() - timeOfPrompt;
        }

        handleActionCompletion(action);
      }

      // Turn It action
      else if (action == (isHardMode ? 5 : 3)) {
        Serial.println("Turn It");
        
        lcd.clear();
        lcd.print("Turn It!");

        timeOfPrompt = millis();
        timeElapsed = millis() - timeOfPrompt;
        encoderState1 = digitalRead(encoderPinA);
        encoderState2 = digitalRead(encoderPinB);
        lastEncoded = (encoderState1 << 1) | encoderState2;
        lastEncoderValue = 0;
        encoderValue = 0;  // Reset encoder value

        while (timeElapsed < gameRate && actionCompleted == 0) {
          encoderState1 = digitalRead(encoderPinA);
          encoderState2 = digitalRead(encoderPinB);
          
          int encoded = (encoderState1 << 1) | encoderState2;
          int sum = (lastEncoded << 2) | encoded;

          if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue++;
          if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue--;

          lastEncoded = encoded;

          // Detect change
          if (encoderValue != lastEncoderValue) {
            actionCompleted = 1;
            score++;
            if (isHardMode) {
              gameRate = gameRate - (gameRate * 0.05);
            }
            lastEncoderValue = encoderValue;
          }
          
          timeElapsed = millis() - timeOfPrompt;
        }

        handleActionCompletion(action);
      }
    }
    
    // Reset game parameters
    gameRate = isHardMode ? 4000 : 6000;  // Reset based on mode
    score = 0;
    
    // Debug print to confirm difficulty selection is called
    Serial.println("Game ended. Selecting difficulty again.");
    
    // Ask for difficulty again
    selectDifficulty();
  }
  
  // Update last button state
  lastStartButtonState = startButtonState;
}

void handleActionCompletion(long actionType) {
  if (actionCompleted == 1) {
    Serial.println("Action completed");

    // Play a unique sound based on the action
    switch (actionType) {
      case 1: // Switch It
        tone(3, 523, 150); // C5
        delay(150);
        tone(3, 659, 150); // E5
        delay(150);
        tone(3, 784, 150); // G5
        delay(150);
        break;
        
      case 2: // Bend It
        tone(3, 440, 100); // A4
        delay(100);
        tone(3, 494, 100); // B4
        delay(100);
        tone(3, 523, 200); // C5
        delay(200);
        break;

      case 3: // Push It
        tone(3, 330, 100); // E4
        delay(100);
        tone(3, 349, 100); // F4
        delay(100);
        tone(3, 392, 200); // G4
        delay(200);
        break;

      case 4: // Tap It
        tone(3, 880, 80);  // A5
        delay(80);
        tone(3, 988, 80);  // B5
        delay(80);
        tone(3, 1047, 80); // C6
        delay(80);
        tone(3, 1175, 120); // D6
        delay(120);
        break;

      case 5: // Turn It
        for (int i = 0; i < 3; i++) {
          tone(3, 700 + i * 100, 100); // rising tones
          delay(100);
        }
        break;

      default:
        tone(3, 880, 150);
        delay(150);
        tone(3, 988, 150);
        delay(150);
        tone(3, 1047, 150);
        delay(150);
        break;
    }

    noTone(3);

    if (isHardMode) {
      gameRate = gameRate - (gameRate * 0.05);
    }
  } else {
    // Same dying sound as before
    Serial.println("Action failed");

    tone(3, 800, 300);
    delay(300);
    tone(3, 600, 300);
    delay(300);
    tone(3, 400, 400);
    delay(400);
    tone(3, 200, 600);
    delay(600);
    noTone(3);

    finalScore = score;
    keepPlaying = 0;

    lcd.clear();
    lcd.print("GAME OVER!");
    lcd.setCursor(0, 1);
    lcd.print("Score: ");
    lcd.print(finalScore);

    delay(2000);
  }
}

