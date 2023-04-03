

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# WLAN Network Interface
The *network* moulde is used to configure the WiFi connection. There are two type of WiFi interfaces, one is the station mode and one is the softAP mode. You can set the WiFi interface to either STA mode or SoftAP mode, but these two mode cannot work simultaneously 

## Network module
You need to import the network module for WiFi conneciton, and create the instance like below for STA or AP interface 
```
>>> import network 
>>> sta_if = network.WLAN(network.STA_IF)
```
Or 
```
>>> import network 
>>> ap_if = network.WLAN(network.AP_IF)
```

You can check if the WiFi interface is active by 
```
>>> sta_if.active()
True
```

Or

```
>>> ap_if.active()
True
```

You can also check the network settings of the interface by:

```
>>> sta_if.ifconfig()
('192.168.1.102', '255.255.255.0', '192.168.1.1', '8.8.8.8')
```
The returned values are: IP address, netmask, gateway, DNS.


### WiFi configuration (STA mode)

First activate the station interface:

```
sta_if.active(True)
```

Then connect to your WiFi network:
```
sta_if.connect('<your SSID>', '<your key>')
```

To check if the connection is established use:
```
sta_if.isconnected()
```

Once established you can check the IP address:
```
sta_if.ifconfig()
('192.168.0.2', '255.255.255.0', '192.168.0.1', '8.8.8.8')
```

Below is the example code you can run to connect the device to a target AP
```
import network
sta_if = network.WLAN(network.STA_IF)
if not sta_if.isconnected():
    print('connecting to network...')
    sta_if.active(True)
    sta_if.connect('<ssid>', '<key>')
    while not sta_if.isconnected():
        pass
print('network config:', sta_if.ifconfig())

```

### WiFi configuration (softAP mode)

First activate the station interface:

```
ap_if.active(True)
```

Then configure the AP settings, the softAP start running after this command :
```
ap_if.config(essid='<ap_ssid>', password='<key>', channel=<channel_number>)
```

Below is the example code you can run to set upt the softAP:
```
import network
ap_if = network.WLAN(network.AP_IF)
ap_if.active(True)
ap_if.config(essid='<ap_ssid>', password='<key>', channel=<channel_number>)

print('network config:', ap_if.ifconfig())

```


## HTTP webserver with softAP

This example shows setting the web server with the device's softAP mode. User can connect the to softAP and open the website by browsing http://<ip_of_the_device> 
```
import network
import socket

nic=network.WLAN(network.AP_IF)
nic.active(True)
nic.config(essid='DEMO_AP', password='12345678', channel=1)

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

```

This should retrieve the webpage and print the HTML to the console.

## Functions

**classnetwork.AbstractNIC(id=None, ...)**  
Instantiate a network interface object. Parameters are network interface dependent. If there are more than one interface of the same type, the first parameter should be id.

**AbstractNIC.active([is_active])**  
Activate (“up”) or deactivate (“down”) the network interface, if a boolean argument is passed. Otherwise, query current state if no argument is provided. Most other methods require an active interface (behaviour of calling them on inactive interface is undefined).

**AbstractNIC.connect([service_id, key=None, *, ...])**  
Connect the interface to a network. This method is optional, and available only for interfaces which are not “always connected”. If no parameters are given, connect to the default (or the only) service. If a single parameter is given, it is the primary identifier of a service to connect to. It may be accompanied by a key (password) required to access said service. There can be further arbitrary keyword-only parameters, depending on the networking medium type and/or particular device. Parameters can be used to: a) specify alternative service identifier types; b) provide additional connection parameters. For various medium types, there are different sets of predefined/recommended parameters, among them:

WiFi: bssid keyword to connect to a specific BSSID (MAC address)

**AbstractNIC.disconnect()**  
Disconnect from network.

**AbstractNIC.isconnected()**  
Returns True if connected to network, otherwise returns False.

**AbstractNIC.scan(*, ...)**  
Scan for the available network services/connections. Returns a list of tuples with discovered service parameters. For various network media, there are different variants of predefined/ recommended tuple formats, among them:

WiFi: (ssid, bssid, channel, RSSI, security, hidden). There may be further fields, specific to a particular device.

The function may accept additional keyword arguments to filter scan results (e.g. scan for a particular service, on a particular channel, for services of a particular set, etc.), and to affect scan duration and other parameters. Where possible, parameter names should match those in connect().


**AbstractNIC.ifconfig()**  
Get IP-level network interface parameters: IP address, subnet mask, gateway and DNS server. When called with no arguments, this method returns a 4-tuple with the above information. To set the above values, pass a 4-tuple with the required information. For example:
```
nic.ifconfig(('192.168.0.4', '255.255.255.0', '192.168.0.1', '8.8.8.8'))
```
**AbstractNIC.config('param')**  
**AbstractNIC.config(param=value, ...)**  
Get or set general network interface parameters. These methods allow to work with additional parameters beyond standard IP configuration (as dealt with by ifconfig()). These include network-specific and hardware-specific parameters. For setting parameters, the keyword argument syntax should be used, and multiple parameters can be set at once. For querying, a parameter name should be quoted as a string, and only one parameter can be queried at a time:
```
# Set WiFi access point name (formally known as SSID) and WiFi channel
ap.config(ssid='My AP', channel=11)
# Query params one by one
print(ap.config('essid'))
print(ap.config('channel'))
```

**AbstractNIC.status([param])**  *(Not support yet)*  
Query dynamic status information of the interface. When called with no argument the return value describes the network link status. Otherwise param should be a string naming the particular status parameter to retrieve.

The return types and values are dependent on the network medium/technology. Some of the parameters that may be supported are:

WiFi STA: use 'rssi' to retrieve the RSSI of the AP signal

WiFi AP: use 'stations' to retrieve a list of all the STAs connected to the AP. The list contains tuples of the form (MAC, RSSI).