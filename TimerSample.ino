// EECEE 5250 Lab 1
// Patrick Holman
// 9/21/2024

#include <Keypad.h> 

//initial variables
boolean toggle1 = 0; // toggle bool for interupt routine
byte counter = 0;
byte tr = 0; // time for red light set by user
byte tg = 0; // time for green light set by user

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

void setup(){
  
  Serial.begin(9600);
  pinMode(13, OUTPUT);

  cli();//Clear interrupt flag bit (disable interrupts)

  TCCR1A = 0;// clear TCCR1A 
  TCCR1B = 0;// clear TCCR1B
  TCNT1  = 0;// Clear the 16 bit timer/counter (TCNT1H and TCNT1L)
  OCR1A = 15624;// set 16 bit output compare register to 1 second -> 16MHz/1024prescalar - 1 = 15624 bits
  TCCR1B |= (1 << WGM12); // Set bit 2 of WGM13:0 for CTC mode (Clear Timer on Compare Match)
  TCCR1B |= (1 << CS12) | (1 << CS10); // Set CS12 and CS10 for 1024 prescaler 
  TIMSK1 |= (1 << OCIE1A); // Enable output compare A match interrupt

  sei();//Set interrupt flag bit (re-enable interrupts)

}


// Timer1's interrupt service routing (ISR)
// The code in ISR will be executed every time timer1 interrupt occurs
// That is, the code will be called once every second
// TODO
//   you need to set a flag to trigger an evaluation of the conditions
//   in your state machine, then potentially transit to next state

ISR(TIMER1_COMPA_vect){//timer1 interrupt 1Hz toggles pin 13 (LED)
//generates pulse wave of frequency 1Hz/2 = 0.5kHz (takes two cycles for full wave- toggle high then toggle low)
  if (toggle1){ // If toggle is 1
    digitalWrite(13,HIGH); // Turn on LED
    toggle1 = 0; // reset toggle
  }
  else{ // If toggle is 0
    digitalWrite(13,LOW); // Turn off LED
    toggle1 = 1; // Set toggle
  }
  counter--;
}
  
void loop(){

  char customKey = customKeypad.getKey(); // Get key pressed

  if (toggle1) {
  }
  else {
  }

  if (customKey){ // If key is pressed
    Serial.println(customKey); // Print key to serial monitor
} 
}

