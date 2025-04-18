#include <LiquidCrystal.h>

// LCD pin connections using only available pins
const int rs = 5, en = 6, d4 = 7, d5 = 8, d6 = 9, d7 = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  // Initialize LCD
  lcd.begin(16, 2);
  lcd.print("LCD Test");
}

void loop() {
  lcd.setCursor(0, 1);
  lcd.print("Hello World!");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("                ");  // Clear line
  delay(1000);
}
