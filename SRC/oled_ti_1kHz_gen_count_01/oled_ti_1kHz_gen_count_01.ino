#define TRACE_ADC 1

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306_STM32.h>

#define LED_RATE 500 // in microseconds; should give 1.0kHz toggles
bool ticktok = false;
bool OutputFlag = false;

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

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

void setup()
{
  Serial.begin(9600);

  pinMode(Adc0Pin, INPUT_ANALOG);
  pinMode(PB12, INPUT);
  pinMode(PB13, INPUT);
  pinMode(PC13, OUTPUT);
  pinMode(PB1, OUTPUT);
  pinMode(PB8, OUTPUT);

  for (int i = 0; i < 3; i++)
  {
    digitalWrite(PC13, 1);
    digitalWrite(PB1, 1);
    digitalWrite(PB8, 1);
    delay(150);
    digitalWrite(PC13, 0);
    digitalWrite(PB1, 0);
    digitalWrite(PB8, 0);
    delay(250);
  }
  digitalWrite(PC13, 0);
  digitalWrite(PB1, 0);
  digitalWrite(PB8, 0);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // initialize with the I2C addr 0x3D (for the 128x64)
  // init done

  // Clear the buffer.
  display.clearDisplay();

  // draw multiple rectangles
  testfillrect();
  display.display();
  delay(2000);
  display.clearDisplay();
  display.display();

  // invert the display
  display.invertDisplay(true);
  delay(500);
  display.invertDisplay(false);
  delay(500);
  display.invertDisplay(true);
  delay(500);
  display.invertDisplay(false);
  delay(500);
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
      float V18650 = Voltage0 * 2.0;
      AdcReadIndex = AdcReadIndex + 1;
      if (MAX_ADC_READINGS <= AdcReadIndex)
      {
        AdcReadIndex = 0;
#if TRACE_ADC
        Serial.print("A0 = ");
        Serial.print(Adc0Value);
        Serial.print(", Vref = ");
        Serial.print(VREF, 3);
        Serial.print(", V0 (Volt) = ");
        Serial.print(Voltage0, 3);
        Serial.print(", 18650 LiON Battery (Volt) = ");
        Serial.print(V18650, 3);
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
    if (ticktok)
    {
      digitalWrite(PC13, 1);
      digitalWrite(PB1, 1);
      digitalWrite(PB8, 1);
    }
    else
    {
      digitalWrite(PC13, 0);
      digitalWrite(PB1, 0);
      digitalWrite(PB8, 0);
    }
    ticktok = !ticktok;
  }
}

void testdrawchar(void)
{
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  for (uint8_t i = 0; i < 168; i++)
  {
    if (i == '\n')
      continue;
    display.write(i);
    if ((i > 0) && (i % 21 == 0))
      display.println();
  }
  display.display();
}

void testdrawcircle(void)
{
  for (int16_t i = 0; i < display.height(); i += 2)
  {
    display.drawCircle(display.width() / 2, display.height() / 2, i, WHITE);
    display.display();
  }
}

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

void testdrawtriangle(void)
{
  for (int16_t i = 0; i < min(display.width(), display.height()) / 2; i += 5)
  {
    display.drawTriangle(display.width() / 2, display.height() / 2 - i,
                         display.width() / 2 - i, display.height() / 2 + i,
                         display.width() / 2 + i, display.height() / 2 + i, WHITE);
    display.display();
  }
}

void testfilltriangle(void)
{
  uint8_t color = WHITE;
  for (int16_t i = min(display.width(), display.height()) / 2; i > 0; i -= 5)
  {
    display.fillTriangle(display.width() / 2, display.height() / 2 - i,
                         display.width() / 2 - i, display.height() / 2 + i,
                         display.width() / 2 + i, display.height() / 2 + i, WHITE);
    if (color == WHITE)
      color = BLACK;
    else
      color = WHITE;
    display.display();
  }
}

void testdrawroundrect(void)
{
  for (int16_t i = 0; i < display.height() / 2 - 2; i += 2)
  {
    display.drawRoundRect(i, i, display.width() - 2 * i, display.height() - 2 * i, display.height() / 4, WHITE);
    display.display();
  }
}

void testfillroundrect(void)
{
  uint8_t color = WHITE;
  for (int16_t i = 0; i < display.height() / 2 - 2; i += 2)
  {
    display.fillRoundRect(i, i, display.width() - 2 * i, display.height() - 2 * i, display.height() / 4, color);
    if (color == WHITE)
      color = BLACK;
    else
      color = WHITE;
    display.display();
  }
}

void testdrawrect(void)
{
  for (int16_t i = 0; i < display.height() / 2; i += 2)
  {
    display.drawRect(i, i, display.width() - 2 * i, display.height() - 2 * i, WHITE);
    display.display();
  }
}

void testdrawline()
{
  for (int16_t i = 0; i < display.width(); i += 4)
  {
    display.drawLine(0, 0, i, display.height() - 1, WHITE);
    display.display();
  }
  for (int16_t i = 0; i < display.height(); i += 4)
  {
    display.drawLine(0, 0, display.width() - 1, i, WHITE);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i = 0; i < display.width(); i += 4)
  {
    display.drawLine(0, display.height() - 1, i, 0, WHITE);
    display.display();
  }
  for (int16_t i = display.height() - 1; i >= 0; i -= 4)
  {
    display.drawLine(0, display.height() - 1, display.width() - 1, i, WHITE);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i = display.width() - 1; i >= 0; i -= 4)
  {
    display.drawLine(display.width() - 1, display.height() - 1, i, 0, WHITE);
    display.display();
  }
  for (int16_t i = display.height() - 1; i >= 0; i -= 4)
  {
    display.drawLine(display.width() - 1, display.height() - 1, 0, i, WHITE);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i = 0; i < display.height(); i += 4)
  {
    display.drawLine(display.width() - 1, 0, 0, i, WHITE);
    display.display();
  }
  for (int16_t i = 0; i < display.width(); i += 4)
  {
    display.drawLine(display.width() - 1, 0, i, display.height() - 1, WHITE);
    display.display();
  }
  delay(250);
}

void testscrolltext(void)
{
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  display.clearDisplay();
  display.println("scroll");
  display.display();

  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
}
