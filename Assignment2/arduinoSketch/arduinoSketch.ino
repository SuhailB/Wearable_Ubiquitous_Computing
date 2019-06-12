/*
  Basic WiFi MQTT example

  This sketch demonstrates the capabilities of the pubsub library in combination
  with the WiFi101 library.

  It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

  It will reconnect to the server if the connection is lost using a blocking
  reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
  achieve the same result without blocking the main loop.



*/
#include <LiquidCrystal.h>
#include <Servo.h>
#include <WiFi101.h>
#include <PubSubClient.h>
#define SERVO 7
#define USERNAME "sebasala"
#define ANGLES_TOPIC "uark/csce5013/sebasala/angles"
#define PHRASE_TOPIC "uark/csce5013/sebasala/phrase"
#define LCD_TOPIC "uark/csce5013/sebasala/lcd"

// Update these with values suitable for your network.

unsigned char* getAngles(char* str, unsigned char length);
char* getMessage(unsigned char* angles, unsigned char length);
int ContainsNewLine(char* str);

char ssid[] = "Suhail Basalama";
char password[] = "12345678";
int status = WL_IDLE_STATUS;
const char* mqtt_server = "thor.csce.uark.edu";

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


Servo servo;
WiFiClient wifiClient;
PubSubClient client(wifiClient);
long lastMsg = 0;
char msg[50];
int value = 0;
long lastCheckConnected = -1000;

char lcd_message[] = "abcdefghijklmnopqrstuvwxyz";

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.print("----Application Started----\n");
  servo.attach(SERVO);
    // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  char index = ContainsNewLine(lcd_message);
  if(index!=-1)
  {
    lcd.setCursor(0,1);
    lcd.print(&lcd_message[index+1]);
    lcd_message[index] = '\0';
  }
  lcd.setCursor(0,0);
  lcd.print(lcd_message);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  while (status != WL_CONNECTED) {
      status = WiFi.begin(ssid, password);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
  ///////////////////////////////////
  if(strcmp(topic,ANGLES_TOPIC)==0)
  {
      unsigned char anglesCount = 1;
      for(int i=0; i<length; i++)
      {
        if((char)payload[i]==',') anglesCount++;
      }
      unsigned char* angles = getAngles((char*)payload, anglesCount);
      for(int i=0; i<anglesCount; i++)
      {
        servo.write(angles[i]);
        delay(1000);
      }
      //////////////////////additional implemention of desktop application
      char* plaintxt = getMessage(angles,anglesCount);
      client.publish(PHRASE_TOPIC,plaintxt);
  }
  else if(strcmp(topic,LCD_TOPIC)==0)
  {
      lcd.clear();
      strcpy(lcd_message,(char*) payload);
      lcd_message[length] = '\0';
      char index = ContainsNewLine(lcd_message);
      if(index!=-1)
      {
        lcd.setCursor(0,1);
        lcd.print(&lcd_message[index+1]);
        lcd_message[index] = '\0';
      }
      lcd.setCursor(0,0);
      lcd.print(lcd_message);
  }
  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(USERNAME)) {
      Serial.println("connected");
      // Once connected, subscribe to input channel
      //client.subscribe(SUBSCRIBE_TOPIC);
      //client.publish(PUBLISH_TOPIC,USERNAME);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

unsigned char* getAngles(char* str, unsigned char length)
{
    unsigned char* angles = (unsigned char*) calloc(length,sizeof(char));
    char* strArray[length];
    char* ptr = strtok(str,", ");
    strArray[0] = ptr;
    unsigned char i=1;
    while(ptr!=NULL)
    {
        ptr = strtok(NULL,", ");
        strArray[i] = ptr;
        i++;
    }
    
    for(i=0; i<length; i++)
    {
        angles[i] = atoi(strArray[i]);
    }
  
    return angles;
}

char* getMessage(unsigned char* angles, unsigned char length)
{
    char* message = (char*) calloc((length/2)+1,sizeof(char));
    unsigned char i;
    for(i=0; i<length/2; i++)
    {
        message[i] = (angles[2*i]/12)*16+(angles[2*i+1]/12);
        
    }
    message[i] = '\0';
    
    return message;
}

int ContainsNewLine(char* str)
{
    for(int i=0; i<strlen(str); i++)
    {
        if(str[i]=='\n') return i;
    }
    return -1;
}

void loop() {

  if(millis() - lastCheckConnected >= 1000){
    if(!client.connected()) {
      reconnect();
      client.publish(PHRASE_TOPIC,"HELLO");
      client.subscribe(ANGLES_TOPIC);
      client.subscribe(LCD_TOPIC);
      client.subscribe(PHRASE_TOPIC);
    }
    client.loop();
    lastCheckConnected = millis();
    
  }
    
    
    
    if(strlen(lcd_message)>16 && ContainsNewLine(lcd_message) == -1)
    {
        for (int positionCounter = 0; positionCounter < 13; positionCounter++) 
        {
          lcd.scrollDisplayLeft();
          delay(200);
        }
    }
  

  
//  value = analogRead(A0);
//  snprintf(msg,10,"%03d",value);
//  client.publish(PUBLISH_TOPIC,msg);
}
