from umqtt.simple import MQTTClient
import time
# Test reception e.g. with:
# mosquitto_sub -t foo_topic


def main(server="localhost"):
    c = MQTTClient("umqtt_client", server)
    c.connect()
    c.publish(b"foo_topic", b"hello")
    print('done')
    c.disconnect()


while 1:
    main('47.98.224.249')
    time.sleep(2)
