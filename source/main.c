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
   #include "nokia5110.h"
  #include <avr/interrupt.h>
  #ifdef _SIMULATE_
  #include "simAVRHeader.h"
  #endif
 #include "usart.h"


// -------- Bluetooth HC-05 MODULE -------------
 unsigned char Data_in;
 unsigned char blt;

  // -------- State Varibles -------------

  enum Bluetooth_States {ON_BLT,OFF_BLT,BLT_INIT};
  enum PWM_States {HIGH,LOW,OFF,PWM_INIT};
  enum NOKIA_States {NOKIA_INIT,WAITING,RETURNING};

  // -------- End State Varibles -------------

  // -------- Shared Varibles -------------

   unsigned char H,L;
   unsigned char k=0x00;
  // -------- End Shared Varibles -------------

  // -------- Clock Varibles -------------
  volatile unsigned char TimerFlag = 0;
  unsigned long _avr_timer_M = 1;
  unsigned long _avr_timer_cntcurr = 0;
// -------- End Clock Varibles -------------
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
int BluetoothTick(int state){
 Data_in = USART_Receive(); /* receive data from Bluetooth device*/
 nokia_lcd_clear();
 nokia_lcd_write_string("Blue",1);
 nokia_lcd_set_cursor(0, 10);
 nokia_lcd_write_string(Data_in, 1);
 nokia_lcd_render();
 switch(state){
   case BLT_INIT:
     state = OFF_BLT;
     break;
   case ON_BLT:
     state = (Data_in =='2') ? OFF_BLT: ON_BLT;
     break;

   case OFF_BLT:
      state = (Data_in  =='1') ? ON_BLT:OFF_BLT;
     break;

   }
   switch(state){
     case ON_BLT:
       blt=0x01;
       break;

     case OFF_BLT:
       if(Data_in == '0'){
         nokia_lcd_write_string("equal 0",1);
         nokia_lcd_set_cursor(0, 10);
         nokia_lcd_write_string(Data_in, 1);
         nokia_lcd_render();
       }
       blt=0x00;
       break;

     }
 return state;
}
  int motorPWM(int state){
    switch(state){
      case PWM_INIT:
         k=0;
         state=OFF;
         break;

      case OFF:
         if(blt==0x00){
           state = OFF;
         }
         else{
           state = HIGH;
           k=0;
         }
         break;
      case LOW:
      if(blt == 0x00){
        state = OFF;
      }
      else{
        if(k < L){
          k++;
          state = LOW;
        }
        else{
          state = HIGH;
          k = 0;
        }

      }
      break;
      case HIGH:
      if(blt == 0x00){
        state = OFF;
      }
      else{
        if(k < H){
          k++;
        }
        else{
          k = 0;
          state = LOW;
        }

      }
      break;
    }
    switch (state) {
      case HIGH:
        PORTC = 0xFF;
        break;
      case LOW:
        PORTC = 0x00;
    }
    return state;
  }


int NokiaTick(int state){


 switch(state){
   case NOKIA_INIT:
    state = WAITING;
    break;

   case WAITING:
     if(blt == 0x00){
       state = WAITING;
     }
     if(blt == 0x01){
       state = RETURNING;
       nokia_lcd_clear();
       nokia_lcd_write_string("RETURN",2);
       nokia_lcd_set_cursor(0, 10);
       nokia_lcd_write_string("YOUR ORDER IS READY!", 1);
       nokia_lcd_render();
     }
     break;

   case RETURNING:
    if(blt == 0x01){
      state = RETURNING;
   }
   if(blt == 0x00){
     state = WAITING;
     nokia_lcd_clear();
     nokia_lcd_write_string("Dans Pizzia",1);
     nokia_lcd_set_cursor(0, 10);
     nokia_lcd_write_string("Order #1", 2);
     nokia_lcd_render();
   }
   break;

   }

 return state;
}




// -------- End Tick Functions -------------






int main(void) {
    /* Insert DDR and PORT initializations */
    DDRC = 0xFF; PORTC= 0x00;  // LED LIGHT
    DDRB = 0xFF;  // Bluetooth
    H=2;L=1;

    initUSART();       // Bluetooth USART
    nokia_lcd_init();
    nokia_lcd_clear();
    nokia_lcd_write_string("ON",1);
    nokia_lcd_render();
    unsigned short i; //scheduler for loop

    static task task1,task2,task3;
    task* tasks[] = {&task1,&task2,&task3};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);




    // Task 2 (Blutooth)
    task1.state = BLT_INIT;
    task1.period = 100;
    task1.elapsedTime = task1.period;
    task1.TickFct = &BluetoothTick;

    // Task 2 (Motor)
    task2.state = PWM_INIT;
    task2.period = 100;
    task2.elapsedTime = task2.period;
    task2.TickFct = &motorPWM;

    // Task 3 (SCREEN)
    task3.state = NOKIA_INIT;
    task3.period = 100;
    task3.elapsedTime = task3.period;
    task3.TickFct = &NokiaTick;


    TimerSet(100); // need to set and create var GCD
    TimerOn();
    nokia_lcd_clear();
    nokia_lcd_write_string("ON2",1);
    nokia_lcd_render();
    while (1) {
      for(i=0; i<numTasks;i++){
        if (tasks[i]->elapsedTime == tasks[i]->period){ //task ready to tick
          tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);// Set Next state
          tasks[i]->elapsedTime=0; //Reset the elapsed time
        }
        tasks[i]->elapsedTime += 100;

      }
      while(!TimerFlag);
      TimerFlag = 0;

    }

    return 0;
}
