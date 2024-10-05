// Define pin connections
int latchPin = 28;  // RCLK
int clockPin = 22; // SRCLK
int dataPin = 23;  // SER

const int digitPins[2] = {27, 26}; // Pins for the 2 digits
const byte segmentPinsOnes[10] = {0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110, 0b10110110, 0b10111110, 0b11100000, 0b11111110, 0b11100110};
const byte segmentPinsTens[10] = {0b00000000, 0b01100000, 0b11011010, 0b11110010, 0b01100110, 0b10110110, 0b10111110, 0b11100000, 0b11111110, 0b11100110};     
int countdown = 20; // countdown time in seconds
int counter = 0;

void setup() {

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  for (int i = 0; i < 2; i++) { 
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], LOW); // Turn off all digits initially
   }

   // Timer1 module overflow interrupt configuration
  TCCR1A = 0;
  TCCR1B = 0b0000001;  // enable Timer1 with prescaler = 1
  TCNT1  = 0;  // set Timer1 preload value to 0 (reset)
  TIMSK1 = 1;  // enable Timer1 overflow interrupt
  OCR1A = 16000; // output compare match when it reaches 16000 (happens 1000 times a second)
}

void loop() {

}
void displayNumber(int num) {

  // Print Tens digit (most left)
  digitalWrite(latchPin,LOW);
  digitalWrite(digitPins[0], HIGH);
  shiftOut(dataPin, clockPin, MSBFIRST, segmentPinsTens[num / 10]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(digitPins[0], LOW);

  // Print Ones digit (most right)
  digitalWrite(latchPin,LOW);
  digitalWrite(digitPins[1], HIGH);
  shiftOut(dataPin, clockPin, MSBFIRST, segmentPinsOnes[num % 10]);
  digitalWrite(latchPin, HIGH);
  digitalWrite(digitPins[1], LOW);

}

ISR(TIMER1_OVF_vect)   // Timer1 interrupt service routine (ISR)
{
  displayNumber(countdown);
  counter++;
  if (counter == 100){
    countdown--;
    counter = 0;
  }
  if (countdown == 0){
    countdown = 20;
  }
}

void clearDisplay() {
  digitalWrite(latchPin, HIGH);
  shiftOut(dataPin, clockPin, MSBFIRST, 0x00);
  digitalWrite(latchPin, LOW);
}

