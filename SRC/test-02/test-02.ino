#define LED_RATE 50  // in microseconds; should give 0.5Hz toggles

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306_STM32.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

int data = 0;
int analogPin = 3;
int prescaler = 4;
int f_pwm = 20000; // frequency (Hz)
int DC = 40; // % duty cycle
double pwmval;
int dcval = 0;

bool TickFlag = false;

void setup() {
  // Set up the LED to blink
  Serial.begin(115200);
  pinMode(analogPin, INPUT_ANALOG);
  pinMode(PB0, OUTPUT);
  pinMode(PB1, OUTPUT);
  pinMode(PC13, OUTPUT);
  pinMode(PB6, PWM);
  pinMode(PB7, PWM);
  Timer2.pause();
  Timer2.setPeriod(LED_RATE); // in microseconds
  Timer2.setMode(TIMER_CH1, TIMER_OUTPUTCOMPARE);
  Timer2.setCompare(TIMER_CH1, 1);
  Timer2.attachInterrupt(TIMER_CH1, handler_led);
  Timer2.refresh();
  Timer2.resume();
  Timer4.setPrescaleFactor(prescaler);
  pwmval = (72000000 / prescaler) / f_pwm;
  Timer4.setOverflow(pwmval);

}

void loop() {
  dcval = analogRead(analogPin);
  dcval = map(dcval, 1024, 0, pwmval, 0);
  Serial.println(dcval);
}

void handler_led(void) {
  if (TickFlag) {
    digitalWrite(PB0, HIGH);
    digitalWrite(PB1, HIGH);
    digitalWrite(PC13, HIGH);
  } else  {
    digitalWrite(PB0, LOW);
    digitalWrite(PB1, LOW);
    digitalWrite(PC13, LOW);
  }
  pwmWrite(PB6, dcval);
  pwmWrite(PB7, pwmval * DC / 100);

  TickFlag = !TickFlag;
}
