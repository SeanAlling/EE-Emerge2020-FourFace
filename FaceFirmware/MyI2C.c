#include <stdint.h>

#include "msp430g2553.h"
#include "MyI2C.h"






/*

    Following came from Simply Embedded
    http://www.simplyembedded.org/tutorials/msp430-i2c-basics/

*/
void I2C_Init(const uint8_t address)
{
    P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
    P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0

    /* Ensure USCI_B0 is in reset before configuring */
    UCB0CTL1 = UCSWRST;
 
    /*
        If address is zero then set up tis device as a I2C master.
    */
    if(address == 0)
    {
        /* Set USCI_B0 to master mode I2C mode */
        UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;
        /** 
         * Configure the baud rate registers for 100kHz when sourcing from SMCLK
         * where SMCLK = 1MHz
         */
        // TODO: This needs to be adjusted for the correct master clock.
        UCB0BR0 = 10; 
        UCB0BR1 = 0;
    }
    /*
        If address is non zero, then set up I2C as a slave.
    */
    else
    {
        /* Set USCI_B0 to slave mode I2C mode */
        UCB0CTL0 = UCMODE_3 | UCSYNC;
        UCB0I2COA = address;
    }

    UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation

    
    
    // address is 0 for master mode
    if(address == 0)
    {

    }
    // address is non zero so must be a slave
    else
    {
        IE2 |= UCB0TXIE + UCB0RXIE;               // Enable TX interrupt
        UCB0I2CIE |= UCSTTIE;                     // Enable STT interrupt
    }
}