# Four Face Firmware Notes

## Slave 

- Implemented with a MSP430
    * slave address = 1
    * Reads status of 9 cap sense buttons
        - two bytes to to transfer all this data
    * Upon request sends master status of all 9 buttons
    * Will receive 4 bytes from the master
        - floor(32 bits / 9 faces) -> 3 bits per face
        - Colors for each face are determined by one of the possible 3  it combos 
            - 2^3 -> 8 combinations, 8 colors possible per face.
    