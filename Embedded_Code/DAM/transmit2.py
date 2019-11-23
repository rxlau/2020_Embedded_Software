import spidev
import time
spi = spidev.SpiDev()
spi.open(0, 0)
spi.max_speed_hz = 1000000
x = []
for i in range(88):
    x.append(i+1)
x = tuple(x)
print(x)
#x = (0,2,3,4,5,0xaa,0xbb,0xcc,0xdd,0xff, 1,1,1,1,1,1,1,1,1, 2,2,2,2,2,2,2,2,9)
while(1):
    resp = spi.xfer(x)
    #print("transmitting")
    time.sleep(.04)
#print(x)
    #if(resp[0]==1):
       # print("Done!")
       # break
    #time.sleep(0.1)
#print(read(resp[0]))
spi.close()
