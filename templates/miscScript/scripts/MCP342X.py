#!/usr/bin/python3 -u

# Written by Tristan DIDIER, OVSG/IPGP, 2019
# This script is a front-end script for miscScript plugin
# It has been, in first place, developped for and tested on BeagleBone Black
# It aims at reading i2c data from MCP342X A/D, used for instance on ADC Pi and ADC Differential Pi Boards of ABelectronics.
# This code is freely inspired from the example code provided by ABelectronics : https://www.abelectronics.co.uk/kb/article/10/adc-pi-on-a-beaglebone-black


########## PARAMETERS ##########

#i2c parameters
i2c_bus=2
adc_addresses=[0x68,0x69]

#MCP342X parameters
used_channels=4
resolution=18
pga_gain=1
continuous=1

#Sampling parameter
sampling_period=2

########## IMPORTS ##########

from smbus2 import SMBus
from time import time,gmtime,strftime,sleep
from math import ceil
from sys import stderr


########### CLASS MCP342X ##########

class MCP342X:
    """ class to drive ADC Pi Board from ABElectronics """

    
    rate_code={12:0x00, 14:0x01, 16:0x02, 18:0x03} #resolution to sample rate selection bits
    pga_code={1:0x00,2:0x01,4:0x02,8:0x03}  #PGA gain to PGA gain selection bits
    resolutionToSPS={12:240, 14:60, 16:15, 18:3.75}  #resolution to sps

    #***** __INIT__ *****

    def __init__(self,i2c_bus,i2c_address,resolution=18,pga_gain=1,continuous=1):
        """ Constructor : initialize object attributs"""
        
        #connect i2c
        self.bus=SMBus(i2c_bus)
        
        #get MCP3422 addresses
        self.i2c_address=i2c_address
        
        #get MCP3422 expected answer size and generate corresponding data/sign_filter
        if resolution==18:
            self.read_size=4
        else:
            self.read_size=3

        self.data_filter=pow(2,resolution-1)-1
        self.sign_filter=pow(2,resolution-1)

        #Generate static configuration
        S10=MCP342X.rate_code[resolution]   # Sample rate selection
        G10=MCP342X.pga_code[pga_gain]   # PGA gain selection
        C10=0x00    # Channel selection
        RDI=1   # Ready bit
        OC=continuous   # Continuous:1 , One-shot:0

        self.staticConf= RDI << 7 | C10 << 5 | OC << 4 | S10 << 2 | G10
        self.currentConfig=self.staticConf

    #***** GETADCREADING *****

    def getAdcReading(self):
        """ Read Value. Return None no data available"""
        
        adcreading = self.bus.read_i2c_block_data(self.i2c_address,self.currentConf,self.read_size)#i2c reading

        confRead = adcreading[self.read_size-1]#Extract conf byte

        if confRead & 128 :# and check if result is ready. If not, return None
            return None;

        #bitarray to int
        data=0
        for i in (range(0,self.read_size-1)):
            data+=int(adcreading[i]) << 8*(self.read_size-2-i)
                    
        #If the result is negative, convert to negative int
        if data & self.sign_filter :
            data=-(self.sign_filter-(data & self.data_filter))
                    
        #Return the result
        return data

    #***** CHANGECHANNEL *****

    def changeChannel(self, channel):
        """ Select channel to read """

        self.currentConf=self.staticConf | channel << 5 # generate conf byte from staticConf and channel parameter
        self.bus.write_byte(self.i2c_address,self.currentConf)
        

##### EPRINT #####

def eprint(msg):
    """ print to stderr """
    print(msg,file=stderr)

##########################
########## MAIN ##########
##########################

if __name__=="__main__":

########## SETUP ###########

    res_tab=[None]*(len(adc_addresses)*used_channels)

    #Initialize ABelec Object
    tab_MCP342X=[];
    for adc_address in adc_addresses :
        tab_MCP342X.append(MCP342X(i2c_bus,adc_address,resolution, pga_gain, continuous))

    #Initialize timing
    nextTime=ceil(time())


########### LOOP ##########

    while(True):
        wait=nextTime-time()

        #***** Check time sync *****

        if wait>0:
            if wait < sampling_period :
                sleep(wait)
            else :
                nextTime=ceil(time())
                eprint("Time to wait before next loop is greater than sampling_period : {} > {} -> New ref time defined".format(wait,sampling_period))
                continue
        else :
            nextTime=ceil(time())
            eprint("Last loop was {} seconds long while sampling_period is {} seconds -> New ref time defined".format(-wait+sampling_period,sampling_period))
            continue

        #***** read ADCs' channels *****
        
        #reset results' tab
        res_tab=[None]*(len(adc_addresses)*used_channels)

        #Read channel by channel, in parallel on all MCP342X
        for channel in range(used_channels) :
            #Select channel on each chip
            for MCP in tab_MCP342X :
                MCP.changeChannel(channel)

            res_counter=0
            i_MCP=0

            while res_counter<len(adc_addresses):

                i_res=channel+i_MCP*used_channels
            
                if res_tab[i_res]==None:
                    res_tab[i_res]=tab_MCP342X[i_MCP].getAdcReading()
                    if res_tab[i_res]!=None:
                        res_counter+=1

                i_MCP=(i_MCP+1)%len(adc_addresses)

        # generate ASCII frame
        frame=strftime("%Y-%m-%d %H:%M:%S", gmtime(nextTime))+','+','.join(map(str,res_tab))#+"\n"

        # and print it on stdout
        print(frame)

        #Set next loop start
        nextTime+=sampling_period



      


                
            

