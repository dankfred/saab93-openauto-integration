#!/usr/bin/env python3
from threading import Thread
import serial
import time
import os

def initSerial():
    global ser
    ser = serial.Serial('/dev/serial0', 9600)
    ser.timeout = 1
    ser.write_timeout = 1
    ser.exclusive = True
    ser.reset_output_buffer()
    ser.reset_input_buffer()

def tryInitSerial():
    print("initializing serial...")
    while True:
        try:
            initSerial()
            break
        except:
            print("retrying...")
	
def readline():
    global ser
    line = ''
    try:
        line = ser.readline().decode('ascii').rstrip()
    except Exception as error:
        ser.close()
        print('serial error: ' + str(error))
        tryInitSerial()
    return line

def handleSerialLine(line):
    key = ''
    if line == 'btn_sw_Voice':
        key = 'M' #toggle google assistant
    elif line == 'btn_sw_SeekRight':
        key = 'N' #next track
    elif line == 'btn_sw_SeekLeft':
        key = 'V' #previous track
    elif line == 'btn_sw_Next':
        key = 'B' #play/pause
    elif line == 'btn_sw_Phone':
        key = 'P' #phone menu / calls
    elif line == 'btn_sid_Utility':
        key = 'F2' #toggle android auto night mode
    elif line == 'nightpanel_on':
        os.system('vcgencmd display_power 0') #turn off display
    elif line == 'nightpanel_off':
        os.system('vcgencmd display_power 1') #turn on display
    if key != '':
        os.system('DISPLAY=:0.0 XAUTHORITY=/home/pi/.Xauthority sudo -E xdotool key ' + key) #simulate keystroke

def handleSerialCom():
    global ser
    tryInitSerial()
    time.sleep(1) #wait for init
    ser.write(b'initialize\n') #send initialize command to esp8266/arduino
    while True:
        line = readline()
        if line != '':
            print(line)
            handleSerialLine(line)
        time.sleep(0.01)
    ser.close()

if __name__ == '__main__':
    thread = Thread(target = handleSerialCom)
    thread.start()