import umachine
import network

ssid = 'demo_ap'
password = '12345678'

pins = [umachine.Pin(i, umachine.Pin.IN) for i in (16, 41, 47)]

html = """<!DOCTYPE html>
<html>
    <head> <title>PIC32MZW1 Pins State</title> </head>
    <body> <h1>PIC32MZW1 Pins State</h1>
        <table border="1"> <tr><th>Pin</th><th>Value</th></tr> %s </table>
    </body>
</html>
"""


nic=network.WLAN(network.STA_IF)
nic.active(True)
nic.connect(ssid, password)
while not nic.isconnected():
    pass
print("wifi connection is done")

import socket
addr = socket.getaddrinfo('0.0.0.0', 80)[0][-1]

s = socket.socket()
s.bind(addr)
s.listen(1)

print('listening on', addr)

while True:
    cl, addr = s.accept()
    print('client connected from', addr)
    cl_file = cl.makefile('rwb', 0)
    while True:
        line = cl_file.readline()
        if not line or line == b'\r\n':
            break
    rows = ['<tr><td>%s</td><td>%d</td></tr>' % (str(p), p.value()) for p in pins]
    response = html % '\n'.join(rows)
    cl.send('HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n')
    cl.send(response)
    cl.close()

