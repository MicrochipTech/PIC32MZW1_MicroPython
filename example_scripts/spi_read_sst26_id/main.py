from umachine import SPI
from umachine import Pin
import utime

print("SPI Test - Read SST26 manufacture ID and device ID")

spi=SPI(2)
rst_pin = Pin(12, mode=Pin.OUT)	# reset pin
cs_pin = Pin(14, mode=Pin.OUT)	# cs pin

rst_pin.on()
utime.sleep(1)
cs_pin.on()

utime.sleep(1)

print("Test is start..")
cs_pin.off()
spi.write(b'\x66')	# sst26 enable reset command
cs_pin.on()

cs_pin.off()
spi.write(b'\x99')	# sst26 memory reset command
cs_pin.on()


cs_pin.off()
spi.write(b'\x06')	# sst26 enable write command
cs_pin.on()

cs_pin.off()
spi.write(b'\x98')	# sst26 global block protection unlock command
cs_pin.on()

txdata = b"\x9f\xAA\xAA\xAA"	# sst26 jedec id read command
rxdata = bytearray(4)

cs_pin.off()
spi.write_readinto(txdata, rxdata)
cs_pin.on()
print("Test is finish..")

print("manufactureID = " + hex(rxdata[1]))
print("deviceID = " + hex(rxdata[2] <<8 | rxdata[3]))
