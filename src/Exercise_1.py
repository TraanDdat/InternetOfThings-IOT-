import paho.mqtt.client as mqttclient
import json
import time

BROKER = "app.coreiot.io"
PORT = 1883
PASSWORD = "12345678"
USERNAME = "Exercise_1"

def subscribed(client, userdata, mid, granted_qos):
    print("Subscribed...")

def connected(client, usedata, flags, rc):
    if rc == 0:
        print("Connected successfully!!")
        client.subscribe("v1/devices/me/rpc/request/+")
    else:
        print("Connection is failed")

client = mqttclient.Client("IOT_Lab_1")
client.username_pw_set(USERNAME,PASSWORD)

client.on_connect = connected
client.connect(BROKER, PORT)
client.loop_start()
client.on_subscribe = subscribed

temp = 20
humi = 50

while True:
    collect_data = {'temperature': temp,
                    'humidity': humi}
    payload = json.dumps(collect_data)
    temp += 1
    humi += 1

    info = client.publish('v1/devices/me/telemetry', payload, 1)
    info.wait_for_publish()
    print("Published:", payload)
    time.sleep(5)