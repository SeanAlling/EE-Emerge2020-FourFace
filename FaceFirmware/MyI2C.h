#ifndef MY_I2C_H_
#define MY_I2C_H_

/*
    Set up I2C. 

    Input: address -> if 0 then set up as master. 
                    if non 0 then slave
    Output: None
    Side Effect: Modifies device registers.
*/
void I2C_Init(const uint8_t address);

#endif //MY_I2C_H_