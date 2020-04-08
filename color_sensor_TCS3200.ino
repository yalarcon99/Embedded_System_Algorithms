//Color sensor TCS3200 and Servomotors SG90
//Yithzak Alarcon - Fernando Quintero
//Embedded Systems

//Libraries to use some functions (_delay_ms) and interrupts
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//Variables to count the number of pulses
int counter = 0;
int counterRED = -1;
int counterGRE = -1;
int counterBLU = -1;
int auxRED = -1;
int auxGRE = -1;
int auxBLU = -1;
int classify = 0;
int aux = 0;

void setup() {
  //Define PORTD as outputs
  //PortD
  DDRD = 0B11110000;
  //Define PB0 of PORTB as output
  //Setting the output of the sensor as an input for uC
  //Setting PB1 y PB2 as output for Servomotors
  //PortB
  DDRB = 0B00000111;
  //Setting the outputs to the LEDs
  //PortC
  DDRC = 0B00011111;
  //Setting frequency scaling to 20% 
  //S1: 0V
  PORTD = 0B00100000;
  //S0: 5V
  PORTB = (1<<PB0);
  //Begin Serial Communication 
  Serial.begin(9600);
  //Configure interrupts
  EIMSK = 0B00000001;
  EICRA = 0B00000011;

  //Setting timer2 with a 128-prescaler
  TCCR2B = 0B00000101;
  TIMSK2 = 0B00000001; 
  TCNT2 = 0;
  //Timer1
  TCCR1B = 0x00;
  DDRB = 0xFF;
  //ICR1 = 4999;
  //Poner TOP a 20 ms
  TCCR1A = (1 << COM1A1)|(1 << COM1B1)|(1 << WGM11);
  TCCR1B = (1 << WGM12) |(1 << WGM13) |(1 << CS11)|(1<<CS10);
  //Uso del modo Fast PWM usando ICR1 como TOP
  //Inicio del PWM prescaler 64
  ICR1 = 4999;
  sei();
}
void servo1On(int);
void servo2On(int);
//S0: PB0 and S1: PD5
//S3: PD7 and S2: PD6
//Out: PD2
void loop() {
  aux = 0;
  //Initialize servomotor 1
  servo1On(0);
  servo1On(1);
  
  while(aux<50){
  //Setting RED (R) filtered photodiodes to be read
  classify = 1;
  PORTD = 0 | (1<<PD5);
  _delay_ms(30);
  auxRED += counterRED;
   
  //Setting GREEN (G) filtered photodiodes to be read
  classify = 2;
  PORTD = (1<<PD7)|(1<<PD6)|(1<<PD5);
  _delay_ms(30);
  auxGRE += counterGRE;
  
  //Setting BLUE (B) filtered photodiodes to be read
  classify = 3;
  PORTD = 0B10000000 | (1<<PD5);
  _delay_ms(30);
  auxBLU += counterBLU;
  
  if(aux == 49){
  auxRED = auxRED/50; 
  Serial.print("\nRed = ");
  Serial.print(auxRED);
  auxGRE = auxGRE/50; 
  Serial.print("\tGreen = ");
  Serial.print(auxGRE);
  auxBLU = auxBLU/50; 
  Serial.print("\tBlue = ");
  Serial.print(auxBLU);
  }
  aux++;
  }
  if((auxRED>=43 & auxRED<=46) & (auxGRE>=33 & auxGRE<=36) & (auxBLU>=45 & auxBLU<=49)){
    Serial.print("\nAns: Is Red!");
    PORTC = (1<<PC4); servo2On(0); _delay_ms(20);
  }else if((auxRED>=40 & auxRED<=42) & (auxGRE>=38 & auxGRE<=40) & (auxBLU>=47 & auxBLU<=50)){
    Serial.print("\nAns: Is Green!"); 
    PORTC = (1<<PC2); servo2On(1); _delay_ms(20);
  }else if((auxRED>=37 & auxRED<=42) & (auxGRE>=36 & auxGRE<=40) & (auxBLU>=54 & auxBLU<=58)){
    Serial.print("\nAns: Is Blue!");
    PORTC = (1<<PC0)|(1<<PC2)|(1<<PC4); servo2On(2); _delay_ms(20);
  }else if((auxRED>=40 & auxRED<=42) & (auxGRE>=37 & auxGRE<=38) & (auxBLU>=50 & auxBLU<=52)){
    Serial.print("\nAns: Is Nothing!");
    PORTC = (1<<PC0)|(1<<PC2)|(1<<PC4); servo2On(3); _delay_ms(20);
  }else{
    Serial.print("\nAns: Is not classified!");
    servo2On(4); _delay_ms(20);
  }
  //Restart the main counters
  counterBLU = -1;
  counterGRE = -1;
  counterRED = -1;
  auxRED = -1;
  auxGRE = -1;
  auxBLU = -1;
  _delay_ms(10);
  servo1On(2);
  _delay_ms(3000);
}

void servo1On(int num1){
  if(num1 == 0){
    //Position Zero
    OCR1A = 500;
    _delay_ms(500);
  }else if(num1 == 1){
    for (int i=500; i>=320; i-=3)
    {
      //Sensor position
      OCR1A = i;
      _delay_ms(50);
    }
    OCR1A = 320;
    _delay_ms(2000);
    }else if(num1 == 2){
      for (int i= 320; i>=100; i-=3)
    {
      //Final position
      OCR1A = i;
      _delay_ms(20);
    }
    _delay_ms(1000);
    }
}

void servo2On(int num1){
  if(num1 == 0){
    //Position Zero
    OCR1B = 350;
    _delay_ms(500);
  }else if(num1 == 1){
    //Position One
    OCR1B = 430;
    _delay_ms(500);
    }else if(num1 == 2){
    //Position Two
    OCR1B = 510;
    _delay_ms(500);
}else if(num1 == 3){
    //Position Three
    OCR1B = 280;
    _delay_ms(500);
}else if(num1 == 4){
    //Position Four
    OCR1B = 260;
    _delay_ms(500);
}
}
//Activate the interrupt with each rising edge given by OUT
//Increase de counter with each rising edge
ISR(INT0_vect){
 counter++;
}
//Interrupt by timer2 overflow
//Assign the value of counter and restart
ISR(TIMER2_OVF_vect){                   
    if(classify == 1){
      counterRED = counter; //RED
    }else if(classify == 2){
      counterGRE = counter; //GREEN
    }else if(classify == 3){
      counterBLU = counter; //BLUE
    }
    //Restart the counter
    counter = 0;
}
