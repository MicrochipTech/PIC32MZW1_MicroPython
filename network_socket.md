

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# Network Socket
Both TCP and UDP socket can be support for TCP/IP network connection

## Socket module
You need to import the socket module to create TCP/ UDP connection  
```
>>> import socket
```


## Create socket

For tests, run below scripts to start the TCP server on your PC
```
python utilities/tcp_server.py
```

To create socket cnnection, you need to make sure the board is connected to the target Wi-Fi AP first. To do this, you can make reference [XXX]() 

To get the IP address of the server:
```
>>> addr_info = socket.getaddrinfo("xxx.com", 6666)
```
Or enter use the ip address directly
```
>>> addr_info = socket.getaddrinfo("192.168.11.3", 6666)
```

The getaddrinfo function returns a list of addresses, and each address has more information than we need. We want to get just the first valid address, and then just the IP address and port of the server. To do this use:
```
>>> addr = addr_info[0][-1]
```

If you type addr_info and addr at the prompt you will see exactly what information they hold.  
Using the IP address we can make a socket and connect to the server:
```
>>> s = socket.socket()
>>> s.connect(addr)
```

Now that we are connected, we can send and receive the data:
```
>>> s.send("hello world")
>>> while True:
...     data = s.recv(500)
...     print(data)
...
```

When this loop executes it should start showing the animation

You should also be able to run this same code on your PC using normal Python if you want to try it out there.

## HTTP GET request

This example shows how to download a webpage. HTTP uses port 80 and you first need to send a "GET" request before you can download anything. As part of the request you need to specify the page to retrieve.

Let's define a function that can download and print a URL:
```
def http_get(url):
    import socket
    _, _, host, path = url.split('/', 3)
    addr = socket.getaddrinfo(host, 80)[0][-1]
    s = socket.socket()
    s.connect(addr)
    s.send(bytes('GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n' % (path, host), 'utf8'))
    while True:
        data = s.recv(100)
        if data:
            print(str(data, 'utf8'), end='')
        else:
            break
    s.close()

http_get('http://micropython.org/ks/test.html')
```

This should retrieve the webpage and print the HTML to the console.

## Simple HTTP server

The following code creates an simple HTTP server which serves a single webpage that contains a table with the state of all the GPIO pins:

```
import umachine
pins = [umachine.Pin(i, umachine.Pin.IN) for i in (16, 41, 47)]

html = """<!DOCTYPE html>
<html>
    <head> <title>PIC32MZW1 Pins State</title> </head>
    <body> <h1>PIC32MZW1 Pins State</h1>
        <table border="1"> <tr><th>Pin</th><th>Value</th></tr> %s </table>
    </body>
</html>
"""

import network
nic=network.WLAN(network.STA_IF)
nic.active(True)
nic.connect('<ssid>', '<key>')
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
```

## Functions

**socket.getaddrinfo(host, port, af=0, type=0, proto=0, flags=0, /)**

 * Translate the host/port argument into a sequence of 5-tuples that contain all the necessary arguments for creating a socket connected to that service. Arguments af, type, and proto (which have the same meaning as for the socket() function) can be used to filter which kind of addresses are returned. If a parameter is not specified or zero, all combinations of addresses can be returned (requiring filtering on the user side).

 * The resulting list of 5-tuples has the following structure:  
     (family, type, proto, canonname, sockaddr)  
     The following example shows how to connect to a given url:
    ```
    s = socket.socket()
    # This assumes that if "type" is not specified, an address for
    # SOCK_STREAM will be returned, which may be not true
    s.connect(socket.getaddrinfo('www.micropython.org', 80)[0][-1])

    ```
    Recommended use of filtering params:
    ```
    s = socket.socket()
    # Guaranteed to return an address which can be connect'ed to for
    # stream operation.
    s.connect(socket.getaddrinfo('www.micropython.org', 80, 0, socket.SOCK_STREAM)[0][-1])
    ```


## Constants
**socket.AF_INET**
**socket.AF_INET6**
* Address family types. Availability depends on a particular MicroPython port.

**socket.SOCK_STREAM**
**socket.SOCK_DGRAM**
Socket types.

