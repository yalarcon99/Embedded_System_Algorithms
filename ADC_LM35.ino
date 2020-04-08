//Temperature sensor LM35 with AD Converter and LCD
//Yithzak A. - Fernando Q.
//Embedded Systems

//Libraries
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <avr/interrupt.h>

//LCD pin assignments
#define RS 4
#define EN 5
#define CONTROL 0
#define DATA 1

//Variables declaration
long datos;
long temp;
int tmax = 50;
int tmin = 25;
int enter = 0;
bool pcinton = 0;

void setup() {
  //PORT Configuration
  DDRB  = 0x3F;       //Set PORTB (PB0-PB5) as outputs - Bin: 0B00111111
  DDRD  = 0B10110000; //Set PORTD (PD4,PD5,PD7) as outputs
  DDRC |= (1<<PC0);   //Set PC0 as output for Rele
  //INT interrupts configuration
  EIMSK = 0B00000011;  //Enable INT0 and INT1
  EICRA = 0B00001111;  //Rising edge generates an interrupt
  //PCINT interrupt configuration
  PCICR  = 0B00000100; //Enable PCIE2, which contains PCINT16-23
  PCMSK2 = 0B01000000; //Enable PCINT22, pin PD6
  //ADC configuration
  ADMUX  = 0B01000101;   //VREF= AVcc, IN=ADC5 (PC5)
  ADCSRA = 0B10001111;  // ADC enabled, No auto, ADC interrupt enable, 16MHz/128= 125KHz
  ADCSRB = 0B00000000;  //auto-trigger is disabled, so nothing to do here
  //LCD initialization
  showData();
  //Enable all interrupts
  sei();
  ADCSRA |=  (1 << ADSC);    //First ADC
  PORTD  &= ~(1<<PD7);       //Turn off LED of PD7
}
void loop(){
   //Show data mode
   while(enter==0){
   showDatalive();
   _delay_ms(500);
   if(temp<tmin){
    PORTC |= (1<<PC0);
   }else if(temp>tmax){
    PORTC &= ~(1<<PC0);
   }
   }
   //Configuration mode
   while(enter==1 || enter==2 || enter==3){
   PORTC &= ~(1<<PC0); 
   configStatus();
   _delay_ms(500);
   }
}


void lcdStart()
{
  _delay_ms(16);  //wait for LCD startup: 15ms
  
  writeCommand(CONTROL, 0x20);  // function set: 4-bit bus
  _delay_us(45);    //execution time is 40us
  // this instruction will be handled as 8-bit bus. However the function assumes 4-bit bus
  // this will be ok, because the lower bits will be ignored by the lcd when it is busy
  
  writeCommand(CONTROL, 0x28);  // function set: 4-bit bus, 2 lines, 5x8 chars
  _delay_us(45);    //execution time is 40us
  
  writeCommand(CONTROL, 0x0E);  // display on/off control: display on, cursor on, no blink
  _delay_us(45);    //execution time is 40us
  
  writeCommand(CONTROL, 0x06);  // entry mode set: increment
  _delay_us(45);    //execution time is 40us
  
  writeCommand(CONTROL, 0x01);  // clear display
  _delay_ms(2);   //execution time is 1.64ms
  
  return;
}

void writeCommand(char RS_type, char CMD)
{
  //Set RS line
  if (RS_type) PORTD |= 1<<RS;
  else PORTD &= ~(1<<RS);
  _delay_us(1); // min delay is 140ns (t_AS)
  
  //Set enable
  PORTD |= 1<<EN;
  
  //Place high 4-bit data/command on bus
  PORTB = (PORTB & 0xF0) | (CMD >> 4); 
  _delay_us(1); // min delay is 195ns (t_DSW)
  
  //Note: min Enable pulse is 450ns (t_PWH). At this point this delay is already met
  
  //Clear enable
  PORTD &= ~(1<<EN);
  _delay_us(1); // delay for next data is 300ns
  
  // set enable
  PORTD |= 1<<EN;
  
  // place low 4-bit data/command on bus
  PORTB = (PORTB & 0xF0) | (CMD & 0x0F);
  _delay_us(1); // min delay is 195ns (t_DSW)
  
  // note: min Enable pulse is 450ns (t_PWH). At this point this delay is already met
  
  // clear enable
  PORTD &= ~(1<<EN);
  _delay_us(1); // delay is 10ns (t_H)

  return;
}

