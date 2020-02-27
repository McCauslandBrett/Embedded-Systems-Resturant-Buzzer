/*	Author: brettmccausland
 *  Partner(s) Name: Jeremy
 *	Lab Section:
 *	Assignment: Lab 11  Exercise 3
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
  #include  <avr/io.h>
  // -- added from provided source files --
  #include "io.h"
  #include "keypad.h"
  // #include "scheduler.h"
  #include <avr/interrupt.h>
  #ifdef _SIMULATE_
  #include "simAVRHeader.h"
  #endif

  // -------- State Varibles -------------

  enum display_States {INIT,START,DISPLAY,PAUSE_REL,PAUSE_PRESS,START_PRESS};
  enum KEYPAD {READ};
  // -------- End State Varibles -------------

  // -------- Shared Varibles -------------
   unsigned char temp;
  // -------- End Shared Varibles -------------

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
  int setKeypad(){
    unsigned char x;
    x = GetKeypadKey();
    switch(x){
      // case '\0':temp = '';break;
      case '1': temp = '1';break;
      case '2': temp = '2';break;
      case '3': temp = '3';break;
      case '4': temp = '4';break;
      case '5': temp = '5';break;
      case '6': temp = '6';break;
      case '7': temp = '7';break;
      case '8': temp = '8';break;
      case '9': temp = '9';break;
      case 'A': temp = 'A';break;
      case 'B': temp = 'B';break;
      case 'C': temp = 'C';break;
      case 'D': temp = 'D';break;
      case '*': temp = '*';break;
      case '0': temp = '0';break;
      case '#': temp = '#';break;
      // default:  temp = 0x1B;break;
    }
    return READ;

  }

 int displaySMTick(int state){
  static unsigned char old_temp;

   switch (state) {
     case INIT:
       state = DISPLAY;
       old_temp=temp;

        break;
     case DISPLAY:
       if (old_temp != temp ){
         LCD_ClearScreen();
         LCD_WriteData(temp);
         old_temp=temp;
       }
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



int main(void) {
    /* Insert DDR and PORT initializations */
  DDRC = 0xFF; PORTC= 0x00;  //LED SCREEN
  DDRD = 0xFF; PORTD = 0x00; //LED SCREEN
  DDRA = 0xF0; PORTA = 0xFF; //KEYPAD

    LCD_init();
    LCD_ClearScreen();
    unsigned short i; //scheduler for loop

    static task task1,task2;
    task* tasks[] = {&task1,&task2};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);


    // Task 1 (getkeypad)
    task1.state = READ;
    task1.period = 10;
    task1.elapsedTime = task1.period;
    task1.TickFct = &setKeypad;

    // Task 2 (displaySM)
    task2.state = INIT;
    task2.period = 300;
    task2.elapsedTime = task2.period;
    task2.TickFct = &displaySMTick;

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
