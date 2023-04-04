import network
import socket

ssid = 'demo_ap'
password = 'password'
nic=network.WLAN(network.STA_IF)
nic.active(True)
nic.connect(ssid, password)
while not nic.isconnected():
    pass
print("wifi connection is done")


def http_get(url):
    import socket
    _, _, host, path = url.split('/', 3)
    addr = socket.getaddrinfo(host, 80)[0][-1]
    s = socket.socket()
    s.connect(addr)
    s.send(bytes('GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n' % (path, host), 'utf8'))
    print("HTTP GET command is send");
    while True:
        data = s.recv(100)
        if data:
            print(str(data, 'utf8'), end='')
        else:
            print("conn is close");
            break
    s.close()

http_get('http://micropython.org/ks/test.html')

