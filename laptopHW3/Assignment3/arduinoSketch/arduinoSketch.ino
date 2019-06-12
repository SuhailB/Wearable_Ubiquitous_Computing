#include <FlashAsEEPROM.h>
#include <FlashStorage.h>
#include <ArduinoJson.h>
#include <WiFi101.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>
#include <PubSubClient.h>
#include <LiquidCrystal.h>

#define LEAP_YEAR(Y)     ( (Y>0) && !(Y%4) && ( (Y%100) || !(Y%400) ) )

#define SERVO 7
#define USERNAME "sebasala"
#define TODOLIST_TOPIC "uark/csce5013/sebasala/todolist"

typedef struct {
  char title[36];
  uint32_t due;
  uint16_t yellow;
  uint16_t red;
} toDoItem;

typedef struct {
  toDoItem item_list[5];
  uint8_t list_length;
} toDoList;

//////////////hardware setup//////////////////////
//LCD
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
//Button
const int button = 7;
//Piezo
const int piezo = 8;
//LEDs
const int red = 10, green = 9, yellow = 8;
//////////////////////////////////////////////////
///////////////Network setup//////////////////////
//WIFI
char ssid[] = "Suhail Basalama";
char password[] = "12345678";
int status = WL_IDLE_STATUS;
//WIFI client
WiFiClient wifiClient;
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
//MTTQ client
const char* mqtt_server = "thor.csce.uark.edu";
PubSubClient client(wifiClient);
long lastMsg = 0;
char msg[50];
int value = 0;
long lastCheckConnected = -1000;

//////////////////////////////////////////////////
////////////////data structures setup/////////////
//to do list struct
toDoList tdList;
char jsonToDoList[500] = "";
//json
// allocate the memory for the document
DynamicJsonDocument doc(1024);
// FlashStorage Library declaring a variable called my_flash_store that is of type toDoList
FlashStorage(flash, toDoList);
//////////////functions///////////////////////////
String getFormattedDate(unsigned long t, unsigned long secs=0);
void insertionSort(toDoList* tdList, unsigned char n=5); 
void printToDoList(toDoList* tdList);
///////////global helper variables////////////////
unsigned char scroll;
unsigned char scroll_counter;
unsigned char scrolling = 0;
unsigned char wifi_counter;
unsigned char time_update;
uint32_t current_time = 0;
unsigned char passedDue = 0;
int wifi_wait=0;

