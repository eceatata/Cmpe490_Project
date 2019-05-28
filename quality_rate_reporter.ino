#include "DHT.h"
#include <SPI.h>
#include <YunClient.h>
#include <PubSubClient.h>
#define DHT11_PIN 5
#define DHTTYPE DHT11

unsigned long oldTime = 0;
unsigned long newTime;
int gCount = 0;
float gTemp = 25.00;
float gHumid = 30.00;

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
float getTemp();
float getHumid();
int peopleCount = 0;
float lastInDistance;
float lastOutDistance;
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
   if((newTime - oldTime) > 5000){
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
  long duration;
  float distance;

   // Clears the trigPin
   digitalWrite(trigPin, LOW);
   delayMicroseconds(2);

   // Sets the trigPin on HIGH state for 10 micro seconds
   digitalWrite(trigPin, HIGH);
   delayMicroseconds(10);
   digitalWrite(trigPin, LOW);

   // Reads the echoPin, returns the sound wave travel time in microseconds
   duration = pulseIn(echoPin, HIGH);

   // Calculating the distance
   distance = duration * 0.034 / 2;
   if(distance > 10 && distance < 40 && waitingIn == 1 ){
      waitingIn = 0;
      gCount++;
   } else if(distance < 10 || distance > 1000) {
      waitingIn = 0;
   } else {
      waitingIn = 1;
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
