import network
import socket

ssid = 'demo_ap'
password = '12345678'
ip = '192.168.2.147'
port = 6666

nic=network.WLAN(network.STA_IF)
nic.active(True)
nic.connect(ssid, password)
while not nic.isconnected():
    pass
print("wifi connection is done")

addr_info = socket.getaddrinfo(ip, port)
addr = addr_info[0][-1]

s = socket.socket()
s.connect(addr)		# or s.connect(ip, port)

s.send("hello world")
print("data is send")
while True:
    data = s.recv(500)
    if len(data) > 0:
        print(data)
        print(str(data, 'utf8'), end='')
    else:
        print("\r\nsocket is close")
        s.close()
        break