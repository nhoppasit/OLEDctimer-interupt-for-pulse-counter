#define LED_RATE 50  // in microseconds; should give 0.5Hz toggles
int data=0;
int analogPin=3;
int prescaler=4;
int f_pwm=20000; // frequency (Hz)
int DC=40; // % duty cycle
double pwmval;
int dcval=0;
void setup() {
    // Set up the LED to blink
    Serial.begin(115200);
    pinMode(analogPin, INPUT_ANALOG);
    pinMode(PC13, OUTPUT);
    pinMode(PB6, PWM);
    pinMode(PB7, PWM);
    Timer2.pause();
    Timer2.setPeriod(LED_RATE); // in microseconds
    Timer2.setMode(TIMER_CH1,TIMER_OUTPUTCOMPARE);
    Timer2.setCompare(TIMER_CH1, 1); 
    Timer2.attachInterrupt(TIMER_CH1,handler_led);
    Timer2.refresh();
    Timer2.resume();
    Timer4.setPrescaleFactor(prescaler);
    pwmval=(72000000/prescaler)/f_pwm;
    Timer4.setOverflow(pwmval);
    
}

void loop() {
    dcval=analogRead(analogPin);
    dcval=map(dcval,1024,0,pwmval,0);
    Serial.println(dcval);

}

void handler_led(void) {
pwmWrite(PB6,dcval); 
pwmWrite(PB7,pwmval*DC/100);  
}