void setup() {
  
  pinMode(button, INPUT);
  pinMode(green, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(red, OUTPUT);
  Serial.begin(9600);
  lcd.begin(16, 2);
  while(!Serial);
  
  Serial.print("----Application Started----\n");
 
    // set up the LCD's number of columns and rows:
 
  setup_wifi();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  timeClient.begin();
  timeClient.setTimeOffset(-5*3600); //-5 hours GMT

  //read to do list from flash
  tdList = flash.read();

  if(tdList.item_list[0].due == 0)
  {
    tdList.list_length = 5;
    for(int i=0; i<tdList.list_length; i++)
    {
        strcpy(tdList.item_list[i].title,"NONE");
        tdList.item_list[i].due = 2000000000;
        tdList.item_list[i].yellow = 0;
        tdList.item_list[i].red = 0;
    }
  }

  unsigned char success = timeClient.forceUpdate();
  current_time = success? timeClient.getEpochTime():0;
  
}

void printToDoList(toDoList* tdList){
  Serial.println("Start printing");
  for(int i=0; i<tdList->list_length; i++)
  {
    Serial.print(tdList->item_list[i].title);
    Serial.print(" ");
    Serial.print(tdList->item_list[i].due);
    Serial.print(" ");
    Serial.print(tdList->item_list[i].yellow);
    Serial.print(" ");
    Serial.print(tdList->item_list[i].red);
    Serial.print("\n");
  }
}
void CopyToDoItems(toDoItem* destination, toDoItem* source){
  strcpy(destination->title,source->title);
  destination->due = source->due;
  destination->yellow = source->yellow;
  destination->red = source->red;
}
void insertionSort(toDoList* tdList, unsigned char n) { 
    int i, j; 
    toDoItem key;
    for (i = 1; i < n; i++) { 
        CopyToDoItems(&key, &tdList->item_list[i]);
        j = i - 1; 

        while (j >= 0 && tdList->item_list[j].due > key.due) {
            CopyToDoItems(&tdList->item_list[j+1],&tdList->item_list[j]); 
            j = j - 1; 
        } 
        CopyToDoItems(&tdList->item_list[j + 1],&key); 
    } 
} 
void setup_wifi() {

  delay(1);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  while (status != WL_CONNECTED && wifi_wait<1) {
      Serial.println(wifi_wait);
      status = WiFi.begin(ssid, password);
      wifi_wait++;
      if(status == WL_CONNECTED) 
      {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        unsigned char success = timeClient.forceUpdate();
        current_time = success? timeClient.getEpochTime():0;
        wifi_wait=0;
        return;
      }
  }
  

}
JsonArray strToJsonArray(char* jsonToDoList){
   // parse a JSON array
  deserializeJson(doc, jsonToDoList);
  // extract the values
  JsonArray array = doc.as<JsonArray>();
  return array;
}
toDoItem* getToDoItem(JsonArray array, unsigned char index){
  JsonObject obj = array[index];
  toDoItem* tdItem = (toDoItem*) calloc(sizeof(toDoItem), 1);
  strcpy(tdItem->title,obj["title"]);
  tdItem->due = obj["due"];
  tdItem->yellow = obj["yellow"];
  tdItem->red = obj["red"];
  return tdItem;
}
String getFormattedTime(unsigned long t, unsigned long secs=0) {
  unsigned long rawTime = secs ? secs : t;
  unsigned long hours = (rawTime % 86400L) / 3600;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

  unsigned long minutes = (rawTime % 3600) / 60;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

  unsigned long seconds = rawTime % 60;
  String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

  return hoursStr + ":" + minuteStr + ":" + secondStr;
}
String getFormattedDate(unsigned long t, unsigned long secs) {
  unsigned long rawTime = (secs ? secs : t) / 86400L;  // in days
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
  return monthStr + "/" + dayStr + "/" + &year19[3] + " " + getFormattedTime(secs ? secs : 0, t);
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    strcpy(jsonToDoList, (char*)payload);
  }

  lcd.clear();
  JsonArray array = strToJsonArray(jsonToDoList);
  tdList.list_length = 5;
  for(int i=0; i<tdList.list_length; i++)
  {
    toDoItem* item = getToDoItem(array, i);
    CopyToDoItems(&tdList.item_list[i], item);
    free(item);
    if(tdList.item_list[i].due == 0)
    {
      strcpy(tdList.item_list[i].title,"NONE");
      tdList.item_list[i].due = 2000000000;
      tdList.item_list[i].yellow = 0;
      tdList.item_list[i].red = 0;
    }
  }
 
  passedDue = 0;


  
//  printToDoList(&tdList);
  insertionSort(&tdList);
//  printToDoList(&tdList);

  flash.write(tdList);

//  timeClient.update();
  unsigned char success = timeClient.forceUpdate();
  current_time = success? timeClient.getEpochTime():0;
  
  Serial.println();
  
}
void reconnect() {
  // Loop until we're reconnected

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
//      delay(5000);
    }
  
}
void loop() {
  
  if(millis() - lastCheckConnected >= 1000){
    
    status = WiFi.status();
    if(status != WL_CONNECTED  && wifi_counter>=10)
    {
      wifi_wait = 0;
      wifi_counter = 0;
      setup_wifi();
    }
    else if(status == WL_CONNECTED)
    {
      
      if(!client.connected()) {
        reconnect();
        client.subscribe(TODOLIST_TOPIC);
      }
      client.loop();
    }
    lastCheckConnected = millis();
    scroll_counter++;
    wifi_counter++;
    time_update++;
  }

  if(time_update%60 == 0)
  {
    time_update = 0;
    if(status == WL_CONNECTED)
    {
        //  timeClient.update();
        unsigned char success = timeClient.forceUpdate();
        current_time = success? timeClient.getEpochTime():0;
    }
    else
      current_time = 0;
    
  }
  
  current_time = (status == WL_CONNECTED)? timeClient.getEpochTime():0;
  
   if(digitalRead(button) == HIGH && !passedDue) 
   {
      scroll++;
      scroll_counter = 1;
      if(scroll>4) scroll = 0;
      lcd.clear();
      delay(200);
   }
  
  //counter to return to the first element if not scrolled for 60 seconds
    if(scroll_counter%60==0)
    {
      scroll_counter = 0;
      scroll = 0;
    }

   if(!passedDue)
   {
        lcd.setCursor(0,0);
        lcd.print(tdList.item_list[scroll].title);
        lcd.setCursor(0,1);
        lcd.print(getFormattedDate(tdList.item_list[scroll].due));
   }
   else
   {    
        tone(6, 1047, piezo);

        lcd.setCursor(0,0);
        lcd.print(tdList.item_list[0].title);
        lcd.setCursor(0,1);
        lcd.print("Deadline passed!");

        if(digitalRead(button)==HIGH)
        {
          printToDoList(&tdList);
          passedDue = 0;
          strcpy(tdList.item_list[0].title,"NONE");
          tdList.item_list[0].due = 2000000000;
          tdList.item_list[0].yellow = 0;
          tdList.item_list[0].red = 0;
          printToDoList(&tdList);
          insertionSort(&tdList);
          printToDoList(&tdList);
          lcd.clear();
          scroll = 0;
          delay(200);
        }
   }
 
   if(current_time < tdList.item_list[0].due - tdList.item_list[0].yellow)
   {
      digitalWrite(yellow,LOW);
      digitalWrite(green,HIGH);
      digitalWrite(red,LOW);
   }
   else if(current_time >= tdList.item_list[0].due - tdList.item_list[0].yellow)
   {
      if(current_time >= tdList.item_list[0].due - tdList.item_list[0].red)
       {
          if(current_time >= tdList.item_list[0].due && !passedDue)
          {
             lcd.clear();
             passedDue = 1;
          }
          else
          {
            digitalWrite(yellow,LOW);
            digitalWrite(green,LOW);
            digitalWrite(red,HIGH);
          }
       }
       else
       {
          digitalWrite(yellow,HIGH);
          digitalWrite(green,LOW);
          digitalWrite(red,LOW);
       }
   }

    
}
