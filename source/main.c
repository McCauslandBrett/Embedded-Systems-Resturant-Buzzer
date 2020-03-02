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
unsigned char Data_in,blt;
void Bluetooth_config(){

}
  // -------- State Varibles -------------

  enum ReadDipSwitch_States {READ_DIP};
  enum Bluetooth_States {ON_BLT,OFF_BLT};
  enum PWM_States {HIGH,LOW};

  // -------- End State Varibles -------------

  // -------- Shared Varibles -------------
   unsigned char temp;
   unsigned char d_switch;
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

  //  Read A0-A1
  // Postcondition
  int ReadDipSwitch(int state){
    static unsigned char AA0,AA1;
    AA0 = ~PINA & 0x01;
    AA1 = ~PINA & 0x02;
    switch(state){
      case READ_DIP:
       if (AA0 && AA1) {
         H = 0x0A;
         L = 0x00;
         break;
       }
       if (!AA0) {
         H = 0x00;
         L = 0x0A;
         break;
       }
       if (AA0 && !AA1) {
         H = 0x05;
         L = 0x05;
         break;
       }
    }
    return READ_DIP;
  }
  //period 1000ms
  int MotorPWM(int state){

    switch(state){
      case HIGH:
        if(k < H && blt){
          state = HIGH;
          k++;
          break;
        }
        else{
          k=0;
          state = LOW;
          break;
      }
      case LOW:
        if(k < L){
          state = LOW;
          k++;
          break;
        }
        if (k > L && !blt) {
          k=0;
          state = LOW;
          break;
        }
        if(k > L && blt)
          k=0;
          state = HIGH;
          break;
        }


      //PINC0 :=  Buzzer
      //PINC1 :=  Operational LIGHT
      switch(state){ //actions
        case HIGH:
           PORTC = 0x03;
        break;
        case LOW:
           PORTC = 0x02;
          break;
        }
    return state;
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
          // LCD
          blt=0x00;
          break;

        case OFF_BLT:
          // LCD
            blt=0x01;
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
    H=0;L=10;

    USART_Init(9600);	;       // Bluetooth USART

    PORTC = 0x02;
    unsigned short i; //scheduler for loop

    static task task1,task2,task3;
    task* tasks[] = {&task1,&task2,&task3};
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
    task3.state = LOW;
    task3.period = 100;
    task3.elapsedTime = task3.period;
    task3.TickFct = &MotorPWM;


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

    return 0;
}
