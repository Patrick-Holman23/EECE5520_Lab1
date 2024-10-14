// EECEE 5250 Lab 1
// Patrick Holman
// 9/21/2024

#include <Keypad.h> 

//initial variables
bool setup_status = 1; // bit to set for startup routine status - initialized as 1 for startup and will clear on completion
bool onehz_toggle = 1; // toggle bool for 1hz interrupt routine
bool twohz_toggle = 0; // toggle bool for 2hz interrupt routine
bool red_startup_flash = 1; // bit to set if red light1 should be flashing 1s ON 1s OFF - initialzied as 1 for startup routine
bool green1_twohz_flash = 0; // bit to set if green light1 should be flashing .5s ON .5s OFF
bool red1_twohz_flash = 0; // bit to set if red light1 should be flashing .5s ON .5s OFF
bool green2_twohz_flash = 0; // bit to set if green light2 should be flashing .5s ON .5s OFF
bool red2_twohz_flash = 0; // bit to set if red light2 should be flashing .5s ON .5s OFF
int counter = 0; // Counter for light sequence time tracking and seven segment display

int TR1 = 0; // first digit of red light time - set by user
int TG1 = 0; // first digit of green light time - set by user
int TR = 0; // full time for red light - set by user
int TG = 0; // full time for green light - set by user

// Seven Segment Display setup
int latchPin = 24;  // RCLK of HC595
int clockPin = 22; // SRCLK of HC595
int dataPin = 23;  // SER of HC595
const int digitPins[2] = {26, 27}; // Pins for the 2 digits { Tens digit, Ones digit}
const byte segmentPinsOnes[10] = {0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110, 0b10110110, 0b10111110, 0b11100000, 0b11111110, 0b11100110};
const byte segmentPinsTens[10] = {0b00000000, 0b01100000, 0b11011010, 0b11110010, 0b01100110, 0b10110110, 0b10111110, 0b11100000, 0b11111110, 0b11100110};
bool display_toggle = 0; // Used to set which digit is displayed

enum {keyA, keyTR1, keyTR, keyPoundR, keyB, keyTG1, keyTG, keyPoundG, keyStar, Red, Green2_3s, Red3s, Green, Green3s, Yellow};
unsigned char trafficState;

int RED_LED1 = 36;
int YELLOW_LED1 = 37;
int GREEN_LED1 = 38;
int RED_LED2 = 41;
int YELLOW_LED2 = 42;
int GREEN_LED2 = 40;
int BUZZER = 49;

const byte ROWS = 4; //four rows for keypad
const byte COLS = 4; //four columns for keypad
char hexaKeys[ROWS][COLS] = { 
  {'1','2','3','A'}, 
  {'4','5','6','B'}, 
  {'7','8','9','C'}, 
  {'*','0','#','D'} 
}; 
byte rowPins[ROWS] = {9, 8, 7, 6}; // define row pins
byte colPins[COLS] = {5, 4, 3, 2}; // define column pins
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); // Create keypad class
char customKey;

