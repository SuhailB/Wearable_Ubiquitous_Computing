#include <ArduinoJson.h>
// Include time.h library
#include <time.h>
// tm struct from time.h -- Contains
// fields describing the time in readable
// format
struct tm *info;
// char array and pointer to hold the string
// version of the date
char date_buf[80] = "";
char* date_buf_ptr = date_buf;

uint32_t cur_time = 1523559641;

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


toDoList tdList;
char jsonToDoList[] = "[{\"title\": \"Homework3\",\"due\": 1523663999,\"yellow\": 900,\"red\": 60},/{\"title\": \"Homework1\",\"due\": 1523663999,\"yellow\": 400,\"red\": 120}]";

// allocate the memory for the document
DynamicJsonDocument doc(1024);

void setup() {
  // Initialize Serial Monitor
  
  Serial.begin(9600);
  while(!Serial);
  Serial.print("----Application Started----\n");


  setenv("TZ", "CST6CDT", 1);
  tzset();
  JsonArray array = strToJsonArray(jsonToDoList);
  toDoItem* item = getToDoItem(array, 0);
  tdList.item_list[0] = *item;
}

JsonArray strToJsonArray(char* jsonToDoList)
{
   // parse a JSON array
  deserializeJson(doc, jsonToDoList);
  // extract the values
  JsonArray array = doc.as<JsonArray>();
  return array;
}

toDoItem* getToDoItem(JsonArray array, unsigned char index)
{
  JsonObject obj = array[index];
  toDoItem* tdItem = (toDoItem*) calloc(sizeof(toDoItem), 1);
  strcpy(tdItem->title,obj["title"]);
  tdItem->due = obj["due"];
  tdItem->yellow = obj["yellow"];
  tdItem->red = obj["red"];
  Serial.println(tdItem->title);
  Serial.println(tdItem->due);
  Serial.println(tdItem->yellow);
  Serial.println(tdItem->red);
  return tdItem;
}

void print_time(){
   //Convert time to a structure readable by time.h library
   time_t epoch = (time_t)cur_time;
   //Populate the structure with the localtime
   info = localtime(&epoch);
   // Convert localtime to char buffer
   date_buf_ptr = asctime(info);
   // Print localtime
   Serial.println(date_buf_ptr);
}

void loop() {
       // put your main code here, to run repeatedly:
  if(millis() %1000 == 1){
    //Every second, update the clock by 1 second
    cur_time += 1;
    //Every 5 seconds print the time in readable format
//    if(cur_time %5 == 0){
//      print_time();
//    }
  }
  delay(1);
}
