#ifndef SERIAL_H
#define SERIAL_H
// try this 1
#include  <avr/io.h>
#include <avr/interrupt.h>
#include "simAVRHeader.h"
//


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

#endif //
