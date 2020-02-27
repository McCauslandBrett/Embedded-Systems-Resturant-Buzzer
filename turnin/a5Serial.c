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
  #define CARD_FOUND		1
  #define CARD_NOT_FOUND	2
  #define ERROR			3

  #define MAX_LEN			16

  //Card types
  #define Mifare_UltraLight 	0x4400
  #define Mifare_One_S50		0x0400
  #define Mifare_One_S70		0x0200
  #define Mifare_Pro_X		0x0800
  #define Mifare_DESFire		0x4403

  // Mifare_One card command word
  # define PICC_REQIDL          0x26               // find the antenna area does not enter hibernation
  # define PICC_REQALL          0x52               // find all the cards antenna area
  # define PICC_ANTICOLL        0x93               // anti-collision
  # define PICC_SElECTTAG       0x93               // election card
  # define PICC_AUTHENT1A       0x60               // authentication key A
  # define PICC_AUTHENT1B       0x61               // authentication key B
  # define PICC_READ            0x30               // Read Block
  # define PICC_WRITE           0xA0               // write block
  # define PICC_DECREMENT       0xC0               // debit
  # define PICC_INCREMENT       0xC1               // recharge
  # define PICC_RESTORE         0xC2               // transfer block data to the buffer
  # define PICC_TRANSFER        0xB0               // save the data in the buffer
  # define PICC_HALT            0x50               // Sleep

  #define Page0_Reserved_1 	0x00
  #define CommandReg			0x01
  #define ComIEnReg			0x02
  #define DivIEnReg			0x03
  #define ComIrqReg			0x04
  #define DivIrqReg			0x05
  #define ErrorReg			0x06
  #define Status1Reg			0x07
  #define Status2Reg			0x08
  #define FIFODataReg			0x09
  #define FIFOLevelReg		0x0A
  #define WaterLevelReg		0x0B
  #define ControlReg			0x0C
  #define BitFramingReg		0x0D
  #define CollReg				0x0E
  #define Page0_Reserved_2	0x0F

  //Page 1 ==> Command
  #define Page1_Reserved_1	0x10
  #define ModeReg				0x11
  #define TxModeReg			0x12
  #define RxModeReg			0x13
  #define TxControlReg		0x14
  #define TxASKReg			0x15
  #define TxSelReg			0x16
  #define RxSelReg			0x17
  #define RxThresholdReg		0x18
  #define	DemodReg			0x19
  #define Page1_Reserved_2	0x1A
  #define Page1_Reserved_3	0x1B
  #define MfTxReg				0x1C
  #define MfRxReg				0x1D
  #define Page1_Reserved_4	0x1E
  #define SerialSpeedReg		0x1F

  //Page 2 ==> CFG
  #define Page2_Reserved_1	0x20
  #define CRCResultReg_1		0x21
  #define CRCResultReg_2		0x22
  #define Page2_Reserved_2	0x23
  #define ModWidthReg			0x24
  #define Page2_Reserved_3	0x25
  #define RFCfgReg			0x26
  #define GsNReg				0x27
  #define CWGsPReg			0x28
  #define ModGsPReg			0x29
  #define TModeReg			0x2A
  #define TPrescalerReg		0x2B
  #define TReloadReg_1		0x2C
  #define TReloadReg_2		0x2D
  #define TCounterValReg_1	0x2E
  #define TCounterValReg_2 	0x2F

  //Page 3 ==> TestRegister
  #define Page3_Reserved_1 	0x30
  #define TestSel1Reg			0x31
  #define TestSel2Reg			0x32
  #define TestPinEnReg		0x33
  #define TestPinValueReg		0x34
  #define TestBusReg			0x35
  #define AutoTestReg			0x36
  #define VersionReg			0x37
  #define AnalogTestReg		0x38
  #define TestDAC1Reg			0x39
  #define TestDAC2Reg			0x3A
  #define TestADCReg			0x3B
  #define Page3_Reserved_2 	0x3C
  #define Page3_Reserved_3	0x3D
  #define Page3_Reserved_4	0x3E
  #define Page3_Reserved_5	0x3F


  #define Idle_CMD 				0x00