void lcdString(char text[])
{
  int i,length;
  
  length= strlen(text);// ver el tamano del vector
  
  for (i=0; i<length; i++)
  {
    writeCommand(DATA, text[i]);
    _delay_us(45);    //execution time is 40us
  }
  
  return;
}

void lcdChar(char c)
{
  writeCommand(DATA, c);
  _delay_us(45);    //execution time is 40us
    
  return;
}

void lcdNumber(int num)
{
  char n[8];
  
  itoa( num, n, 10);//Transforma un entero a string (vector de caracteres)
  lcdString(n);
  
  return;
}

void lcdFloat(float num)
{
  int i,d;
  float n;
  
  i= (int) num;   //get integer part
  if ( num < 0 ) n= i - num;    //get fraction
  else n= num - i;      
  d= (int)( n*10 );  //convert fraction to integer: 2 digits
  
  lcdNumber( i );
  lcdChar( '.' );
  lcdNumber( d );
  
  return;
}
void setLcdCursor(char line, char col)
{
  char position=0x00;
  
  //Make sure position is inside limits
  if ( line < 0 ) line= 0;  //2-line lcd
  if ( line > 1 ) line= 1;
  if ( col < 0 ) col= 0;    //16-char lcd
  if ( col > 15 ) col= 15;
  
  if ( line == 0 ) position= 0x80 + col;
  if ( line == 1 ) position= 0xC0 + col;
  
  writeCommand(CONTROL, position);    //Set DDRAM address to position
  _delay_us(45);    //Execution time is 40us
  
  return;
}
void clearLcd()
{
  writeCommand(CONTROL, 0x01);  //Clear display
  _delay_ms(2);   //Execution time is 1.64ms
  
  return;
}

void configStatus(){
  _delay_ms(25);
  clearLcd();
  lcdString("Td=");lcdNumber(tmin);lcdChar(223);lcdString("C");lcdString(" Tu=");lcdNumber(tmax);lcdChar(223);lcdString("C");
  setLcdCursor(1,0);
  lcdString("Configuration...");
}

void showData(){
  lcdStart();
  lcdString("Yithzak");
  setLcdCursor(1,0);
  lcdString("A.S");
  _delay_ms(25);
  clearLcd();
  lcdString(" Td=");lcdNumber(tmin);lcdString(" Tu=");lcdNumber(tmax);
  setLcdCursor(1,0);
  lcdString("   T(act)=");lcdNumber(temp);
}

void showDatalive(){
  _delay_ms(25);
  clearLcd();
  lcdString("Td=");lcdNumber(tmin);lcdChar(223);lcdString("C");lcdString(" Tu=");lcdNumber(tmax);lcdChar(223);lcdString("C");
  setLcdCursor(1,0);
  lcdString("  T(act)=");lcdNumber(temp);lcdChar(223);lcdString("C");
}
//Increase the temperature values
ISR(INT0_vect)
{
   if(enter==2){ 
    tmin++;
   }else if(enter==3){
    tmax++;
   }
   _delay_ms(50);
}
//Decrease the temperature values
ISR(INT1_vect)
{
   if(enter==2){ 
    tmin--;
   }else if(enter==3){
    tmax--;
   }
   _delay_ms(50);
}
//PCIE2 subroutine
//PD6 - PCINT22
//Enter
ISR(PCINT2_vect)
{
   while(PIND & (1<<PD6)){pcinton = 1;}
   if(pcinton){
    enter++;
    PORTD |= (1<<PD7);
    if(enter>=4) enter = 0;
    pcinton = 0;
   }
}
//Data acquisition and ADC converter
ISR(ADC_vect)
{
  datos = ADCW;         // read ADC result
  temp = datos*500/1024;
  ADCSRA |= (1 << ADSC);
}
