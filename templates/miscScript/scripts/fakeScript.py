#!/usr/bin/python3 -u

# Simple Script example to generate fix rate ASCII frames with random data and print it on standard output

###
import sys,time
from datetime import datetime,timedelta
import random


### Parameters ###
channels_nb=3 #How many channel do you want ?
period_s=1 #Sample_period (second)
period_ms=0 #Sample_period (millisecond, can be combined with period_s)


try:

    next_time=datetime.now()+timedelta(seconds=period_s,microseconds=period_ms*1000)

    while True:
        data=""
        for i in range(channels_nb):
            data=data+","+str(int(round(random.uniform(-10000,10000),0)))

        timeStr=datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
        msg=timeStr+data+"\n"
        print(msg)

        time.sleep((next_time-datetime.now())/timedelta(seconds=1))
        next_time= next_time+timedelta(seconds=period_s,microseconds=period_ms*1000)

except Exception as msg:
    raise


