unsigned char NTPsuccess;
unsigned char MQTT_counter;
unsigned char NTP_counter;
void loop() {

    status = WiFi.status();
    
    if(millis() - lastCheckConnected >= 1000){
      lastCheckConnected = millis();
      wifi_counter++;
      MQTT_counter++;
      NTP_counter++;
      scroll_counter++;
    }
    
    //check wifi connection every 20 seconds
    if(wifi_counter%20==0 && status != WL_CONNECTED){
      wifi_counter = 0;
      wifi_wait = 0;
      setup_wifi();
      status = WiFi.status();
      if(status == WL_CONNECTED) 
      {
        NTPsuccess = timeClient.forceUpdate();
        if(!NTPsuccess) Serial.println("NTP time update failed");
      }
    }
    //if WIFI is available, check MQTT server every 5 seconds
    if(MQTT_counter%5==1 && status == WL_CONNECTED)
    {
      MQTT_counter = 0;
      static unsigned char success = 0;
      if(!client.connected()) {
        Serial.println("Attempting MQTT connection...");
        success = client.connect(USERNAME);
        if(success)
        {
          Serial.println("Connected to MQTT server. Subscribing to topic...");
          success = client.subscribe(TODOLIST_TOPIC);
          if(!success) 
            Serial.println("Subscribing failed. Next try in 5 seconds");
        }
      }
      if(success)
        client.loop();
      
    }
    //if WIFI is available, check NTP server every 5 seconds
    if(NTP_counter%60==0 && status == WL_CONNECTED){
      NTP_counter = 0;
      NTPsuccess = timeClient.forceUpdate();
      if(!NTPsuccess) Serial.println("NTP time update failed");
    }

    current_time = (status == WL_CONNECTED && NTPsuccess)? timeClient.getEpochTime():0;

     //counter to return to the first element if not scrolled for 60 seconds
    if(scroll_counter%60==0)
    {
      scroll_counter = 0;
      scroll = 0;
    }
    
    if(digitalRead(button) == HIGH && !passedDue) 
    {
      scroll++;
      scroll_counter = 1;
      if(scroll>4) scroll = 0;
      lcd.clear();
      delay(200);
    }
  
    ////////////////FSM logic///////////////
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