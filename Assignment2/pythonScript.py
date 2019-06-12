# -*- coding: utf-8 -*-
"""
Test MQTT for Arduino MKR1000
Created on Fri Feb 23 14:53:01 2018

@author: ahnelson
"""
import paho.mqtt.client as mqtt
import time

#Constants used in the script
TOPIC = 'uark/csce5013/sebasala/phrase'
AD_TOPIC = 'uark/csce5013/sebasala/ad'
flag = 0;
# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
	# print(msg.topic+" "+str(msg.payload))
	# if(msg.topic == AD_TOPIC):
		# client.publish(TOPIC,str(msg.payload),True)

def on_subscribe(client,userdata,mid,granted_qos):
	print("Subscribing to topic: " + client.topic)

#Get an instance of the MQTT Client object
client = mqtt.Client()
#Set the function to run when the client connects
client.on_connect = on_connect
#Set the function to run when a message is received
client.on_message = on_message
#
client.on_subscribe = on_subscribe
#Connect to the broker.mqtt-dashboard.com server on port 1883
client.connect("thor.csce.uark.edu", 1883, 60)


client.subscribe(TOPIC)
client.subscribe(AD_TOPIC)

# print("Publishing \"HELLO\" to topic " + TOPIC)
client.publish(TOPIC,"HELLO",True)

print(time.time())

#Non-blocking loop. Starts a new thread that listens for messages and prints them
client.loop_forever()
