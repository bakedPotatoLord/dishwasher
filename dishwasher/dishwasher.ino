#include <LiquidCrystal_I2C.h>
#include <Servo.h>  // Servo library used to control ESC

#define u32 uint32_t

// Rotary encoder pins
#define datPin 2
#define clkPin 3
#define swPin 12

//relay pin
#define relayPin 17 //A3



// ESC control pin and PWM values
#define servoPin 11
#define pwmStop 1500  // PWM value that stops the motor
#define pwmRun 1650   // PWM value that runs the motor

// Encoder position (represents wash time in seconds)
volatile int position = 0;

// Remaining wash time and start timestamp
u32 remaining;
u32 washStart;

// Track button state from previous loop iteration
u8 pressedLast = 0;


// Flag indicating whether the dishwasher is currently running
u8 washing = 0;

// ESC object
Servo esc;

// LCD object configured with pin definitions above
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {

  

  Serial.begin(9600);  // Initialize serial for debugging

  Serial.println("howdy!");

  esc.attach(servoPin);  // Attach ESC to servo control pin

  // initialize the LCD with 16 columns and 2 rows
  lcd.init();
  lcd.backlight();

  // Configure encoder pins with pullups
  pinMode(swPin, INPUT_PULLUP);
  pinMode(clkPin, INPUT_PULLUP);
  pinMode(datPin, INPUT_PULLUP);

  pinMode(relayPin,OUTPUT);

  // Attach interrupts for rotary encoder channels
  attachInterrupt(digitalPinToInterrupt(clkPin), isrClk, CHANGE);
}

void loop() {

  // Serial.println(position);
  //   // Temporarily disable interrupts while reading shared state
  noInterrupts();
  int savedPos = position;
  // Re-enable interrupts
  interrupts();

  if (washing) {
    // Display wash countdown
    lcd.setCursor(0, 0);
    lcd.print("Time Remaining");
    lcd.setCursor(0, 1);

    // Calculate actual remaining time based on elapsed millis
    u32 now = millis();
    u32 elapsed = ((now - washStart) / 1000);
    int realRem = remaining - elapsed;

    // Stop motor and exit wash mode when timer expires
    if (realRem <= 0) {

      // Serial.println("killing");
      // Serial.println(now);
      // Serial.println(washStart);
      // Serial.println(remaining);
      // Serial.println(realRem);

      esc.writeMicroseconds(pwmStop);
      digitalWrite(relayPin,0);
      washing = 0;
      return;
    }

    // Convert seconds to minutes and seconds
    int hours = realRem / 60;
    int seconds = realRem % 60;

    // Clear previous line contents
    lcd.print("       ");

    // Print time remaining
    lcd.setCursor(0, 1);
    lcd.print(hours);
    lcd.print(':');
    lcd.print(seconds);

  } else {
    // Display time-setting mode
    lcd.setCursor(0, 0);
    lcd.print("Set wash time ");
    // lcd.setCursor(0, 2);

    // Convert encoder position into minutes and seconds
    int hours = savedPos / 60;
    int seconds = savedPos % 60;

    // Clear previous line contents
    // lcd.print("     ");

    // Display currently selected time
    lcd.setCursor(0, 1);
    lcd.print(hours);
    lcd.print(':');
    if (seconds < 9) lcd.print("0");
    lcd.print(seconds);
  }



  // Read encoder pushbutton (active low)
  u8 pressed = !digitalRead(swPin);
  // Serial.print("pressed: ");
  // Serial.println(pressed);

  // Detect rising edge of button press
  if (!pressedLast && pressed) {

    // Wait for button release (simple debounce)
    while (!digitalRead(swPin)) {};

    // Toggle washing state
    if (washing) {
      // Stop wash cycle
      esc.writeMicroseconds(pwmStop);
      digitalWrite(relayPin,0);
      washing = 0;
    } else {
      // Start wash cycle
      esc.writeMicroseconds(pwmRun);
      digitalWrite(relayPin,1);

      // Serial.println("starting wash");
      // Capture current timer settings
      remaining = position;
      washStart = millis();
      washing = 1;
      // Serial.print("wash start: ");
      // Serial.println(washStart);
    }
  }
  //update pressedLast state
  pressedLast = pressed;
  // Slow loop slightly to reduce LCD flicker and CPU usage
  delay(10);
}

// Interrupt handler for CLK signal of rotary encoder
void isrClk() {
  // Ignore encoder changes while washing
  if (washing) return;

  // Determine rotation direction by comparing CLK and DAT
  if (digitalRead(clkPin) != digitalRead(datPin)) {
    position++;  // clockwise
  } else {
    // counterclockwise with lower bound at 0
    position = (position > 0) ? position - 1 : 0;
  }
}
