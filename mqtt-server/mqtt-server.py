import paho.mqtt.client as mqtt
import time
import mysql.connector


def on_message(client, userdata, message):
    print("message received ", str(message.payload.decode("utf-8")))
    recv = str(message.payload.decode("utf-8")).split('-')
    data = [float(recv[0]), float(recv[1]), time.strftime("%Y%m%d%H%M", time.localtime())]
    try:
        insert(data)
    except Exception as e:
        print(e)
    print("succeed")


def insert(data):
    global database, cursor
    sql = "INSERT INTO data (temp,humi,time) VALUES (%s,%s,%s)"
    val = ("%f" % data[0], "%f" % data[1], "%s" % data[2])
    cursor.execute(sql, val)
    database.commit()


database = mysql.connector.connect(host='47.98.224.249', user='root', passwd='Dyc20000830.', database='temperature')
cursor = database.cursor()
client = mqtt.Client(client_id='sql')
client.connect('47.98.224.249')
client.subscribe('test')
client.on_message = on_message
client.loop_start()
while True:
    time.sleep(2)
client.loop_stop
