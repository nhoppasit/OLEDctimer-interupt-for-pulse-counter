
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306_STM32.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

bool BlinkState = true;

void setup()
{
  pinMode(PB12, OUTPUT_OPEN_DRAIN);
  pinMode(PB13, OUTPUT_OPEN_DRAIN);
  pinMode(PB1, PWM_OPEN_DRAIN);
  pinMode(PB15, OUTPUT);

  pinMode(PA8, INPUT_PULLUP);
  pinMode(PB4, INPUT_PULLUP);
  pinMode(PB5, INPUT_PULLUP);

  digitalWrite(PB12, LOW);
  digitalWrite(PB13, LOW);
  pwmWrite(PB1, 0);
  digitalWrite(PB15, LOW);

  Serial.begin(9600);
  delay(250);
  Serial1.begin(9600);
  delay(250);
  Serial.println("Starting...");

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // initialize with the I2C addr 0x3D (for the 128x64)
  // display.begin(SSD1306_SWITCHCAPVCC, 0x3D);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  display.clearDisplay();
  display.display();
  Serial.println("Init done.");

  // Title
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("DARKSUN AIR BOX 2.0");
  display.println("---------------------");
  display.setTextSize(1);
  display.println();
  display.println("1. TANK ADJUST");
  display.println();
  display.println("2. CALIBRATE");
  display.println();
  display.println("3. PRESET MAIN");
  display.display();
  delay(2000);

  // invert the display
  for (int iBlink = 0; iBlink < 10; iBlink++)
  {
    display.invertDisplay(BlinkState);
    BlinkState = !BlinkState;
    delay(200);
  }
  display.invertDisplay(false);
}

bool toggle = true;
void loop()
{
  if (toggle)
  {
    digitalWrite(PB12, HIGH);
    digitalWrite(PB13, HIGH);
    pwmWrite(PB1, 10000);
    digitalWrite(PB15, HIGH);
  }
  else
  {
    digitalWrite(PB12, LOW);
    digitalWrite(PB13, LOW);
    pwmWrite(PB1, 0);
    digitalWrite(PB15, LOW);
  }
  toggle = !toggle;

  //Scan key-1
  Serial.print(digitalRead(PA8));
  Serial.println(digitalRead(PB4));
  Serial.println(digitalRead(PB5));
  delay(70);
}