void setup(){
  
  Serial.begin(9600);
  pinMode(RED_LED1, OUTPUT);
  pinMode(YELLOW_LED1, OUTPUT);
  pinMode(GREEN_LED1, OUTPUT);
  pinMode(RED_LED2, OUTPUT);
  pinMode(YELLOW_LED2, OUTPUT);
  pinMode(GREEN_LED2, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  trafficState = keyA; // Initial state is waiting for KeyA

  pinMode(latchPin, OUTPUT); 
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  for (int i = 0; i < 2; i++) { 
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], LOW); // Turn off all digits initially
   }

  cli(); //Clear interrupt flag bit (disable interrupts)
  // 1hz interrupt for time tracking and red startup light flashing
  TCCR1A = 0; // clear TCCR1A 
  TCCR1B = 0; // clear TCCR1B
  TCNT1  = 0; // Clear the 16 bit timer/counter (TCNT1H and TCNT1L)
  OCR1A = 15624; // set 16 bit output compare register to 1 second -> (16MHz / 1024prescalar) * 1second - 1 = 15624 bits
  TCCR1B |= (1 << WGM12); // Set bit 2 of WGM13:0 for CTC mode (Clear Timer on Compare Match)
  TCCR1B |= (1 << CS12) | (1 << CS10); // Set CS12 and CS10 for 1024 prescaler 
  TIMSK1 |= (1 << OCIE1A); // Enable output compare A match interrupt

  // 2hz interrupt for red and green light flashing for 3 seconds before light change
  TCCR3A = 0; // clear TCCR2A 
  TCCR3B = 0; // clear TCCR2B
  TCNT3  = 0; // Clear the 16 bit timer/counter (TCNT1H and TCNT1L)
  OCR3A = 31249; // set 8 bit output compare register to 2hz -> (16MHz / 256 prescalar) * 0.5seconds - 1 = 31249 bits
  TCCR3B |= (1 << WGM12); // Set bit 2 of WGM13:0 for CTC mode (Clear Timer on Compare Match)
  TCCR3B |= (1 << CS12); // Set CS12 256 prescaler 
  TIMSK3 |= (1 << OCIE1A); // Enable output compare A match interrupt

  // 120hz interrupt for seven segment display
  TCCR4A = 0; // clear TCCR2A 
  TCCR4B = 0; // clear TCCR2B
  TCNT4  = 0; // Clear the 16 bit timer/counter (TCNT1H and TCNT1L)
  OCR4A = 16666; // set 8 bit output compare register to 120hz -> (16MHz / 8prescalar ) / 120hz - 1 ~ 16666 bits
  TCCR4B |= (1 << WGM12); // Set bit 2 of WGM13:0 for CTC mode (Clear Timer on Compare Match)
  TCCR4B |= (1 << CS11); // Set CS11 8 prescaler 
  TIMSK4 |= (1 << OCIE1A); // Enable output compare A match interrupt
  sei();//Set interrupt flag bit (re-enable interrupts)

}


ISR(TIMER1_COMPA_vect){ // 1Hz Interrupt Routine

  if (counter > 0){
  counter--; // Decrement counter for light sequence time tracking and seven segment display
  }

  if (red_startup_flash == 1){
    if (onehz_toggle){ // If toggle is 1
    digitalWrite(RED_LED1, HIGH); // Turn on LED
    digitalWrite(RED_LED2, HIGH); // Turn on LED
    onehz_toggle = 0; // reset toggle
    }

    else{ // If toggle is 0
    digitalWrite(RED_LED1, LOW); // Turn off LED
    digitalWrite(RED_LED2, LOW); // Turn on LED
    onehz_toggle = 1; // Set toggle
    }
  }
}

ISR(TIMER3_COMPA_vect){

  if (red1_twohz_flash == 1){
    if (twohz_toggle){ // If toggle is 1
    digitalWrite(RED_LED1, HIGH); // Turn on LED
    twohz_toggle = 0; // reset toggle
    }

    else{ // If toggle is 0
    digitalWrite(RED_LED1, LOW); // Turn off LED
    twohz_toggle = 1; // Set toggle
    }
  }

  if (green1_twohz_flash == 1){
    if (twohz_toggle){ // If toggle is 1
    digitalWrite(GREEN_LED1, HIGH); // Turn on LED
    twohz_toggle = 0; // reset toggle
    }

    else{ // If toggle is 0
    digitalWrite(GREEN_LED1, LOW); // Turn off LED
    twohz_toggle = 1; // Set toggle
    }
  }

  if (red2_twohz_flash == 1){
    if (twohz_toggle){ // If toggle is 1
    digitalWrite(RED_LED2, HIGH); // Turn on LED
    twohz_toggle = 0; // reset toggle
    }

    else{ // If toggle is 0
    digitalWrite(RED_LED2, LOW); // Turn off LED
    twohz_toggle = 1; // Set toggle
    }
  }

  if (green2_twohz_flash == 1){
    if (twohz_toggle){ // If toggle is 1
    digitalWrite(GREEN_LED2, HIGH); // Turn on LED
    twohz_toggle = 0; // reset toggle
    }

    else{ // If toggle is 0
    digitalWrite(GREEN_LED2, LOW); // Turn off LED
    twohz_toggle = 1; // Set toggle
    }
  }
}

