//Modified keypad
//Yithzak Alarcon - Fernando Quintero
//PB0-3: rows (inputs)
//PC0-3: columns (outputs)
//PD0-3: key result (outputs for leds)

#include <avr/io.h>
#include <util/delay.h>

#define rows 4
#define cols 4
const int rowPins[rows]= {0, 1, 2, 3};  //Row pins
const int colPins[cols]= {0, 1, 2, 3};  //Column pins
const int numberPre[4] = {2, 0, 1, 9};  //Predefined number

//Variables and counters
int number = -1;
int counterA = 0;
int verifKey = 0;
int countBlock = 0;
int countVerif = 0;
bool check = 0;

int keys[rows][cols]= 
{
  {1, 2, 3, 10},
  {4, 5, 6, 11},
  {7, 8, 9, 12},
  {14, 0, 15, 13}
};


void setup() 
{
  //Port configuration
  DDRB= 0x00;
  DDRC= 0xFF;
  DDRD= 0B00111111;
}

//Function main
void loop() 
{ 
  //Store the key obtained into the variable 'number'
  number = keypad();
  //Put the conditional to receive numbers only from 0 to 9
  if(number>=0 & number<10){
    //Turn on each led one by one and depends of the keystroke
    PORTD |= 1<<counterA;
    _delay_ms(500);
    //Condition to compare number digit by digit with the predefined number
    if(numberPre[counterA] == number)
      verifKey++;
    //Activate Red or Green LED
    if(verifKey==4 & counterA==3){
      //Activate Green LED
      PORTD = 0B00010000;
      _delay_ms(1000);
      verifKey = 0;
      countVerif = 0;
      countBlock = 0;
      check = 1;
      //Desactivate Green LED
      PORTD = 0B00000000;
    }else if(verifKey<3 & counterA==3){
      //Activate Red LED
      PORTD = 0B00100000;
      _delay_ms(1000);
      verifKey = 0;
      countVerif++;
      //Desactivate Red LED
      PORTD = 0B00000000;
    }
    counterA++;
    //Restart the main counter and increase the counter until lockout
    if(counterA>3){
      counterA = 0;
      PORTD &= 0B00000000;
      countBlock++;
    }
    //Use 'countVerif' to check the user get wrong key three times
    //Use 'countBlock' to count the user make three attempts
    if(countBlock==3 & countVerif<10){
      flash();
      countVerif = 0;
      countBlock = 0;
    }
    if(check){
      check = 0;
      countVerif = 0;
      countBlock = 0;
      counterA = 0;
     }
    }
    number = -1;
}

//Function to blink the red LED
void flash(){
  int i = 0;
  while(i < 10){
    PORTD = 0B00100000;
    _delay_ms(500);
    PORTD = 0B00000000;
    _delay_ms(500);
    i++;
  }
}

//Keypad function
int keypad()
{
  int i,j,tecla,flag;

  tecla= -1;
  //Make sure all cols are inactive
  PORTC= 0;

  //Start scanning
  flag= 0;
  for ( i=0; i<cols; i++ )
  {
    //Activate i column
    PORTC |= 1 << colPins[i];

    //Check rows
    for ( j=0; j<rows; j++ )
    {
      if ( PINB & (1<<rowPins[j]) )
      {
        _delay_ms(5); //Wait 5ms for rebound and read again
        if ( PINB & (1<<rowPins[j]) )
        {
          tecla= keys[j][i];
          flag= 1; //To stop row loop
          break;
        }
      }
    }

    //Desactivate i column and stop if key detected
    PORTC &= ~(1 << colPins[i]);
    if (flag == 1) break;
  }
  return tecla;
}
