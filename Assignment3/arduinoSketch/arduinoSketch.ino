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
 
#include <WiFi101.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <LiquidCrystal.h>




#define LEAP_YEAR(Y)     ( (Y>0) && !(Y%4) && ( (Y%100) || !(Y%400) ) )

#define SERVO 7
#define USERNAME "sebasala"
#define TODOLIST_TOPIC "uark/csce5013/sebasala/todolist"


// Update these with values suitable for your network.
struct tm *info;
// char array and pointer to hold the string
// version of the date
char date_buf[80] = "";
char* date_buf_ptr = date_buf;


char ssid[] = "Suhail Basalama";
char password[] = "12345678";
int status = WL_IDLE_STATUS;
const char* mqtt_server = "thor.csce.uark.edu";

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
Servo servo;
//WIFI client
WiFiClient wifiClient;
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
//MTTQ client
PubSubClient client(wifiClient);

typedef struct {
  char title[36];
  uint32_t due;
  uint16_t yellow;
  uint16_t red;
} toDoItem;

// Create a structure for your to-do list
// Contains:
// item_list -- length 5 array of to-do items
// list_length -- 8 bit unsigned integer
typedef struct {
  toDoItem item_list[5];
  uint8_t list_length;
} toDoList;


long lastMsg = 0;
char msg[50];
int value = 0;
long lastCheckConnected = -1000;

char lcd_message[] = "";

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.print("----Application Started----\n");
  servo.attach(SERVO);
    // set up the LCD's number of columns and rows:
 
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  timeClient.begin();
  timeClient.setTimeOffset(-5*3600); //-5 hours GMT
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

String getFormattedTime(unsigned long secs=0) {
  unsigned long rawTime = secs ? secs : timeClient.getEpochTime();
  unsigned long hours = (rawTime % 86400L) / 3600;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

  unsigned long minutes = (rawTime % 3600) / 60;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

  unsigned long seconds = rawTime % 60;
  String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

  return hoursStr + ":" + minuteStr + ":" + secondStr;
}

String getFormattedDate(unsigned long secs=0) {
  unsigned long rawTime = (secs ? secs : timeClient.getEpochTime()) / 86400L;  // in days
  unsigned long days = 0, year = 1970;
  uint8_t month;
  static const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31};

  while((days += (LEAP_YEAR(year) ? 366 : 365)) <= rawTime)
    year++;
  rawTime -= days - (LEAP_YEAR(year) ? 366 : 365); // now it is days in this year, starting at 0
  days=0;
  for (month=0; month<12; month++) {
    uint8_t monthLength;
    if (month==1) { // february
      monthLength = LEAP_YEAR(year) ? 29 : 28;
    } else {
      monthLength = monthDays[month];
    }
    if (rawTime < monthLength) break;
    rawTime -= monthLength;
  }

  String yearStr = String(year);
  char year19[5];
  strcpy(year19,yearStr.c_str());
  String monthStr = ++month < 10 ? "0" + String(month) : String(month); // jan is month 1  
  String dayStr = ++rawTime < 10 ? "0" + String(rawTime) : String(rawTime); // day of month  
  return monthStr + "/" + dayStr + "/" + &year19[2] + " " + getFormattedTime(secs ? secs : 0);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
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

void print_time(){
   //Convert time to a structure readable by time.h library
   time_t epoch = (time_t)timeClient.getEpochTime();
   //Populate the structure with the localtime
   info = localtime(&epoch);
   // Convert localtime to char buffer
   date_buf_ptr = asctime(info);
   // Print localtime
   Serial.println(date_buf_ptr);
}

void loop() {

   while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  
  if(millis() - lastCheckConnected >= 1000){
    if(!client.connected()) {
      reconnect();
      client.subscribe(TODOLIST_TOPIC);
    }
    client.loop();
    lastCheckConnected = millis();
    
  }

//  formattedDate = getFormattedDate();
   if(millis() %1000 == 1){
      Serial.println(getFormattedDate());
   }
    
    
}
