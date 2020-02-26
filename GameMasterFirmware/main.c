//Includes for game
#include <stdbool.h>
#include <stdio.h> 
#include <stdlib.h> 
// #include "rand.h"
//Include for sending data to slave
#include "msp432.h"
#include "driverlib.h"

//Pins being used:
// 1.0, 1.2, 1.4, 1.5, 2.1 2.2, 2.4, 1.6, 2.7 (These are defined as BITS[0-F] in msp430g2553.h)
#define DELAY_50ms  (400000)      // 8MHz MCLK Cycles for 50ms Delay
#define DELAY_100ms (800000)
#define NUM_LEDS (207)
#define LEDS_PER_SQUARE (NUM_LEDS/9)
uint32_t prevButtonStates;
//////////////SEND DATA TO SLAVE/////////////////////////////
const eUSCI_UART_Config uartConfig =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        78,                                     // BRDIV = 78
        2,                                       // UCxBRF = 2
        0,                                       // UCxBRS = 0
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_LSB_FIRST,                  // LSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
};
//////////////////////////////////////////////////////////////

//////////////MASTER.C GAME CODE//////////////////////////////
typedef struct 
{
    int score;
    bool  eliminated;
} Player;

typedef struct 
{
    int totalRoundCount;
    int crntRound;
    int initialSquareCount; // How many squares emitted in the pattern
    //Difficulty
} Game;


//Globals
static Game game;
static Player* player1 = NULL;
static Player* player2 = NULL;
static Player* player3 = NULL;
static Player* player4 = NULL;


//Enums
typedef enum  ledStripPins
{
    SQUARES_A,
    SQUARES_B,
    SQUARES_C,
    SQUARES_D,
    SQUARES_E,
    SQUARES_F,
    SQUARES_G,
    SQUARES_H,
    SQUARES_I,
    NUM_SQUARES
} squareNames;


//Functions
//MASTER
void startGame(int totalRoundCount, int initialSquareCount); //Round time in ms
void initializePlayer(Player *player);
int* generatePattern(const int sequenceLength); //Returns new pattern. Should only be called from newRound();
int* newRound(const int sequenceLength); 
void displayPattern(int* roundPattern, int sequenceLength);
void sendSquareColor(uint16_t squareAndColor);
void fillStrip(uint16_t color);
void clearStrip();
//void sendDataToSlaves();


////////////////////////////////////////////////////////////////////


 
int main(void)
{
    //Seed for random numbers
    // srand(trueRand()); 
    
    // Halting WDT
    WDT_A_holdTimer();
    

    /* Selecting P1.2 and P1.3 in UART mode */
        MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
                GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

        /* Setting DCO to 12MHz */
        CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

        /* Configuring UART Module */
        MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);

        /* Enable UART module */
        MAP_UART_enableModule(EUSCI_A0_BASE);

        /* Enabling interrupts */
        MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
        MAP_Interrupt_enableInterrupt(INT_EUSCIA0);
        MAP_Interrupt_enableSleepOnIsrExit();
        MAP_Interrupt_enableMaster();

    
    // initialize game and players
    uint16_t buttonStates = 0x0;
    startGame(5, 2);
    player1 = malloc(sizeof* player1);
    initializePlayer(player1);
    int sizePatternArray;
    int* pattern;
    uint16_t pressedPlate = 0x0;
    int i = 0;
    
    
    //Wait until the player has pressed the center square and signifies they are ready
    //Set center square to blue until pressed
    uint16_t readyUpSquare = 0x00;
    readyUpSquare |= (0x4 << 0);
    readyUpSquare |= (0x2 << 1);
    //setSquareColor(readyUpSquare);
    //This needs to be set to check if the returned square is square
    clearStrip();
    
    
    for (i = 0; i < game.totalRoundCount && !player1->eliminated; i++)
    {
        buttonStates = 0x0;
        prevButtonStates = 0x0;
        sizePatternArray = game.initialSquareCount + game.crntRound;
        pattern = newRound(sizePatternArray);
        
        int j = 0;
        for (; j < sizePatternArray && !player1->eliminated; j++) //Input from user
        {
            while(1)
            {
                pressedPlate = 0x0; //This should be set to rectrieve pressed square from slave
                if (pressedPlate != 0)
                    break;
                __delay_cycles(DELAY_100ms);  // Delay 50ms (14Hz button scan rate)
            }
            if (pressedPlate != pattern[j])
                player1->eliminated = true;
            else
                player1->score++;
        }
        for (i = 0; i < 1; i++) //1 second timer
            __delay_cycles(10 * DELAY_100ms);
        clearStrip();
    }
        
    if (!player1->eliminated)  //Player wins
        fillStrip(3); //Fill the strip with green.
    else  //Player loses
        fillStrip(1); //Fill the strip with red
}


