//******************************************************************************
//  MSP430G2xx3 Demo - USCI_B0 I2C Slave TX multiple bytes to MSP430 Master
//
//  Description: This demo connects two MSP430's via the I2C bus. The slave
//  transmits to the master. This is the slave code. The interrupt driven
//  data transmission is demonstrated using the USCI_B0 TX interrupt.
//  ACLK = n/a, MCLK = SMCLK = default DCO = ~1.2MHz
//                               
//               MSP430G2xx3      10k  10k     MSP430G2xx3
//                   slave         |    |        master
//             -----------------   |    |  -----------------
//           -|XIN  P3.1/UCB0SDA|<-|---+->|P3.1/UCB0SDA  XIN|-
//            |                 |  |      |                 |
//           -|XOUT             |  |      |             XOUT|-
//            |     P3.2/UCB0SCL|<-+----->|P3.2/UCB0SCL     |
//            |                 |         |                 |
//
//  D. Dang
//  Texas Instruments Inc.
//  February 2011
//  Built with CCS Version 4.2.0 and IAR Embedded Workbench Version: 5.10
//******************************************************************************

#include <stdint.h>
#include "msp430g2553.h"

#include "MyI2C.h"

uint32_t ledStatesOld = 0;
uint32_t ledStatesNew = 0;
uint8_t count = 0;

/*
    Returns a bit mask of buttons that are pressed.

*/
uint16_t ButtonSelected(void)
{
    // Assume no button is pressed.
    uint8_t result = 0;
    /*
        Start checking all the buttons that are pressed. First button pressed will take priority. 
    */
    if ((P1IN & BIT3) != BIT3) 
    {
        result |= (1<<0);
    }
    
    if ((P1IN & BIT4) != BIT4) 
    {
        result |= (1<<0);
    }
    
    if ((P1IN & BIT5) != BIT5) 
    {
        result |= (1<<1)
    }
    
    if ((P2IN & BIT3) != BIT3) 
    {
        result |= (1<<2)
    }
    
    if ((P2IN & BIT4) != BIT4) 
    {
        result |= (1<<3)
    }
    
    if ((P2IN & BIT5) != BIT5) 
    {
        result |= (1<<4)
    }
    
    if ((P1IN & BIT1) != BIT1) 
    {
        result |= (1<<5)
    }
    
    if ((P1IN & BIT2) != BIT2) 
    {
        result |= (1<<6)
    }
    
    if ((P2IN & BIT0) != BIT0) 
    {
        result |= (1<<7)
    }
    
    return result;
}



/*
    These strings are transmitted depending on cmd the slave has 
    received from he master. 
*/


volatile uint8_t dataToSend = 0;
volatile uint8_t cmd = 0;
volatile uint8_t result = 0;
volatile uint8_t Turnon = 0;

int main(void)
{
    
    WDTCTL = WDTPW + WDTHOLD;                 // Stop Watchdog Timer
    
    // GPIO SETUP    
    P1DIR |= 0x01; // set P1.0 as Output
    P2DIR |= BIT6;
    P1DIR &= ~(BIT3 | BIT4 | BIT5 | BIT1 | BIT2); // set to input
    P1REN |= (BIT3 | BIT4 | BIT5 | BIT1 | BIT2); // Use an internal resistor
    P1OUT |= (BIT3 | BIT4 | BIT5 | BIT1 | BIT2); // The internal resistor is pullup
    P2DIR &= ~(BIT3 | BIT4 | BIT5 | BIT0); // set to input
    P2REN |= (BIT3 | BIT4 | BIT5 | BIT0); // Use an internal resistor
    P2OUT |= (BIT3 | BIT4 | BIT5 | BIT0); // The internal resistor is pullup

    // Slave address is 0x48.
    I2C_Init(0x48);
  
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL  = CALDCO_16MHZ;
  
  /*
      I2C used interrrupts so need to have the system  must enable intterupts
      in order for froper oeration of I2C.
  */ 
  __bis_SR_register(GIE);
  while (1)
  {
    dataToSend = ButtonSelected();
  }
}

//------------------------------------------------------------------------------
// The USCI_B0 data ISR is used to move data from MSP430 memory to the
// I2C master. PTxData points to the next byte to be transmitted, and TXByteCtr
// keeps track of the number of bytes transmitted.
//------------------------------------------------------------------------------
__attribute__((interrupt(USCIAB0TX_VECTOR))) 
void USCIAB0TX_ISR(void)

    // Tx interrupt
    if (IFG2 & UCB0TXIFG)
    {
        // Command is zero and length of string 1 has not been sent, so send 
        // the next character of string.
        UCB0TXBUF = dataToSend;
    }
    /*
        Else must be Rx interrupt.
        Single byte is received which is used to set the command. The generated 
        command is used to determine what message will be sent from the slave. 
    */
    else
    {
        // If count has reached 4 that means the device has received  byts which means the 
        // 32 bit LED control word has been received. Transfer the LED CMD word to the led state old 
        // // value which is the one that the main software used.
        if(count == 4)
        {
            ledStatesld = ledStatesNew;
        }
        else if(count < 4)
        {
            ledStatesNew = (ledStatesNew<<8) | UCB0RXBUF;
            count++;
        }
        else
        {
            // Do nothing. Must wait for master to send a start cmd. 
        }
        
    }
}


/*
    This interrupt will trigger every time a start condition is sensed on the I2C bus.
    Resets byte transfer count to 0. 
*/
__attribute__((interrupt(USCIAB0RX_VECTOR))) 
void USCIAB0RX_ISR(void)
{
    UCB0STAT &= ~UCSTTIFG;                    // Clear start condition int flag
    ledStatesNew = 0;
    count = 0;
}