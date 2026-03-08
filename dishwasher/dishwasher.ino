
#include <LiquidCrystal.h>
#include <Servo.h>

#define rs 9
#define en 10
#define d4 7
#define d5 6
#define d6 5
#define d7 4

#define datPin 2
#define clkPin 3
#define swPin 12
#define CLICKS_PER_STEP 4  // this number depends on your rotary encoder

#define servoPin 11


#define pwmStop 1500
#define pwmRun 1650


volatile int position = 0;

int remaining;
int washStart;

u8 pressedLast = 0;
u8 washing = 0;

Servo esc;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {

  Serial.begin(9600);

  esc.attach(servoPin);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.


  pinMode(swPin, INPUT_PULLUP);

  pinMode(clkPin, INPUT_PULLUP);
  pinMode(datPin, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(clkPin), isrClk, CHANGE);
  attachInterrupt(digitalPinToInterrupt(datPin), isrDat, CHANGE);
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  noInterrupts();
  if (washing) {
    lcd.setCursor(0, 0);
    lcd.print("Time Remaining");
    lcd.setCursor(0, 1);

    int realRem = remaining - ((millis() - washStart) / 1000);

    if (realRem <= 0) {
      esc.writeMicroseconds(pwmStop);
      washing = 0;
      return;
    }

    int hours = realRem / 60;
    int seconds = realRem % 60;
    lcd.print("     ");

    lcd.setCursor(0, 1);
    lcd.print(hours);
    lcd.print(':');
    lcd.print(seconds);

  } else {
    lcd.setCursor(0, 0);
    lcd.print("Set wash time ");
    lcd.setCursor(0, 1);

    int hours = position / 60;
    int seconds = position % 60;
    lcd.print("     ");

    lcd.setCursor(0, 1);
    lcd.print(hours);
    lcd.print(':');
    lcd.print(seconds);
  }

  interrupts();


  u8 pressed = !digitalRead(swPin);




  if (!pressedLast && pressed) {
    while(!digitalRead(swPin)){};
    Serial.println("washing");

    if (washing) {
      esc.writeMicroseconds(pwmStop);
      washing = 0;
    } else {
      esc.writeMicroseconds(pwmRun);
      remaining = position;
      washStart = millis();
      washing = 1;
    }
  }


  delay(100);
}

void isrClk() {
  if (washing) return;
  if (digitalRead(clkPin) == digitalRead(datPin)) {
    position++;
  } else {
    position = (position >= 0) ? position - 1 : 0;
  }
}
void isrDat() {
  if (washing) return;
  if (digitalRead(clkPin) != digitalRead(datPin)) {
    position++;
  } else {
    position = (position >= 0) ? position - 1 : 0;
  }
}
