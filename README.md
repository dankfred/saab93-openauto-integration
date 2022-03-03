
# Saab 9-3 NG I-Bus OpenAuto Integration

Allows for the integration of steering wheel buttons, nightpanel and more with OpenAuto.
There is potential for more integrations, this is just the tip of the iceberg.
This is by no means production-ready, this is merely a weekend project I took on.

# Demo
https://www.youtube.com/watch?v=x3kj8ivPpb4

# Required Hardware
Aside from the (obvious) Raspberry Pi, you will need:

 - MCP2515 CAN module
 - Arduino / NodeMCU / esp8266 (basically any microcontroller)

### Setup Diagram
```mermaid
graph LR
A[Saab I-Bus] -- CAN H + L --> B[MCP2515] -- wiring --> C[Microcontroller] -- serial --> D[Raspberry Pi]
```
You can technically bypass the Microcontroller if you read the CAN messages from the raspberry, but I found it to be easier to debug with this setup.

# Getting Started
Wire up the MCP2515 to your microcontroller of choice:

| MCP2515 | ESP8266 | Arduino |
| ------- | ------- | ------- |
| INT     | optional | optional
| SCK     | D5      | 13
| MOSI    | D7      | 11
| MISO    | D6      | 12
| CS      | D2      | 10
| GND     | GND     | GND
| VCC     | 5V      | 5V

**MCP2515 needs 5v to operate.**

Update the script with the GPIO pin that you have connected the MCP2515's CS pin to.

Once wired up, flash the the microcontroller with Arduino IDE. Make sure you have the [mcp_can](https://github.com/coryjfowler/MCP_CAN_lib) library installed. For those using a NodeMCU/esp8266 or any other controller other than an Arduino, make sure you have the proper board support installed on Arduino IDE.

## MCP2515 vehicle wiring
Connect the Can High (H) to a I-Bus wire and Can Low (L) to ground.
You can splice a I-Bus wire on the ICM (Infotainment) connector (green cable, pin 1)

| MCP2515| Vehicle |
|----|--------|
| L | Ground|
| H | I-Bus|

**Note:** For some reason, when the Raspberry Pi is connected to the MCP2515, attaching L to Ground alone doesn't always work. What worked for me is to attach L to the MCP2515's GND pin with a 4k7Ω resistor (given that the raspberry is getting power from the vehicle):

![](https://cdn.fredaikis.com/public/7fbfebb54ead6f77424acb7132e93069/3abd2d52dbf06128ef6809f4cca5f60b/mcp_resistor.png)

See: http://saabworld.net/showthread.php?t=25524

> The bus lead is a green cable that is internally grounded in each control module via a 9 k8 or a 4 k8 resistor.

## Testing I-Bus
With the MCP2515 connected to the car and microcontroller, you should be able to attach Serial Monitor with a laptop and view the live data on the steering wheel buttons. Once sure it works, move on to setting up the Raspberry.

## Raspberry Pi Setup
Wire up a serial connection between your microcontroller and the Raspberry Pi.
 - **Warning**: Arduino usually operates at 5v, the Raspberry operates serial at 3.3v.
	 - You want to make sure you use a board that operates at 3.3v as to not fry the Raspberry Pi
	 - This is why I ended up opting for a Wemos D1 mini (esp8266) - *and partially because that's what I had laying around*

| Microcontroller | Raspberry Pi |
|----|--------|
| TX | RX (8) |
| RX | TX (10) |
| VCC | 5V / 3.3V |
| GND| GND |

If your microcontroller is powered by 3.3v, make sure you give the MCP2515 5V.

Make sure serial is enabled on your Raspberry Pi:
 - Open `/boot/config.txt`
 - Make sure that: `enable_uart=1`, add it if it doesn't exist.

Console over serial needs to be disabled on your Raspberry Pi:
*Otherwise it will create interferance and corrupt your serial communication*

 - Open `/boot/cmdline.txt`
 - Remove: `console=serial0, 115200`

Install xdotool: *(necessary to simulate keypresses for media controls)*

    sudo apt-get install -y xdotool

If you are running OpenAuto, you should already have python & pyserial installed. If not:

    sudo apt-get install -y python3
    sudo pip3 install pyserial

Reboot your Raspberry Pi.

The `canbus.py` python script reads serial messages sent by the microcontroller and translates that into actions such as Keystrokes *(OpenAuto media controls)*.

Copy `canbus.py` to any directory on your Raspberry, I will be using `/var/canbus.py`.
Make sure the file can be executed:

    sudo chmod +x /var/canbus.py

You can now test the whole setup by running the python script:

    sudo /var/canbus.py
   You should be able to see the incoming serial messages from the microcontroller.
**Note:** the script needs to run as super-user in order for it to work.

Making the script start when the pi boots:
There are many ways of doing this, I'm using the `/etc/rc.local` approach.
 - Open `/etc/rc.local`
 - Add to the end: `sudo /usr/bin/python3 /var/canbus.py &`
	 - The `&` sign is to signal it to start on a detached process as to not block the rc.local script.

Reboot your Raspberry Pi.

And with this, you should have working steering wheel controls and a basic first integration of your Saab 9-3 with OpenAuto.

## Good to know
 - The MCP2515 can be very picky about voltage, make sure it's getting reliable 5v.
 - Car USB chargers can cause noise on the car's ground interfering with the MCP2515's ability to read/write data to the car's bus.
