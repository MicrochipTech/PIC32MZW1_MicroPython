import network
import socket
import gc

softap_ssid = 'MicroPython_Demo_AP'
softap_pw = '12345678'

nic=network.WLAN(network.AP_IF)
nic.active(True)
nic.config(essid=softap_ssid, password=softap_pw, channel=1)

def web_page():
  html = """<html><head><meta name="viewport" content="width=device-width, initial-scale=1"></head>
  <body><h1>Hello, World!</h1></body></html>"""
  return html


sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
sock.bind(('0.0.0.0',80))
sock.listen(3)

while True:
  conn, addr = sock.accept()
  print('Got a connection from %s' % str(addr))
  request = conn.recv(1024)
  print('Content = %s' % str(request))
  response = web_page()
  conn.send(response)
  conn.close()