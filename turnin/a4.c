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
  // #include "usart.h"
  // -- added from provided source files --
  #include "io.h"
  // #include "Serial.h"
  #include <avr/interrupt.h>
  #ifdef _SIMULATE_
  #include "simAVRHeader.h"
  #endif
 // ================ Serial.h =================
 #define DD_MISO    PB6
 #define DD_MOSI    PB5
 #define DD_SCK     PB7
 #define DD_SS      PB4
 #define DDR_SPI    DDRB


 // Note:  1. See the IO Port description for how to define
 // the SPI pin directions. The following code examples show
 // how to initialize the SPI as a Master and how to perform a
 // simple transmission.

 // DDR_SPI in the examples must be replaced
 // by the actual Data Direction Register controlling the SPI pins.
 // DD_MOSI (DDB5)
 // DD_MISO
 // DD_SCK (DDRB)
 // must be replaced by the actual data
 // direction bits for these pins.

 //      E.g. if MOSI is placed on pin PB5,
 //      replace DD_MOSI with DDB5 and DDR_SPI with DDRB.


 // Slave Mode
 // When the SPI is configured as a Slave, the Slave Select (SS) pin
 // is always input. When SS is held low, the SPI is activated, and
 // MISO becomes an output if configured so by the user. All other pins
 // are inputs. When SS is driven high, all pins are inputs, and the
 // SPI is passive, which means that it will not receive incoming data.
 // The SPI logic will be reset once the SS pin is driven high.
 // The SS pin is useful for packet/byte synchronization to keep
 // the slave bit counter synchronous with the master clock generator.
 // When the SS pin is driven high, the SPI slave will immediately reset
 // the send and receive logic, and drop any partially received data in
 // the Shift Register.

 void SPI_SlaveInit(void)
 {
 /* Set MISO output, all others input */
   DDR_SPI = (1<<DD_MISO); //DDRB = (1<<DD6); //DDRB = (1<<PINB4);
 /* Enable SPI */
   SPCR = (1<<SPE)|(1<<SPIE);// SPCR = (1<<SPE);
 }


 // while ((SPSR & (1<<SPIF)));
 char SPI_SlaveReceive(void)
 {
   /* Wait for reception complete */
    while((SPSR & (1<<SPIF)));

   /* Return Data Register */
     return SPDR;
 }
 // void SPI_MasterInit(void) {
 //     /* Set MOSI and SCK output, all others input */
 //     DDRB = (1<<DDB5)|(1<<DDB7);
 //     /* Enable SPI, Master, set clock rate fck/16 */
 //     SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
 // }
 // void SPI_MasterTransmit(char cData) {
 //   /* Start transmission */
 //   SPDR = cData;
 //    while(!(SPSR & (1<<SPIF))); /* Wait for transmission complete */
 //   }
// =======    END Serial.h     ====



  // -------- State Varibles -------------

  enum ReadDipSwitch_States {DIP_READ};
  enum ReadRFID_States{RFID_READ};
  enum display_States {INIT,DISPLAY};


  // -------- End State Varibles -------------

  // -------- Shared Varibles -------------
  unsigned char rfid_reading;
  unsigned char  old_rfid_reading;
  unsigned char d_switch;
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
  // Precondition:
  //  Read A0-A7
  // Postcondition


 int ReadRFID(int state){

   switch(state){
     case RFID_READ:
        // LCD_WriteData('A');
        rfid_reading = SPI_SlaveReceive();
        LCD_WriteData(rfid_reading+ '0');

     break;

   }

   return RFID_READ;

 }

 int displaySMTick(int state){

   switch (state) {
     case INIT:
       state = DISPLAY;
       old_rfid_reading=rfid_reading;

        break;
     case DISPLAY:
       // if (old_rfid_reading != rfid_reading ){
       //   LCD_ClearScreen();
       //   LCD_WriteData('A');
       //   old_rfid_reading = rfid_reading;
       // }
        break;

   }
   return state;
 }
// -------- End Tick Functions -------------






int main(void) {
    /* Insert DDR and PORT initializations */
    DDRC = 0xFF; PORTC= 0x00;  //LED SCREEN
    DDRD = 0xFF; PORTD = 0x00; //LED SCREEN
    /* Set MISO output, all others input */
    DDRB = 0x40; PORTB = 0x00; // RFID READER

    // RFID READER
      // initUSART();
      SPI_SlaveInit();

    // --  RFID READER End  --

    //-- LCD Screen ---
    LCD_init();
    LCD_ClearScreen();
         // LCD_WriteData('B');
    // LCD_WriteData(rfid_reading);
    //-- LCD Screen End ---

    unsigned short i; //scheduler for loop

    static task task1,task2;
    task* tasks[] = {&task1,&task2};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);


    // Task 1 (RFID READER)
    task1.state = RFID_READ;
    task1.period = 300;
    task1.elapsedTime = task1.period;
    task1.TickFct = &ReadRFID;

    // Task 2 (LCD Screen DISPLAY)
    task2.state = INIT;
    task2.period = 300;
    task2.elapsedTime = task2.period;
    task2.TickFct = &displaySMTick;




    TimerSet(100); // need to set and create var
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
