
/**
 * saab-canbus
 * Reads from and writes to the Instrumentation Bus (I-Bus) on a Saab 9-3 (tested on my 2005 with ICM3)
 */

#include <SPI.h>
#include "esp8266_pinout.h"
#include "saab_canbus.h"
#include "mcp_can.h"

#define array_size(_Array) (sizeof(_Array) / sizeof(_Array[0]))

//D2 is the pin where the MCP2515's CS lead is attached to
MCP_CAN CAN(D2);

void setup()
{
    Serial.begin(9600);

    delay(1000);

    Serial.println("Initializing...");
    
    initializeCanBus();

    initializeGpio();

    Serial.println("Setup Complete!");
}

/**
 * Initializing of the MCP2515 module
 */

void initializeCanBus()
{
    while(CAN.begin(MCP_ANY, CAN_33K3BPS, MCP_8MHZ) != CAN_OK)
    {
        Serial.println("MCP init failed, retrying...");
        delay(1000);
    }

    // set mode to normal
    CAN.setMode(MCP_NORMAL);

    Serial.println("MCP initialized.");
}

/**
 * Initializing of GPIO
 */
void initializeGpio()
{
    for(int i = 0; i < array_size(busMappings); ++i)
    {
        sBusMapping & bm = busMappings[i];

        /* setup the GPIO  */
        if(bm.stateFlags & EBS_GPIO)
        {
            pinMode(bm.gpioSync, OUTPUT);
            digitalWrite(bm.gpioSync, LOW);
            Serial.println("BusMapping '" + bm.Name + "': sync on GPIO: " + String(int(bm.gpioSync)));
        }
    }
}

/**
 * main loop
 */
void loop()
{
    handleRecv();
    handleSerial();
    
    updatePing();
}

/**
 * Handle an incomming canbus message
 */
void handleRecv()
{    
    static unsigned char len = 0;
    static unsigned char buf[8];
    static unsigned long canId = 0;
    static byte ext = 0;

    while (CAN.checkReceive() == CAN_MSGAVAIL)
    {        
        CAN.readMsgBuf(&canId, &ext, &len, buf);

        // call the appropriate message handler
        switch (canId)
        {
        case 0x290:
        case 0x310:
        case 0x460:
        case 0x370:
            parseBusCommand(canId, buf, len);
            break;
        default:
            //Serial.println("Msg:" + String(canId));
            break;        
        }
    }
}


/**
 * Handle an incomming serial message
 */

void handleSerial()
{
    if (Serial.available() > 0)
    {
        String msg = Serial.readStringUntil('\n');

        if(msg == "initialize")
            headUnitInitialize();
    }
}

/**
 * Handle head unit initialize
 */
 
void headUnitInitialize()
{    
    Serial.println("Head Unit: Initialize");

    for(int i = 0; i < array_size(busMappings); ++i)
    {
        sBusMapping & bm = busMappings[i];

        /* switch to aux automatically if we haven't touched the button already */
        if(bm.Name == "btn_sw_Src" && bm.counter == 0)
        {
            bm.counter ++;
            sendSourceChangeCommand();
        }
    }
}

/**
 * Update ping messages
 * This is used to debug serial connection to raspberry pi 
 * to make sure the raspberry is receiving connections
 */

void updatePing()
{
    static unsigned i = 0;
    static unsigned long tlast = 0;
    unsigned long tnow = millis();
    if(tnow > tlast + 10000)
    {
        tlast = tnow;
        i++;
        Serial.println("ping" + String(i));
    }
}


/*
 *  Bus
 */

void parseBusCommand(unsigned long canId, unsigned char * msg, unsigned char size)
{      
    for(int i = 0; i < array_size(busMappings); ++i)
    {
        sBusMapping & bm = busMappings[i];
        if(bm.canId != canId)
            continue;

        bool bSet = msg[bm.byteIdx] == bm.byteValue;

        if(bm.stateFlags & EBS_Change)
        {
            if(bSet != bm.state)
            {
                bm.state = bSet;
                Serial.println(bm.Name);
            }
        }
        else if(bm.stateFlags & EBS_Toggle)
        {
            if(bSet != bm.state)
            {
                bm.state = bSet;
                Serial.println(bm.Name + (bSet ? "_on" : "_off"));

                if(bm.stateFlags & EBS_GPIO)
                    digitalWrite(bm.gpioSync, bSet ? HIGH : LOW);
            }
        }
        else if(bSet)
        {
            bm.counter ++;
            Serial.println(bm.Name);
        }
    }
}

/**
 * Debug: Dump unknown
*/
void parseUnknown(unsigned char *msgBuf)
{
    for(int i = 0; i < 8; ++i)
    {
        char b = msgBuf[i];
        if(b != 0)
        {
            Serial.println("unk b" + String(i) + ": " + String(int(b)));
        }
    }
}

/*
 * Commands
*/

void sendSourceChangeCommand()
{
    Serial.println("Switching Source...");
    unsigned char btnMsg[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    btnMsg[3] = 0x3;
    CAN.sendMsgBuf(0x290, 0, 8, btnMsg);
}

void sendVolumeUp()
{
    Serial.println("Increasing Volume...");
    unsigned char btnMsg[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    btnMsg[3] = 0x1;
    CAN.sendMsgBuf(0x290, 0, 8, btnMsg);
}

void sendVolumeDown()
{
    Serial.println("Decreasing Volume...");
    unsigned char btnMsg[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    btnMsg[3] = 0x2;
    CAN.sendMsgBuf(0x290, 0, 8, btnMsg);
}
