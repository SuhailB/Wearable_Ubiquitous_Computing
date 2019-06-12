#include <Servo.h>

//pin and constant definitions
#define BUTTON 0
#define RED 1
#define GREEN 2
#define SENSOR A1
#define SERVO 7
#define THRESHOLD 800

//global variables and objects
Servo servo;
unsigned char mode = 0;
unsigned int counter;
unsigned char time;
int now;
int prev;
unsigned char pos;

void setup() {
  
  //setting up different components
  pinMode(BUTTON,INPUT);
  pinMode(RED,OUTPUT);
  pinMode(GREEN,OUTPUT);
  pinMode(SENSOR,INPUT);
  Serial.begin(9600);
  servo.attach(SERVO);

  //initial blinking upon reset
  for(int i=0; i<3; i++)
  {
    digitalWrite(GREEN,HIGH);
    digitalWrite(RED,HIGH);
    delay(100);
    digitalWrite(GREEN,LOW);
    digitalWrite(RED,LOW);
    delay(100);
  }
}

void loop() {

  servo.write(0);

//   //two variables to count the time an iteration takes
//  prev = now;
//  now = micros();
//  //reading the brightness level from the sensor
//  unsigned short brightness = analogRead(SENSOR);
//
////switch between the two modes
//  switch(mode)
//  {
//    case 0:
//    {
//      //devided brightness (max 1023) by 5.7 to fit 
//      //in the servo.write function which takes (max 180)
//      //1023/180 = 5.7
//      pos = brightness/5.7;
//      servo.write(pos);
//
//      if(digitalRead(BUTTON)==1)
//      {
//        mode = 1;
//        digitalWrite(GREEN,HIGH);
//        servo.write(179);
//        pos = 179;
//        time = 0;
//        //delay for preventing rapid switching between modes
//        delay(200);
//      }
//      
//    }
//    break;
//    case 1:
//    {
//      if(brightness>THRESHOLD && pos>0)
//      {
//        
//       counter++;
//       //timer for 30 seconds using 10^6 micros / iteration time (now-prev)
//       if(counter>=1000000/(now-prev))
//       {
//        counter = 0;
//        time++;
//       }
//       //when time is 30 seconds position should be zero
//       pos = 180 - (6*time);
//       servo.write(pos);
//      }
//      
//      else if(pos==0)
//      {
//        digitalWrite(RED,HIGH);
//        digitalWrite(GREEN,LOW);
//      }
//      
//      if(digitalRead(BUTTON)==1)
//      {
//        mode = 0;
//        digitalWrite(RED,LOW);
//        digitalWrite(GREEN,LOW);
//        //delay for preventing rapid switching between modes
//        delay(200);
//      }
//    }
//  }
//  
}
