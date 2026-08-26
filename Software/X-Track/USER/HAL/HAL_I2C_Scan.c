#include "HAL.h"

int I2C_Scan(void)
{
    uint8_t error, address;
    int nDevices;

    if(!Wire_Begin())
    {
        Serial_Println("I2C: init failed");
        return -1;
    }

    Serial_Println("I2C: device scanning...");

    nDevices = 0;
    for (address = 1; address < 127; address++ )
    {
        Wire_BeginTransmission(address);
        error = (uint8_t)Wire_EndTransmission();

        if (error == 0)
        {
            Serial_Print("I2C: device found at address 0x");
            if (address < 16)
                Serial_Print("0");
            Serial_PrintHex(address);
            Serial_Println(" !");

            nDevices++;
        }
        else if (error == 4)
        {
            Serial_Print("I2C: unknow error at address 0x");
            if (address < 16)
                Serial_Print("0");
            Serial_PrintHex(address);
            Serial_Println("");
        }
    }

    Serial_Printf("I2C: %d devices was found\r\n", nDevices);
    return nDevices;
}
