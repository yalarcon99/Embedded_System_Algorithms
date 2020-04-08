//Ultrasound sensor - HC-SR04 - Robot Car
//Yithzak Alarcon - Fernando Quintero
//Embedded Systems

//Libraries to use some functions (_delay_ms) and interrupts
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//Defines pins numbers
//trigPin -> 4 - PD4
//echoPin -> 2 - PD2
const int trigPin = 4;
const int echoPin = 2;

//Defines variables
long timecalc; double distance;
long counter; long suma;
long result; long timer;
long distIzq; long distCen;
long distDer; int i=0; int a;

void setup() {
DDRD = (1<<trigPin)||~(1<<echoPin); //trigger Pin - Output | echo Pin - Input
DDRD = 0b11111000; 
DDRB = 0B11111110;//Port B as output.
DDRC = 0x0F;

//Configure timer0 - SERVOMOTOR
TCCR0A = (1<<WGM01)|(1<<WGM00)|(1<<COM0B1)|(1<<COM0A1); 
TCCR0B = (1<<WGM02)|(1<<WGM13)|(1<<CS02)|(1<<CS00);
OCR0A = 255; //FPWM = FOSC / ( N * ( 1 + TOP ) )a

//Configure timer1 - ULTRASOUND
TCCR1A = TCCR1B = 0;
TCNT1 = 0;
distance = 0; counter = 0; a = 0;

//Configure timer2 - MOTORS
TCCR2A = (1<<COM2A1)|(1<<COM2B1)|(1<<COM0B1)|(1<<WGM20);
TCCR2B = (1<<CS21);
OCR2A = 150;
OCR2B = 150;
Avanzar();

//Starts the serial communication
sei();
}

long getDistance();

void loop(){
//El servomotor hace un barrido
 for (i= 6; i<=20; i+=1)
 {
   OCR0B = i;
   Medicion(); //Función que activa el ultrasonido y detecta obstaculos
   _delay_ms(10);
 }
 for (int i= 20; i>=6; i=i-1)
 {
   OCR0B = i;
   Medicion(); //Función que activa el ultrasonido y detecta obstaculos
   _delay_ms(10);
 }

}

long getDistance(){
//Clears the Trig pin
PORTD &= ~(1<<PD4);
_delay_us(5);
//Set the Trig pin on '1' for 10 micro seconds
PORTD |= (1<<PD4);
_delay_us(10);
//Clears the Trig pin
PORTD &= ~(1<<PD4);
_delay_us(5);
//Wait until ECHO pin change to 5V
while(!(PIND & (1<<echoPin)));
//Set the TIMER1 
TCCR1B = 0B00000011; //Set 64-prescaler
while((PIND & (1<<echoPin)) && TCNT1 < 8000);
TCCR1B = 0;
timer = TCNT1;
//Calculating the distance [(time * 0.034 cm/us)/2]
distance = (double)timer*0.017*4;
TCNT1 = 0;
return distance;
}

void Medicion(){
  distCen = getDistance(); //Se obtiene la medición en cm
  _delay_ms(10);
  if(distCen > 0 & distCen < 30){
    Action(); //Se decide el sentido de los motorreductores
  }
  else{
    _delay_us(10);
  }
}

void Action(){
  Detenerse();
  _delay_ms(500);
  Reversa();
  _delay_ms(500);
  if(OCR0B<20){
  Derecha();
  _delay_ms(500);
  }
  else{
  Izquierda();
  _delay_ms(500);
  }
  Avanzar();
}

//FORWARD
void Avanzar(){
PORTC |=  (1<<PC0);
PORTC &= ~(1<<PC1);
PORTC |=  (1<<PC2);
PORTC &= ~(1<<PC3);}

//STOP
void Detenerse(){
PORTC &= ~(1<<PC0);
PORTC &= ~(1<<PC1);
PORTC &= ~(1<<PC2);
PORTC &= ~(1<<PC3);}

//REVERSE
void Reversa(){
PORTC &= ~(1<<PC0);
PORTC |=  (1<<PC1);
PORTC &= ~(1<<PC2);
PORTC |=  (1<<PC3);}
 
//LEFT
void Izquierda(){
PORTC &= ~(1<<PC0);
PORTC |=  (1<<PC1);
PORTC |=  (1<<PC2);
PORTC &= ~(1<<PC3);}

//RIGHT
void Derecha(){
PORTC |=  (1<<PC0);
PORTC &= ~(1<<PC1);
PORTC &= ~(1<<PC2);
PORTC |=  (1<<PC3);}
