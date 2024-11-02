# !/usr/bin/python
# -*- coding: utf-8 -*-
#021124 21:15

import paho.mqtt.client as mqtt
import json

TOPIC = "Airquality"
BROKER_ADDRESS = "localhost"
PORT = 1883

try:
    from iot_secrets import secrets_MQTT_mosquitto_local
except ImportError:
    print("secrets_MQTT_mosquitto_local are kept in iot_secrets.py, please add them there!")
    raise
user    = secrets_MQTT_mosquitto_local.get('user')
passwd  = secrets_MQTT_mosquitto_local.get('passwd')
 

def on_message(client, userdata, message):
    msg = str(message.payload.decode("utf-8"))
    #print("message received: ", msg)
    #print("message topic: ", message.topic)
    obj = json.loads(msg)   # convert msg to Dictionary 
    #print(obj)
    print("Pressure []:",obj["pressure"])  # extract Item 
    print("Temperature [°C]:",obj["temperature"])
    print("Humidity [rH]:",obj["humidity"])
    print("CO2 Equivalent",obj["4"])
    
    
def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT Broker: " + BROKER_ADDRESS)
    client.subscribe(TOPIC)

if __name__ == "__main__":
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.username_pw_set(user, passwd )
    client.connect(BROKER_ADDRESS, PORT)

    client.loop_forever()