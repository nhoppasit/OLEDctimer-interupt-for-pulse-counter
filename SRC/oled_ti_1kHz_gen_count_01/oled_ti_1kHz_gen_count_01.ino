#define _TRACE_ 0

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306_STM32.h>

/* Timer interrupt for pulse generation and counting */
#define LED_RATE 500 // in microseconds; should give 1.0kHz toggles
bool ticktok = false;
bool OutputFlag = false;
int OnCount = 0;

/* Adc moving average */
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH 16
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

/* Adc moving average */
unsigned long t0Adc = 0;
bool Adc_FLAG = true;    //
const int ADC_TIME = 10; /*ms*/
const int MAX_ADC_READINGS = 100;
float VREF = 0;
int AdcReadIndex = 0; // the index of the current reading
#define Adc0Pin PA6
int Adc0Value = 0;
float Voltage0 = 0;
float V0Readings[MAX_ADC_READINGS]; // the Adc0Readings from the analog input
float V0Total = 0;                  // the running Adc0Total
float V18650 = 0;
float BatteryPercent = 0;

const int OLED_TIME = 50; /*milliseconds*/
unsigned long t0Oled = 0;

void setup()
{
#if _TRACE_
  Serial.begin(9600);
#endif
  pinMode(Adc0Pin, INPUT_ANALOG);

  pinMode(PB12, INPUT);
  pinMode(PB13, INPUT);

  pinMode(PC13, OUTPUT);
  pinMode(PB1, OUTPUT);
  pinMode(PB8, OUTPUT);
  pinMode(PB9, OUTPUT);

  for (int i = 0; i < 3; i++)
  {
    digitalWrite(PC13, 0);
    digitalWrite(PB1, 1);
    digitalWrite(PB8, 1);
    digitalWrite(PB9, 1);
    delay(150);
    digitalWrite(PC13, 1);
    digitalWrite(PB1, 0);
    digitalWrite(PB8, 0);
    digitalWrite(PB9, 0);
    delay(250);
  }
  digitalWrite(PC13, 1);
  digitalWrite(PB1, 0);
  digitalWrite(PB8, 0);
  digitalWrite(PB9, 0);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // initialize with the I2C addr 0x3D (for the 128x64)
  // init done

  // Clear the buffer.
  display.clearDisplay();

  // draw multiple rectangles
  testfillrect();
  display.display();
  delay(1000);
  display.clearDisplay();
  display.display();

  // invert the display
  for (int i = 0; i < 3; i++)
  {
    display.invertDisplay(true);
    delay(150);
    display.invertDisplay(false);
    delay(250);
  }
  display.clearDisplay();
  display.display();

  Timer2.pause();
  Timer2.setPeriod(LED_RATE); // in microseconds
  Timer2.setMode(TIMER_CH1, TIMER_OUTPUTCOMPARE);
  Timer2.setCompare(TIMER_CH1, 1);
  Timer2.attachInterrupt(TIMER_CH1, handler_led);
  Timer2.refresh();
  Timer2.resume();
}

void loop()
{
  ReadAdc(true);
  OutputFlag = digitalRead(PB12) == LOW;
  if (digitalRead(PB13) == LOW)
    OnCount = 0;
  oledShow();
}

void oledShow()
{
  if (OLED_TIME < (millis() - t0Oled))
  {
    // text display tests
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print("BAT: ");
    display.print(BatteryPercent, 0);
    display.println("%");
    display.println("PULSE:");
    display.setCursor(0, 40);
    display.setTextSize(3);
    display.print((float)OnCount, 0);
    if (0 <= AdcReadIndex && AdcReadIndex < MAX_ADC_READINGS / 2)
      display.fillCircle(display.width()-6, 5, 6, WHITE);
    display.display();
    t0Oled = millis();
  }
}

/*
  -----------------------------------------------------------------------------
  Analog: A0, A1
  -----------------------------------------------------------------------------
*/
void ReadAdc(bool flag)
{
  if (flag)
  {
    if (ADC_TIME < (millis() - t0Adc))
    {
      // VREF = 1.2 * 4095.0 / (float)adc_read(ADC1, 17);
      VREF = 3.3;

      Adc0Value = analogRead(Adc0Pin);
      float V0 = VREF * (float)Adc0Value / 4095.0 + 0.039;
      V0Total = V0Total - V0Readings[AdcReadIndex];
      V0Readings[AdcReadIndex] = V0;
      V0Total = V0Total + V0Readings[AdcReadIndex];
      Voltage0 = V0Total / (float)MAX_ADC_READINGS;
      V18650 = Voltage0 * 2.0;
      BatteryPercent = (float)((int)((V18650 - 3.18) * 100.0));
      if (100.0 < BatteryPercent)
        BatteryPercent = 100.0;
      if (BatteryPercent < 0.0)
        BatteryPercent = 0.0;
      AdcReadIndex = AdcReadIndex + 1;
      if (MAX_ADC_READINGS <= AdcReadIndex)
      {
        AdcReadIndex = 0;
#if _TRACE_
        Serial.print("Pulse Count = ");
        Serial.print(OnCount);
        Serial.print(" | A0 = ");
        Serial.print(Adc0Value);
        Serial.print(", Vref = ");
        Serial.print(VREF, 3);
        Serial.print(", V0 (Volt) = ");
        Serial.print(Voltage0, 3);
        Serial.print(", 18650 LiON Battery (Volt) = ");
        Serial.print(V18650, 3);
        Serial.print(", Percent = ");
        Serial.print(BatteryPercent);
        Serial.print("%");
        Serial.println();
#endif
      }
      t0Adc = millis();
    }
  }
}

void handler_led(void)
{
  if (OutputFlag)
  {
    digitalWrite(PC13, 0);
    if (ticktok)
    {
      digitalWrite(PB1, 1);
      digitalWrite(PB8, 1);
      digitalWrite(PB9, 1);
      OnCount++;
    }
    else
    {
      digitalWrite(PB1, 0);
      digitalWrite(PB8, 0);
      digitalWrite(PB9, 0);
    }
    ticktok = !ticktok;
  }
  else
  {
    digitalWrite(PC13, 1);
    digitalWrite(PB1, 0);
    digitalWrite(PB8, 0);
    digitalWrite(PB9, 0);
  }
}

///
void testfillrect(void)
{
  uint8_t color = 1;
  for (int16_t i = 0; i < display.height() / 2; i += 3)
  {
    // alternate colors
    display.fillRect(i, i, display.width() - i * 2, display.height() - i * 2, color % 2);
    display.display();
    color++;
  }
}