#define Mem_CMD					0x01
#define GenerateRandomId_CMD	0x02
#define CalcCRC_CMD				0x03
#define Transmit_CMD			0x04
#define NoCmdChange_CMD			0x07
#define Receive_CMD				0x08
#define Transceive_CMD			0x0C
#define Reserved_CMD			0x0D
#define MFAuthent_CMD			0x0E
#define SoftReset_CMD			0x0F
 // ================ Serial.h =================
 #define DD_MISO    PB6
 #define DD_MOSI    PB5
 #define DD_SCK     PB7
 #define DD_SS      PB4
 #define DDR_SPI    DDRB


 #define SPI_PORT	PORTB
 #define SPI_PIN		PINB


 #define SPI_SS		  PB4
 #define SPI_SCK		PB7
 #define ENABLE_CHIP() (SPI_PORT &= (~(1<<SPI_SS)))
 #define DISABLE_CHIP() (SPI_PORT |= (1<<SPI_SS))

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
  ENABLE_CHIP();
   SPCR = (1<<SPE)|(1<<SPIE);// SPCR = (1<<SPE);
 }
 void spi_init()
 {
 	DDR_SPI = (1<<DD_MISO);
 	SPCR = (1<<SPE);
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

  uint8_t spi_transmit(uint8_t data)
{
	while(!(SPSR & (1<<SPIF)));
	return SPDR;
}
  void mfrc522_init();
  void mfrc522_write(unsigned char reg, unsigned char data);
  unsigned char mfrc522_read(unsigned char reg);
  void mfrc522_reset();
  unsigned char mfrc522_to_card(unsigned char cmd, unsigned char *send_data, unsigned char send_data_len, unsigned char *back_data, uint32_t *back_data_len);
  unsigned char	mfrc522_request(unsigned char req_mode, unsigned char * tag_type);


 int ReadRFID(int state){

   switch(state){
     case RFID_READ:
        // LCD_WriteData('A');

      rfid_reading = mfrc522_read(VersionReg);
        LCD_WriteData(rfid_reading + '0');

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
    DDRB = 0x40; PORTB = 0x40; // RFID READER

    // RFID READER
      // initUSART();
      // SPI_SlaveInit();
       spi_init();
       mfrc522_init();
    // --  RFID READER End  --

    //-- LCD Screen ---
    LCD_init();
    LCD_ClearScreen();
         // LCD_WriteData('B');
    LCD_DisplayString(1,"Hello");
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
void mfrc522_write(unsigned char reg, unsigned char data)
{
  ENABLE_CHIP();
  delay_ms(2000);
  spi_transmit((reg<<1)&0x7E);
  spi_transmit(data);
  DISABLE_CHIP();
}
void mfrc522_init()
{
  unsigned char byte;
  mfrc522_reset();

  mfrc522_write(TModeReg, 0x8D);
    mfrc522_write(TPrescalerReg, 0x3E);
    mfrc522_write(TReloadReg_1, 30);
    mfrc522_write(TReloadReg_2, 0);
  mfrc522_write(TxASKReg, 0x40);
  mfrc522_write(ModeReg, 0x3D);

  byte = mfrc522_read(TxControlReg);
  if(!(byte&0x03))
  {
    mfrc522_write(TxControlReg,byte|0x03);
  }
}
unsigned char mfrc522_read(unsigned char reg)
{
  unsigned char data;
  ENABLE_CHIP();
  spi_transmit(((reg<<1)&0x7E)|0x80);
  data = spi_transmit(0x00);
  DISABLE_CHIP();
  return data;
}
void mfrc522_reset()
{
  mfrc522_write(CommandReg,SoftReset_CMD);
}
unsigned char mfrc522_to_card(unsigned char cmd, unsigned char *send_data, unsigned char send_data_len, unsigned char *back_data, uint32_t *back_data_len)
{
  unsigned char status = ERROR;
    unsigned char irqEn = 0x00;
    unsigned char waitIRq = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned char	tmp;
    uint32_t i;

    switch (cmd)
    {
        case MFAuthent_CMD:		//Certification cards close
    {
      irqEn = 0x12;
      waitIRq = 0x10;
      break;
    }
    case Transceive_CMD:	//Transmit FIFO data
    {
      irqEn = 0x77;
      waitIRq = 0x30;
      break;
    }
    default:
      break;
    }

    //mfrc522_write(ComIEnReg, irqEn|0x80);	//Interrupt request
    n=mfrc522_read(ComIrqReg);
    mfrc522_write(ComIrqReg,n&(~0x80));//clear all interrupt bits
    n=mfrc522_read(FIFOLevelReg);
    mfrc522_write(FIFOLevelReg,n|0x80);//flush FIFO data

  mfrc522_write(CommandReg, Idle_CMD);	//NO action; Cancel the current cmd???

  //Writing data to the FIFO
    for (i=0; i<send_data_len; i++)
    {
    mfrc522_write(FIFODataReg, send_data[i]);
  }

  //Execute the cmd
  mfrc522_write(CommandReg, cmd);
    if (cmd == Transceive_CMD)
    {
    n=mfrc522_read(BitFramingReg);
    mfrc522_write(BitFramingReg,n|0x80);
  }

  //Waiting to receive data to complete
  i = 2000;	//i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms???
    do
    {
    //CommIrqReg[7..0]
    //Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
        n = mfrc522_read(ComIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitIRq));

  tmp=mfrc522_read(BitFramingReg);
  mfrc522_write(BitFramingReg,tmp&(~0x80));

    if (i != 0)
    {
        if(!(mfrc522_read(ErrorReg) & 0x1B))	//BufferOvfl Collerr CRCErr ProtecolErr
        {
            status = CARD_FOUND;
            if (n & irqEn & 0x01)
            {
        status = CARD_NOT_FOUND;			//??
      }

            if (cmd == Transceive_CMD)
            {
                n = mfrc522_read(FIFOLevelReg);
                lastBits = mfrc522_read(ControlReg) & 0x07;
                if (lastBits)
                {
          *back_data_len = (n-1)*8 + lastBits;
        }
                else
                {
          *back_data_len = n*8;
        }

                if (n == 0)
                {
          n = 1;
        }
                if (n > MAX_LEN)
                {
          n = MAX_LEN;
        }

        //Reading the received data in FIFO
                for (i=0; i<n; i++)
                {
          back_data[i] = mfrc522_read(FIFODataReg);
        }
            }
        }
        else
        {
      status = ERROR;
    }

    }

    return status;
}
unsigned char	mfrc522_request(unsigned char req_mode, unsigned char * tag_type)
{
  unsigned char  status;
  uint32_t backBits;//The received data bits

  mfrc522_write(BitFramingReg, 0x07);//TxLastBists = BitFramingReg[2..0]	???

  tag_type[0] = req_mode;
  status = mfrc522_to_card(Transceive_CMD, tag_type, 1, tag_type, &backBits);

  if ((status != CARD_FOUND) || (backBits != 0x10))
  {
    status = ERROR;
  }

  return status;
}
