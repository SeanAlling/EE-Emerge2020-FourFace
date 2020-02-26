#include <msp430.h>
#include "ws2812.h"

#include<stdint.h>

void gradualFill(uint32_t n, ColorMap);
void write_ws2811_hs_16(const uint8_t *data, uint16_t length, uint8_t pinmask, uint16_t portmask);


void toggleLED()
{

    static     uint8_t state = 0;
    if(state==0)
    {
        state = 1;
        P1OUT &= ~BIT0;   
    }
    else
    {
        state = 0;
        P1OUT |= BIT0;		// else LED ON
    }


}

int main(void) 
{
    WDTCTL = WDTPW + WDTHOLD;  // Stop WDT
    if (CALBC1_16MHZ==0xFF)    // If calibration constant erased
    {
        while(1);              // do not load, trap CPU!!
    }

    // configure clock to 16 MHz
    // clk cycle = 62.5ns
    BCSCTL1 = CALBC1_16MHZ;    // DCO = 16 MHz
    DCOCTL = CALDCO_16MHZ;


    P1DIR |= BIT0;				// P1.0 (Red) -> Output


    // initialize LED strip
    toggleLED();

    initStrip();  // ***** HAVE YOU SET YOUR NUM_LEDS DEFINE IN WS2812.H? ******
    // set strip color red
    fillStrip(COLOR_GREEN);

    // show the strip
    showStrip();
    toggleLED();
    // gradually fill for ever and ever

    gradualFill(NUM_LEDS, COLOR_RED);  // green

    toggleLED();
    gradualFill(NUM_LEDS, COLOR_BLUE);  // blue
    toggleLED();
    gradualFill(NUM_LEDS, COLOR_GREEN);  // magenta
    toggleLED();
//gradualFill(NUM_LEDS,COLOR_OFF);  // yellow
    toggleLED();


    while (1) 
    {







    }
}

void gradualFill(uint32_t n, ColorMap color)
{
    
    for (uint32_t i = 0; i < n; i++)
    {        // n is number of LEDs
        setLEDColor(i, color);
        __delay_cycles(1000000);       // lazy delay
    }
    showStrip();
}
