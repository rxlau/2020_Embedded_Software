#! /usr/bin/python
import sys
sys.path.append('/home/pi/DAM/gps-python3/gpspy3')
sys.path.append('/home/pi/DAM/gps-python3')

import spidev #library for spi communication
from gps import *
import time
import Adafruit_ADS1x15 #Library for ADC initializations and functions

spi = spidev.SpiDev()
spi.open(0,0)
spi.max_speed_hz = 1000000
x = [0]*88

gpsd = GPS(mode=WATCH_ENABLE|WATCH_NEWSTYLE)
adc = Adafruit_ADS1x15.ADS1115() # initialize adc variable
latval = 999
longval = 999
speedval = 999
GAIN = 2
# Print nice channel column headers.
#print('| {0:>6} | {1:>6} | {2:>6} | {3:>6} |'.format(*range(4)))
#print('-' * 37)
try:
 
 
    while True:
        values = [0]*88
        for i in range(4):
            values[i] = adc.read_adc(i,gain=GAIN)
        #print('| {0:>6} | {1:>6} | {2:>6} | {3:>6} |'.format(*values))
        report = gpsd.next()
        if report['class'] == 'TPV':
            latval = getattr(report,'lat',0.0)
            longval = getattr(report,'lon',0.0)
            timeval = getattr(report,'time','')
            speedval = getattr(report,'speed','nan')
            trackval = getattr(report, 'track', 0.0)
            values[4] = latval 
            values[5] = longval 
            values[6] = speedval 
            #print('| {0:>6} | {1:>6} | {2:>6} |'.format(*gpsvalues))
        for i in range(88):
            x[i] = int(values[i])
        if (values[4] != 0) and (values[5] != 0):
            print(x)
            print("\n")
        #x = tuple(x)
        resp = spi.xfer(x)
        time.sleep(0.65) 
 
except (KeyboardInterrupt, SystemExit): #when you press ctrl+c
    print("Done.\nExiting.")
