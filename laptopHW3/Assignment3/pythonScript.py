# -*- coding: utf-8 -*-
"""
Test MQTT for Arduino MKR1000
Created on Fri Feb 23 14:53:01 2018

@author: ahnelson
"""
import paho.mqtt.client as mqtt
import time

#Constants used in the script
ToDoList_TOPIC = 'uark/csce5013/sebasala/todolist'
AD_TOPIC = 'uark/csce5013/sebasala/ad'
flag = 0;
# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
	print(msg.topic+" "+str(msg.payload))
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

current_time = int(time.time())
current_time -= 5*3600
t1 = current_time + 180
t2 = current_time + 120
t3 = current_time + 60
client.subscribe(ToDoList_TOPIC)

t1_str = str(t1)
t2_str = str(t2)
t3_str = str(t3)

# print(current_time_str)

print("Publishing \"JSON\" to topic " + ToDoList_TOPIC)

tdList = "[{\"title\": \"Homework3\",\"due\": " + t1_str + ",\"yellow\": 20,\"red\": 10},{\"title\": \"Homework1\",\"due\": " + t2_str + ",\"yellow\": 15,\"red\": 5},{\"title\": \"Algorithms\",\"due\": "+t3_str+",\"yellow\": 10,\"red\": 5}]"

client.publish(ToDoList_TOPIC,tdList, retain=True)



#Non-blocking loop. Starts a new thread that listens for messages and prints them
client.loop_forever()
