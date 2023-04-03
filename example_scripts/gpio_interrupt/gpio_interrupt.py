# This test is used to Pin 44 (Mirko bus AN pin) to trigger Pin 14 (Mirko bus CS pin) at rising edge
# To perfrom tests, connect Pin 44 (Mirko bus AN pin) to Pin 14 (Mirko bus CS pin)

from umachine import Pin
import utime

def callback_test(p):
    print("IRQ event is triggered ..")

p14 = Pin(14, mode=Pin.IN)
p14.irq(handler=callback_test, trigger=Pin.IRQ_RISING)

p44 = Pin(44, Pin.OUT)

i = 0
j = 0
while True:
    #utime.sleep(1)
    i = i+1
    if i%100000 == 0:
        j = j+1;
        if j % 2 == 0:
            print('low')
            p44.off()
        else:
            print('high')
            p44.on()