**socket.IPPROTO_UDP**
**socket.IPPROTO_TCP**
* IP protocol numbers. Availability depends on a particular MicroPython port. Note that you don’t need to specify these in a call to socket.socket(), because SOCK_STREAM socket type automatically selects IPPROTO_TCP, and SOCK_DGRAM - IPPROTO_UDP. Thus, the only real use of these constants is as an argument to setsockopt().




## class socket

** class socket.socket(af=AF_INET, type=SOCK_STREAM, proto=IPPROTO_TCP, /)**
* Create a new socket using the given address family, socket type and protocol number. Note that specifying proto in most cases is not required (and not recommended, as some MicroPython ports may omit IPPROTO_* constants). Instead, type argument will select needed protocol automatically:

    ````
    # Create STREAM TCP socket
    socket(AF_INET, SOCK_STREAM)
    # Create DGRAM UDP socket
    socket(AF_INET, SOCK_DGRAM)
    ````
    
## Methods

**socket.close()**
* Mark the socket closed and release all resources. Once that happens, all future operations on the socket object will fail. The remote end will receive EOF indication if supported by protocol.

* Sockets are automatically closed when they are garbage-collected, but it is recommended to close() them explicitly as soon you finished working with them.

**socket.bind(address)**
* Bind the socket to address. The socket must not already be bound.

**socket.listen([backlog])**
* Enable a server to accept connections. If backlog is specified, it must be at least 0 (if it’s lower, it will be set to 0); and specifies the number of unaccepted connections that the system will allow before refusing new connections. If not specified, a default reasonable value is chosen.

**socket.accept()**
* Accept a connection. The socket must be bound to an address and listening for connections. The return value is a pair (conn, address) where conn is a new socket object usable to send and receive data on the connection, and address is the address bound to the socket on the other end of the connection.

**socket.connect(address)**
* Connect to a remote socket at address.

**socket.send(bytes)**
* Send data to the socket. The socket must be connected to a remote socket. Returns number of bytes sent, which may be smaller than the length of data (“short write”).

**socket.sendall(bytes)**
* Send all data to the socket. The socket must be connected to a remote socket. Unlike send(), this method will try to send all of data, by sending data chunk by chunk consecutively.

* The behaviour of this method on non-blocking sockets is undefined. Due to this, on MicroPython, it’s recommended to use write() method instead, which has the same “no short writes” policy for blocking sockets, and will return number of bytes sent on non-blocking sockets.

**socket.recv(bufsize)**
* Receive data from the socket. The return value is a bytes object representing the data received. The maximum amount of data to be received at once is specified by bufsize.

**socket.sendto(bytes, address)**
* Send data to the socket. The socket should not be connected to a remote socket, since the destination socket is specified by address.

**socket.recvfrom(bufsize)**
* Receive data from the socket. The return value is a pair (bytes, address) where bytes is a bytes object representing the data received and address is the address of the socket sending the data.

**socket.setsockopt(level, optname, value)** *(Not support)*  
* Set the value of the given socket option. The needed symbolic constants are defined in the socket module (SO_* etc.). The value can be an integer or a bytes-like object representing a buffer.

**socket.settimeout(value)**

* Set a timeout on blocking socket operations. The value argument can be a nonnegative number expressing milliseconds, or None. If a non-zero value is given, subsequent socket operations will raise an OSError exception if the timeout period value has elapsed before the operation has completed. If zero is given, the socket is put in non-blocking mode. If None is given, the socket is put in blocking mode.

**socket.setblocking(flag)**
* Set blocking or non-blocking mode of the socket: if flag is false, the socket is set to non-blocking, else to blocking mode.

**socket.read([size])**
* Read up to size bytes from the socket. Return a bytes object. If size is not given, it reads all data available from the socket until EOF; as such the method will not return until the socket is closed. This function tries to read as much data as requested (no “short reads”). This may be not possible with non-blocking socket though, and then less data will be returned.

**socket.readinto(buf[, nbytes])**
* Read bytes into the buf. If nbytes is specified then read at most that many bytes. Otherwise, read at most len(buf) bytes. Just as read(), this method follows “no short reads” policy.

* Return value: number of bytes read and stored into buf.

**socket.readline()**
* Read a line, ending in a newline character.

* Return value: the line read.

**socket.write(buf)**
* Write the buffer of bytes to the socket. This function will try to write all data to a socket (no “short writes”). This may be not possible with a non-blocking socket though, and returned value will be less than the length of buf.

* Return value: number of bytes written.