void startGame(int totalRoundCount, int initialSquareCount)
{
    game.totalRoundCount = totalRoundCount;
    game.initialSquareCount = initialSquareCount;
    game.crntRound = 0;
}


void initializePlayer(Player *player)
{
    player->score = 0;
    player->eliminated = false;
}


int* generatePattern(const int sequenceLength)
{
    int* newPattern;
    //Allocate a block of memory the length of sequenceLength with each block large enough for an int
    newPattern = calloc(sequenceLength, sizeof(int));
    int i;
    for ( i = 0; i < sequenceLength; i++)
        *(newPattern + i) = (rand() % (SQUARES_I - SQUARES_A + 1)) + SQUARES_A;
    return newPattern;
}


int* newRound(const int sequenceLength)
{
    int* pattern = generatePattern(sequenceLength);
    game.crntRound++;
    displayPattern(pattern, sequenceLength);
    return pattern;
}
 
 
void displayPattern(int* roundPattern, int sequenceLength)
{
    uint16_t squareAndColor = 0x00;
    int i = 0;
    for (; i < sequenceLength; i++)
    {
        squareAndColor = 0x00;
        squareAndColor |= ((roundPattern[i]) << 0);
        squareAndColor |= (0x3 << 1);
        sendSquareColor(squareAndColor);
        __delay_cycles(10 * DELAY_100ms);
    }
    for (i = 0; i < 2; i++) //2 second timer
        __delay_cycles(10 * DELAY_100ms);
    fillStrip(2);
    for (i = 0; i < 1; i++) //1 second timer
        __delay_cycles(10 * DELAY_100ms);
    clearStrip();
}


//Received the hex number squareAndColor.
//Its first bit is the square to set.
//Second bit is the color.
//This function sends the first bit to the slave then right shifts by 1
//then sends the second bit/
void sendSquareColor(uint16_t squareAndColor) 
{
    int i = 0;
    for (; i < 2; i++)
    {
        UART_transmitData(EUSCI_A0_BASE, squareAndColor & 0x1);
        squareAndColor >>= 1;
    }
}


void fillStrip(uint16_t color)
{
    uint16_t squareAndColor = 0x00;
    squareAndColor |= ((color) << 1);
    int i = 0;
    for (; i < 9; i++)
    {
        squareAndColor &= (0x0 << 0);
        squareAndColor |= ((i) << 0);
        sendSquareColor(squareAndColor);
    }
}


void clearStrip()
{
    uint16_t squareAndColor = 0x00;
    squareAndColor |= (0x0 << 1);
    int i = 0;
    for (; i < 9; i++)
    {
        squareAndColor &= (0x0 << 0);
        squareAndColor |= ((i) << 0);
        sendSquareColor(squareAndColor);
    }
}

/* EUSCI A0 UART ISR - Echoes data back to PC host */
void EUSCIA0_IRQHandler(void)
{
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A0_BASE);

    MAP_UART_clearInterruptFlag(EUSCI_A0_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        MAP_UART_transmitData(EUSCI_A0_BASE, MAP_UART_receiveData(EUSCI_A0_BASE));
    }

}
