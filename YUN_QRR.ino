#include "DHT.h"
#include <SPI.h>
#include <YunClient.h>
#include <PubSubClient.h>
#define DHT11_PIN 5
#define DHTTYPE DHT11

const float DIST_LOW_DEADZONE = 3;
const float DIST_UP_DEADZONE = 200;
const float DIST_LOW_LIMIT = 5;
const float DIST_UP_LIMIT = 20;
const int N_MEASUREMENTS = 5;

float gCount = 10;

bool print_distances = true;

const int DHT_RATE = 10000; //miliseconds

unsigned long oldTime = 0;
unsigned long newTime;

float gTemp = -1; //Cloud doesn't update when -1
float gHumid = -1; //Cloud doesn't update when -1

void callback(char* topic, byte* payload, unsigned int length);

//Creating Sensor Pins
const int pinRed = 5, pinGreen = 4, trigPin = 3, echoPin = 2, trigPin2 = 8, echoPin2 = 7;
//Global Variables
char message_buff[100];
//Prepare functions
void callback(char* topic, byte* payload, unsigned int length);
String buildClientName();
String buildJson();

void countPeople();
float getDistance();
float getTemp();
float getHumid();
int peopleCount = 0;
int waitingIn = 0; 
int waitingOut = 0; 
DHT dht(DHT11_PIN, DHT11);

//IBM Watson IoT Platform Details
char orgName[] = "v09y0m";
char macstr[] = "counter";
char server[] = "v09y0m.messaging.internetofthings.ibmcloud.com";
char type[] = "arduino";

char token[] = "OdYNyJrT4Vk*7rTu-U";

int port = 1883;
String clientName = buildClientName();
//Default topic name of Watson IoT Platform
String topicName = String("iot-2/cmd/+/fmt/json");

//Objects for Arduino works
YunClient yunClient;
PubSubClient client(server, port, callback, yunClient);
//Initialize Arduino
void setup() {
 Serial.begin(9600);
 Bridge.begin();
 dht.begin();
 pinMode(pinRed, OUTPUT);
 pinMode(pinGreen, OUTPUT);
 pinMode(trigPin, OUTPUT);
 pinMode(echoPin, INPUT);
 pinMode(trigPin2, OUTPUT);
 pinMode(echoPin2, INPUT);
 delay(200);
}

//Running code
void loop() {
  newTime = millis();
  char clientStr[34];
  clientName.toCharArray(clientStr, 34);
  char topicStr[26];
  topicName.toCharArray(topicStr, 26);
  if (!client.connected()) {
     digitalWrite(pinRed, HIGH); //Device could not connect
     digitalWrite(pinGreen, LOW);
     Serial.print("Trying to connect to: ");
     Serial.println(clientStr);
     client.connect(clientStr, "use-token-auth", token);
     if (client.connect(clientStr, "use-token-auth", token)) {
         client.setCallback(callback);
         Serial.println("Connection successful ...");
         if (client.subscribe("iot-2/cmd/cid/fmt/json", 0)) {
            Serial.println("Subscribed to iot-2/cmd/cid/fmt/json");
         } else {
            Serial.println("NOT Subscribed to iot-2/cmd/cid/fmt/json");
         }
     }
 }
 if (client.connected()) {
   digitalWrite(pinRed, LOW);
   digitalWrite(pinGreen, HIGH); //Successful connection
   if((newTime - oldTime) > DHT_RATE){
    gTemp = getTemp();
    String json = buildJson();
    char JsonStr[200];
    json.toCharArray(JsonStr, 200);
    Serial.println(JsonStr);
    client.publish("iot-2/evt/status/fmt/json", JsonStr);
    gHumid = getHumid();
    json = buildJson();
    json.toCharArray(JsonStr, 200);
    Serial.println(JsonStr);
    client.publish("iot-2/evt/status/fmt/json", JsonStr);
    oldTime = newTime;
   }
   else{
      countPeople();
      String json = buildJson();
      char JsonStr[200];
      json.toCharArray(JsonStr, 200);
      Serial.println(JsonStr);
      client.publish("iot-2/evt/status/fmt/json", JsonStr);
   }
  }
}

// Builds the clientName
String buildClientName () {
 String data = "";
 data += "d:";
 data += orgName;
 data += ":";
 data += type;
 data += ":";
 data += macstr;
 return data;
}
// Builds the JSON
String buildJson() {
 char buffer[60];
 String data = "{\"d\":{\"Count\":";
   dtostrf(gCount, 4, 2, buffer);
   data += buffer;
   data += ",\"Temperature\":";
   dtostrf(gTemp, 4, 2, buffer);
   data += buffer;
   data += ",\"Humidity\":";
   dtostrf(gHumid, 3, 2, buffer);
   data += buffer;
   data += "}}";
 return data;
}

// Getting Temperature and Humidity Values
void callback(char* topic, byte* payload, unsigned int length) {}

void countPeople() {
  //CHECK PEOPLE COMING IN
  float distance1;
  float distance2;
  int measured = 0;
  for(int i=0; i<N_MEASUREMENTS; i++){
    float distance = getDistance(1);
    if(distance < DIST_LOW_DEADZONE || distance > DIST_UP_DEADZONE){
      continue;
    }
    else{
      distance1 += distance;
      measured++;
    }
  }
  if(measured > 0){
      distance1 /= measured;
  }
  else{
    distance1 = 0;
  }
   if(distance1 > DIST_LOW_LIMIT && distance1 < DIST_UP_LIMIT && waitingIn == 0 ){
      waitingIn = 1;
      gCount++;
   }
   else if(distance1 > DIST_UP_LIMIT && waitingIn == 1){
      waitingIn = 0;
   }
   //CHECK PEOPLE GOING OUT
   measured = 0;
   for(int j=0; j<N_MEASUREMENTS; j++){
    float distance = getDistance(2);
    if(distance < DIST_LOW_DEADZONE || distance > DIST_UP_DEADZONE){
      continue;
    }
    else{
      distance2 += distance;
      measured++;
    }
  }
  if(measured > 0){
      distance2 /= measured;
  }
  else{
    distance2 = 0;
  }
   if(distance2 > DIST_LOW_LIMIT && distance2 < DIST_UP_LIMIT && waitingOut == 0 ){
      waitingOut = 1;
      gCount--;
      if(gCount < 0){
        gCount = 0;
      }
   }
   else if(distance2 > DIST_UP_LIMIT && waitingOut == 1){
      waitingOut = 0;
   }
   if(print_distances){
       Serial.println("d1: ");
       Serial.println(distance1);
       Serial.println("d2: ");
       Serial.println(distance2);
   }
}

float getTemp(){
 float tempValue = dht.readTemperature();
 return tempValue;
}

float getHumid(){
 float humidValue = dht.readHumidity();
 return humidValue;
}

float getDistance(int inOut){
  int t, e;
  if(inOut == 1){
    t = trigPin;
    e = echoPin;
  }
  else if(inOut == 2){
    t = trigPin2;
    e = echoPin2;
  }
  // Clears the trigPin
   digitalWrite(t, LOW);
   delayMicroseconds(2);
   
   // Sets the trigPin on HIGH state for 10 micro seconds
   digitalWrite(t, HIGH);
   delayMicroseconds(10);
   digitalWrite(t, LOW);

   // Reads the echoPin, returns the sound wave travel time in microseconds
   return pulseIn(e, HIGH, 11764) * 0.034 / 2;
}
