#ifndef _SAAB_CANBUS_H_
#define _SAAB_CANBUS_H_

enum eBusStateFlags
{
  EBS_Change = (1 << 0),
  EBS_Toggle = (1 << 1),
  EBS_GPIO = (1 << 2),
};

struct sBusMapping
{
    unsigned long canId;
    String Name;
    byte byteIdx;
    byte byteValue;
    byte state;
    unsigned int stateFlags;
    unsigned int counter;
    unsigned int gpioSync;
  
    sBusMapping()
    {
        canId = 0;
        state = 0;
        counter = 0;
    }
  
    sBusMapping(unsigned long id, String name, byte idx, byte value, unsigned int flags = 0, unsigned int gpio = 0)
    {
        canId = id;
        Name = name;
        byteIdx = idx;
        byteValue = value;
        stateFlags = flags;
        gpioSync = gpio;
        state = 0;
        counter = 0;        
    }
};

//listing of these and more messages and their structure: 
//https://github.com/leighleighleigh/saab-93NG-IBUS or https://www.trionictuning.com/forum/viewtopic.php?f=46&t=5763

sBusMapping busMappings[] = 
{
    /* <canbus message id>, <button name>, <byte index>, <expected value>, <flags> */
    {0x290, "btn_sw_VolUp", 3, 1},
    {0x290, "btn_sw_VolDown", 3, 2},
    {0x290, "btn_sw_Src", 3, 3},
    {0x290, "btn_sw_Voice", 3, 4},
    {0x290, "btn_sw_SeekRight", 3, 5},
    {0x290, "btn_sw_SeekLeft", 3, 6},
    {0x290, "btn_sw_Next", 3, 17},
    {0x290, "btn_sw_Phone", 3, 18},
    {0x290, "btn_sw_IndLeft", 4, 64}, //Indicators
    {0x290, "btn_sw_IndRight", 4, -128}, //Indicators
    {0x310, "btn_sid_Utility", 5, 32, EBS_Change}, //Empty SID button
    {0x460, "nightpanel", 0, 64, EBS_Toggle}, //Night panel
    {0x370, "reverse", 0, 1, EBS_Toggle | EBS_GPIO, D8} //Reverse Gear
};

#endif