ISR(TIMER4_COMPA_vect){

  clearDisplay(); // Turn off digits

  // delayMicroseconds(1000); // Small delay for FET to turn off

  display_toggle ^= 1; // Toggle display bool

  // Shift out screen bits
  if(display_toggle == 0){
    displayNumber(0, counter / 10 ); // Shift out data for tens digit
  }
  else{
    displayNumber(1, counter % 10); // Shift out data for ones digit
  }

  // Turn display back on
  if(display_toggle == 0){
    digitalWrite(digitPins[0], HIGH); // Turn on tens digit
  }
  else{
    digitalWrite(digitPins[1], HIGH); // Turn on ones digit
  }
}

void loop(){

  switch (trafficState) {

    case keyA: // Get the first key A for red light configuration
      Serial.println("Waiting for keyA: ");
      customKey = customKeypad.getKey();
      if (customKey == 'A') { // Key A has been pressed
        trafficState = keyTR1; 
        break;
      }
      else {  
        break; 
      }

    case keyTR1: // Get the first digit for red light time
      Serial.println("Waiting for keyTR1");
      customKey = customKeypad.getKey();
      if (customKey >= '0' && customKey <= '9') { // If key pressed is an integer
        TR1 = int(customKey - '0');
        trafficState = keyTR; 
        break;
      }
      else {
        break;
      }

    case keyTR: // Get the second digit for red light time
      Serial.println("Waiting for full KeyTR");
      customKey = customKeypad.getKey();
      if (customKey >= '0' && customKey <= '9') { // If key pressed is an integer
        TR = TR1*10 + int(customKey - '0');
        trafficState = keyPoundR; 
        break;
      }
      else { 
        break;
      }

    case keyPoundR: // Get the pound sign to complete red light configuration
      Serial.println("Waiting for keyPoundR");
      customKey = customKeypad.getKey();
      if (customKey == '#') { 
        trafficState = keyB; 
        break;
      }
      else {  
        break; 
      }

    case keyB: // Get the first key B for green light configuration
      Serial.println("Waiting for keyB");
      customKey = customKeypad.getKey();
      if (customKey == 'B') { // Key A has been pressed
        trafficState = keyTG1; 
        break;
      }
      else {  
        break; 
      }

    case keyTG1: // Get the first digit for green light time
      Serial.println("Waiting for keyTG1");
      customKey = customKeypad.getKey();
      if (customKey >= '0' && customKey <= '9') { // If key pressed is an integer
        TG1 = int(customKey - '0');
        trafficState = keyTG; 
        break;
      }
      else {   
        break; 
      }

    case keyTG: // Get the second digit for green light time
      Serial.println("Waiting for keyTG");
      customKey = customKeypad.getKey();
      if (customKey >= '0' && customKey <= '9') { // If key pressed is an integer
        TG = TG1*10 + int(customKey - '0');
        trafficState = keyPoundG; 
        break;
      }
      else {  
        break; 
      }

    case keyPoundG: // Get the pound sign to complete green light configuration
      Serial.println("Waiting for keyPoundG");
      customKey = customKeypad.getKey();
      if (customKey == '#') { 
        trafficState = keyStar; 
        break;
      }
      else {  
        break; 
      }

    case keyStar: // Get the pound sign to start the traffic lights
      Serial.println("Waiting for keyStar");
      customKey = customKeypad.getKey();
      if (customKey == '*') {
        red_startup_flash = 0; // Stop flashing red light
        digitalWrite(RED_LED1, LOW);
        digitalWrite(RED_LED2, LOW);
        TCNT1  = 0; // reset timer1 register so that the full red time will occur
        counter = TR; // Set counter to red light time
        trafficState = Red; 
        break;
      }
      else {  
        break; 
      }

    case Red: // Start Red light sequence
      Serial.println("Starting Red light sequence");
      digitalWrite(RED_LED1, HIGH); 
      digitalWrite(GREEN_LED2, HIGH); // Turn on green light for opposing traffic
      if (counter <= 6) { // When counter = 6 the green light for opposing traffic is 3 seconds away from changing
        TCNT3 = 0; // Reset timer3 register
        boolean twohz_toggle = 0; // Make this zero so LED is off after first half second
        trafficState = Green2_3s;
      }
      else {  
        break; 
      }

    case Green2_3s:
      green2_twohz_flash = 1; // Start flashing green light for opposing traffic
      digitalWrite(BUZZER, HIGH); // Activate buzzer
      if (counter <= 3) {
        digitalWrite(GREEN_LED2, LOW); 
        digitalWrite(BUZZER, LOW);
        green2_twohz_flash = 0;
        TCNT3 = 0; // Reset timer3 register
        boolean twohz_toggle = 0; // Make this zero so LED is off after first half second
        trafficState = Red3s; 
        break;
      }
      else {  
        break; 
      }

    case Red3s: // Start flashing red every half a second
      red1_twohz_flash = 1;
      digitalWrite(BUZZER, HIGH);
      digitalWrite(YELLOW_LED2, HIGH); // Turn on yellow light for opposing traffic since forward traffic is 3 seconds away from turning green
      if (counter == 0) { 
        red1_twohz_flash = 0;
        digitalWrite(RED_LED1, LOW);
        digitalWrite(YELLOW_LED2, LOW);
        digitalWrite(BUZZER, LOW);
        counter = TG;
        trafficState = Green; 
        break;
      }
      else {  
        break; 
      }

    case Green: // Start Green light sequence
      Serial.println("Starting Green light sequence");
      digitalWrite(GREEN_LED1, HIGH);
      digitalWrite(RED_LED2, HIGH); // Turn on red light for opposing traffic since forward traffic has green
      if (counter <= 3) {
        TCNT3 = 0; // Reset timer3 register
        boolean twohz_toggle = 0; // Make this zero so LED is off after first half second
        trafficState = Green3s; 
        break;
      }
      else {  
        break; 
      }

    case Green3s: // Start flashing green every half a second
      green1_twohz_flash = 1;
      digitalWrite(BUZZER, HIGH);
      if (counter == 0) { 
        green1_twohz_flash = 0;
        digitalWrite(GREEN_LED1, LOW);
        digitalWrite(BUZZER, LOW);
        counter = 3;
        trafficState = Yellow; 
        break;
      }
      else {  
        break; 
      }

    case Yellow: // Start Yellow light sequence
      digitalWrite(YELLOW_LED1, HIGH);
      digitalWrite(BUZZER, HIGH);
      red2_twohz_flash = 1;
      if (counter == 0) {
        digitalWrite(YELLOW_LED1, LOW);
        digitalWrite(RED_LED2, LOW);
        digitalWrite(BUZZER, LOW);
        red2_twohz_flash = 0;
        counter = TR; 
        trafficState = Red; 
        break;
      }
      else {  
        break; 
      }

    default:
      Serial.println("Default State - return to start");
      trafficState = keyA;
      break;
  }
}

void displayNumber(bool digit, int num) { // Function for seven segment display

  if (digit == 0) { // Display Tens digit (left)
    digitalWrite(latchPin,LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, segmentPinsTens[num]);
    digitalWrite(latchPin, HIGH);
  }
  else { // Display Ones digit (right)
    digitalWrite(latchPin,LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, segmentPinsOnes[num]);
    digitalWrite(latchPin, HIGH);
  }

}

void clearDisplay() {
  digitalWrite(digitPins[0], LOW);
  digitalWrite(digitPins[1], LOW);
}
