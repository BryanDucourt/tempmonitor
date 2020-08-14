from machine import Pin, UART
from umqtt.simple import MQTTClient
import time
import uos


uos.dupterm(None, 1)
uart = UART(0, 115200)
uart.init(baudrate=115200, bits=8,parity=None,stop=1,timeout=10,rx=Pin(3))
buff = 1024

c = MQTTClient('umqtt_client', '47.98.224.249')
c.connect()
read=b''
uart.write(bytearray('111'))
while True:
    if uart.any():
        read = uart.read(11)
        uart.write(read)
        c.publish(b'test', read)







