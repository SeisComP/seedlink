#!/usr/bin/python3 -u

# Simple Script to generate fix rate ASCII frames and send it througth a serial port
# pyserial library  have to be installed installed first (https://github.com/pyserial/pyserial)
# For testing, you can use socat to get virtual serial ports :
# socat -d -d pty,raw,echo=0 pty,raw,echo=0

###
import serial
import sys,time
from datetime import datetime,timedelta
import random

### Parameters ###
channels_nb=3 #How many channnel do you want
port="/dev/ttyUSB0" #Serial port used by this script to output data
ser_speed=9600 #Serial port speed
period_s=1 #Sample Period (second)
period_ms=0 #Sample Period (millisecond, can be combined with period_s)


try:

    ser=serial.Serial(port,ser_speed,rtscts=0)
    next_time=datetime.now()+timedelta(seconds=period_s,microseconds=period_ms*1000)

    while True:
        data=""
        for i in range(channels_nb):
            data=data+","+str(round(random.uniform(-10000,10000),0))
        timeStr=datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
        msg=timeStr+data+"\n"
        print(msg)
        ser.write(msg.encode())

        time.sleep((next_time-datetime.now())/timedelta(seconds=1))
        next_time= next_time+timedelta(seconds=period_s,microseconds=period_ms*1000)

except Exception as msg:
    raise

finally:
    ser.close()
