from umachine import Pin
from umachine import Timer


cnt = 0

def mycallback(t):
    global cnt
    cnt = cnt + 1
    if (cnt % 2 != 0):
        print("yellow led off") 
        p1.on()
    else:
        print("yellow led on")
        p1.off()


p1 = Pin(16, Pin.OUT)


tim=Timer(0)
tim.init(period=2000, callback=mycallback)

while True:
  pass