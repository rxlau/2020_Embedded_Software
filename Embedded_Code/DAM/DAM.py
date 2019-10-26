#! /usr/bin/python
 
from gps import *
import time
    
gpsd = gps(mode=WATCH_ENABLE|WATCH_NEWSTYLE)
   
try:
 
 
    while True:
        report = gpsd.next()
        if report['class'] == 'TPV':
             
            latval = getattr(report,'lat',0.0)
            longval = getattr(report,'lon',0.0)
            timeval = getattr(report,'time','')
            speedval = getattr(report,'speed','nan')
            trackval = getattr(report, 'track', 0.0)
            print(timeval)
        time.sleep(1) 
 
except (KeyboardInterrupt, SystemExit): #when you press ctrl+c
    print "Done.\nExiting."
