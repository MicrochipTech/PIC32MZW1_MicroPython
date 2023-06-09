import socket
import sys
import time

# Application settings
i = 0
interval = 3
ip = '192.168.2.58'
port = 6665
print ('TCP client settings:\nIP:   %s\nPort: %d' % (ip, port))
print ('Please make sure above values are pointing to your WINC device.\n')

# Create a TCP/IP socket.
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.settimeout(300)

# Connect the socket to the port where the server is listening.
server_address = (ip, port)
print ('Connecting to %s port %s...' % server_address)
sock.connect(server_address)
while i < 10: 
    i = i + 1
    try:
    
    
        # Send TCP packet to WINC1500 TCP server.
        print ('Sending TCP packet...')
        sock.sendall('Hello'.encode())
       	
        # WINC1500 TCP server will echo received data back to client.
        data = sock.recv(1460)
        print ('Received', data, 'from WFI32')
        time.sleep(interval)
    except (socket.error, socket.timeout) as msg:
        print ('Error - ',msg)
    finally:
        pass
sock.close()
sock = None
sys.exit(1)
