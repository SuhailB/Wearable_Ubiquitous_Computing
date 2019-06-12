// Uses FlashStorage library available in the Arduino libraries manager
#include <FlashAsEEPROM.h>
#include <FlashStorage.h>

// Create a structure for a to-do item
// Contains:
// title -- a 36 byte char array (string)
// due -- 32 bit unsigned integer
// yellow -- 16 bit unsigned integer
// red -- 16 bit unsigned integer
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

// FlashStorage Library declaring a variable called my_flash_store that is of type toDoList
FlashStorage(my_flash_store, toDoList);

// Create a toDoList structure
toDoList myToDoList;

// Character array buffer for string manipulation
char my_buf[36];

void setup() {
  Serial.begin(115200);
  // Delay 10 seconds so that you have time to get serial monitor ready
  delay(10000);

  // Read the content of "my_flash_store" and assign it to "myToDoList"
  // If flash is empty, will contain 0s
  myToDoList = my_flash_store.read();
  // Print current list length
  print_list_to_serial();
  Serial.println(myToDoList.list_length);
}

void loop() {
    // Declare variables to be used in the loop
    String title;
    int list_len = myToDoList.list_length;
    String input_string;

    // Print out what is to be entered
    Serial.println("Please enter to-do-title:");
    // Wait until there is data available on Serial buffer
    while(!Serial.available());  
    // Read in serial buffer until null or new line
    title = Serial.readStringUntil('\n');
    // Copy title to a char array
    title.toCharArray(my_buf,36);
    // Copy the char array over into the toDoList stucture
    strcpy(myToDoList.item_list[list_len].title,my_buf);

    // Print out what is to be entered
    Serial.println("Please enter deadline in epoch time:");
    while(!Serial.available());
    // Wait until there is data available on Serial buffer
    input_string = Serial.readStringUntil('\n');  
    // Convert string to an int -- Defaults to 0 if not an integer
    uint32_t deadline = input_string.toInt();
    Serial.println("Please enter yellow deadline:");
    while(!Serial.available());  
    input_string = Serial.readStringUntil('\n');  
    // Convert string to an int -- Defaults to 0 if not an integer
    uint16_t yellow = input_string.toInt();
    Serial.println("Please enter red deadline:");
    while(!Serial.available());  
    input_string = Serial.readStringUntil('\n'); 
    // Convert string to an int -- Defaults to 0 if not an integer 
    uint16_t red = input_string.toInt();

    // Copy values from variables in current scope to the toDoList items
    myToDoList.item_list[list_len].due = deadline;
    myToDoList.item_list[list_len].yellow = yellow;
    myToDoList.item_list[list_len].red = red;

    // Increment list length
    // Note that if length == 5, then the array is full
    // No error handling for that in this code
    myToDoList.list_length += 1;  
    // Print the current list to the serial port
    print_list_to_serial();
    // Store the current structure to flash
    my_flash_store.write(myToDoList);
}

void print_list_to_serial(){
  Serial.println("-----------------");
  Serial.println("Current To Do List:");
  Serial.print("List Length: ");
  Serial.println(myToDoList.list_length);
  Serial.println();
  for(int i = 0; i<myToDoList.list_length; i++){
    Serial.print("To Do Item #");
    Serial.println(i);
    Serial.print("Title: ");
    Serial.println(myToDoList.item_list[i].title);
    Serial.print("Deadline: ");
    Serial.println(myToDoList.item_list[i].due);    
  }
  Serial.println("-----------------");
}
