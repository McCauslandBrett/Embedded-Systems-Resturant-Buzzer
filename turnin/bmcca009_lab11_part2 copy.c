/*	Author: brettmccausland
 *  Partner(s) Name: Jeremy
 *	Lab Section:
 *	Assignment: Lab 11  Exercise 1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
  #include  <avr/io.h>
  // -- added from provided source files --
  #include "io.h"
  // #include "keypad.h"
  // #include "scheduler.h"
  #include "keypad.h"
  #include <avr/interrupt.h>
  #ifdef _SIMULATE_
  #include "simAVRHeader.h"
  #endif

  // -------- State Varibles -------------

  enum display_States {INIT,START,DISPLAY,PAUSE_REL,PAUSE_PRESS,START_PRESS};
  // -------- End State Varibles -------------

  // -------- Shared Varibles -------------

  char msg[53]= {    'C','S','1','2','0','B',' ','i',
                     's',' ','L','e','g','e','n','d',
                     '.','.','.',' ','w','a','i','t',
                     ' ','f','o','r',' ','i','t',' ',
                     'D','A','R','Y','!',' ',' ',' ',
                     ' ',' ',' ',' ',' ',' ',' ',' ',
                     ' ',' ',' ',' ',' '};

  unsigned char A0;
  // -------- End Shared Varibles -------------

 void shift_left(char _dis[],int size){
   for (int i=0;i<size-1;i++ )
   {
     _dis[i]= _dis[i+1];
   }
 }
 void concat(char _dis[],char _msg[],int i,int size){
    _dis[size-1]= _msg[i];
 }
 void newDisplay(char _dis[],char _msg[],int i,int size){
   shift_left(_dis,size);
   concat(_dis,_msg,i,size);

 }
  // -------- Clock Varibles -------------
  volatile unsigned char TimerFlag = 0;
  unsigned long _avr_timer_M = 1;
  unsigned long _avr_timer_cntcurr = 0;
// -------- End Clock Varibles -------------
//  -------- scheduler -------------
typedef struct _task{
	// Tasks should have members that include: state, period,
	//a measurement of elapsed time, and a function pointer.
	signed char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;
// -------- End scheduler -------------

  // -------- Tick Functions -------------

 int displaySMTick(int state){
   static unsigned char c;
   static unsigned char k;


   // read
   A0= ~PINA & 0x01;
   switch (state) {
     case INIT:
       state = START;
        break;
      case START:
       state = (A0) ? START: DISPLAY;
       break;
      case DISPLAY:
        state = (!A0) ? DISPLAY:PAUSE_PRESS;
        break;
      case PAUSE_PRESS:
        state = (A0) ? PAUSE_PRESS: PAUSE_REL;
        break;
      case PAUSE_REL:
        state = (!A0) ? PAUSE_REL:START_PRESS;
        break;
      case START_PRESS:
        state = (A0) ? START_PRESS:DISPLAY;
        break;
   }
   switch (state) { // actions
     case INIT:
        break;
      case START:
        k=0;
        static char di[]={' ',' ',' ',' ',' ',' ',' ',' ',
                          ' ',' ',' ',' ',' ',' ',' ',' ','\0'};
        break;
      case DISPLAY:

        LCD_DisplayString(1,di);
        newDisplay(di,msg,k,16);
        if (k < 53){k++;}
        if (k >=53){k=0;}
        break;
      case PAUSE_PRESS:
        break;
      case PAUSE_REL:
        break;
      case START_PRESS:
        break;
   }

   return state;
 }
// -------- End Tick Functions -------------



// -------- Timmer Functions -------------
  void TimerOn(){
    TCCR1B = 0x0B;
    OCR1A = 125;
    TIMSK1 = 0x02;
    TCNT1 = 0;
    _avr_timer_cntcurr = _avr_timer_M;
    SREG |= 0x80;

  }
  void TimerOff() {
   TCCR1B = 0x00;
  }
  void TimerISR(){
    TimerFlag = 1;
  }
  ISR(TIMER1_COMPA_vect){
    _avr_timer_cntcurr--;
    if(_avr_timer_cntcurr == 0){
      TimerISR();
      _avr_timer_cntcurr = _avr_timer_M;
    }
  }
  // set timer to tick every M ms
  void TimerSet(unsigned long M){
    _avr_timer_M=M;
    _avr_timer_cntcurr = _avr_timer_M;
  }
// -------- End Timmer Functions -------------

void setKeypad(){
  unsigned char x;
  x = GetKeypadKey();
  switch(x){
    case '\0':PORTB = 0x1F;break;
    case '1': PORTB = 0x01;break;
    case '2': PORTB = 0x02;break;
    case '3': PORTB = 0x03;break;
    case '4': PORTB = 0x04;break;
    case '5': PORTB = 0x05;break;
    case '6': PORTB = 0x06;break;
    case '7': PORTB = 0x07;break;
    case '8': PORTB = 0x08;break;
    case '9': PORTB = 0x09;break;
    case 'A': PORTB = 0x0A;break;
    case 'B': PORTB = 0x0A;break;
    case 'C': PORTB = 0x0A;break;
    case 'D': PORTB = 0x0D;break;
    case '*': PORTB = 0x0E;break;
    case '0': PORTB = 0x00;break;
    case '#': PORTB = 0x0F;break;
    default:  PORTB = 0x1B;break;
  }

}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRC = 0xFF; PORTC= 0x00;
  DDRD = 0xFF; PORTD = 0x00;
  DDRA = 0x00; PORTA = 0xFF; // Configure port A's as inputs
  DDRB = 0xFF; PORTB = 0x00; // output, init to 0's

    LCD_init();
    LCD_ClearScreen();



    unsigned short i; //scheduler for loop

    static task task1;
    task* tasks[] = {&task1};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);


    // Task 1 (displaySM)
    task1.state = INIT;
    task1.period = 500;
    task1.elapsedTime = task1.period;
    task1.TickFct = &displaySMTick;

    TimerSet(10); // need to set and create var
    TimerOn();
    while (1) {
      for(i=0; i<numTasks;i++){
        if (tasks[i]->elapsedTime == tasks[i]->period){ //task ready to tick
          tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);// Set Next state
          tasks[i]->elapsedTime=0; //Reset the elapsed time
        }
        tasks[i]->elapsedTime += 10;

      }
      while(!TimerFlag);
      TimerFlag = 0;

    }
    return 0;
}
