import network
import socket
import gc
from uselect import select
import utime

i = 0
connections = []
ssid = 'demo_ap'
password = '12345678'

nic=network.WLAN(network.STA_IF)
nic.active(True)
nic.disconnect()
nic.connect(ssid, password)
while not nic.isconnected():
    pass
print("wifi connection is done")
sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
sock.bind(('0.0.0.0',6665))
sock.setblocking(False)
sock.listen(3)
print('listening on')

ret = gc.threshold(2000)
gc.enable()

while True:
    r = []
    inputs = [sock]
    r, w, err = select(inputs, (), (), 0)
    
    for s in r:
        if s is sock:
            print('Next1..')
            connection, addr = s.accept()
            
            connection.setblocking(0)
            connections.append(connection)

    
    for i in connections:
        try:
            data = i.recv(128)
            if len(data) > 0:
                print(data)
                i.send(data)
            else:
                print('socket close..')
                i.close()
                connections.remove(i)
        except:
            pass