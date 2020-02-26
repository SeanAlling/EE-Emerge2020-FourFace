# Four Face Firmware Notes

## Slave 

- Implemented with a MSP430
    * Slave communicates to the master using UART. 
        - UART is used since to meet timing requirements for the neopixels, the SPI peripheral is used. On this MSP430 the hardware I2C and the SPI share the same pins.
        - Master sends two bytes of data to the slave
            * Byte 0: 0-8 which corresponds to the square led quare. 
            * Byte 1: 0-3 which corresponds to the color that the square should be.
        - Slave sends the master to bytes
            * Byte 0: Corresponds to the button status for squares 0 - 8. 
            * Byte 1: Corresponds to the button status for squares 9. The 7 MSB bits are don't cares. 