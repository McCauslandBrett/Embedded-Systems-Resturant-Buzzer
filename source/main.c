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
  #include <avr/interrupt.h>
  #ifdef _SIMULATE_
  #include "simAVRHeader.h"
  #endif
 #include "USART_RS232_H_file.h"


// -------- Bluetooth HC-05 MODULE -------------
unsigned char Data_in;
void Bluetooth_config(){

}


  // -------- State Varibles -------------

  enum ReadDipSwitch_States {READ_DIP};
  enum Bluetooth_States {ON_BLT,OFF_BLT};

  // -------- End State Varibles -------------

  // -------- Shared Varibles -------------
   unsigned char temp;
   unsigned char d_switch;
   unsigned char H,L;
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

  //  Read A0-A7
  // Postcondition
  int ReadDipSwitch(int state){
    // switch(state){
    //   case READ_DIP:
    //     d_switch = ~PINA & 0x01;
    //     if(d_switch){PORTC=0x01;}
    //     if(!d_switch){PORTC=0x00;}
    // }
    return READ_DIP;
  }
int BluetoothTick(int state){
    Data_in = USART_RxChar(); /* receive data from Bluetooth device*/

    switch(state){
      case ON_BLT:
        state = (Data_in =='1') ? OFF_BLT: ON_BLT;
        break;

      case OFF_BLT:
        state = (Data_in =='1') ? ON_BLT: OFF_BLT;
        break;

      }
      switch(state){
        case ON_BLT:
          PORTC = 0x03;
          break;

        case OFF_BLT:
          PORTC = 0x02;
          break;

        }
    return state;
}





// -------- End Tick Functions -------------






int main(void) {
    /* Insert DDR and PORT initializations */
    DDRC = 0xFF; PORTC= 0x00;  // LED LIGHT
    DDRA = 0x00;  // Dip Switch
    DDRB = 0xFF;  // Bluetooth

    USART_Init(9600);	;       // Bluetooth USART

    PORTC = 0x01;
    unsigned short i; //scheduler for loop

    static task task1,task2;
    task* tasks[] = {&task1,&task2};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);


    // Task 1 (DipSwitch)
    task1.state = READ_DIP;
    task1.period = 100;
    task1.elapsedTime = task1.period;
    task1.TickFct = &ReadDipSwitch;

    // Task 2 (Blutooth)
    task2.state = OFF_BLT;
    task2.period = 100;
    task2.elapsedTime = task2.period;
    task2.TickFct = &BluetoothTick;

    // Task 2 (Motor)
    // task3.state = READ_BLT;
    // task3.period = 100;
    // task3.elapsedTime = task3.period;
    // task3.TickFct = &PWMTick;


    TimerSet(100); // need to set and create var GCD
    TimerOn();
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
    while(1)
	{
		Data_in = USART_RxChar();						/* receive data from Bluetooth device*/
		if(Data_in =='1')
		{
			PORTC = 0x03;							/* Turn ON LED */
			// USART_SendString("LED_ON");					/* send status of LED i.e. LED ON */

		}
		else if(Data_in =='2')
		{
			PORTC = 0x02;							/* Turn OFF LED */
			// USART_SendString("LED_OFF"); 				/* send status of LED i.e. LED OFF */
		}
		// else
			// USART_SendString("Select proper option");	/* send message for selecting proper option */

	// return 0;
  }
    return 0;
}
