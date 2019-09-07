#include <UIPEthernet.h>
#include <PubSubClient.h>
#include "DHT.h"

#define CLIENT_ID       "ArduinoMQTT"
#define PUB_TOPIC       "f/temperature"
#define PUB_TOPIC_2     "f/humedad"
#define SUB_TOPIC       "f/control"
#define PUBLISH_DELAY   2500
#define DHTPIN          7
#define DHTTYPE         DHT22
#define RELAY_PIN       6

uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};

IPAddress localIP(10,0,0,8);
IPAddress netmask(255,255,255,0);
IPAddress mqttServer(10,0,0,10);
EthernetClient ethClient;
PubSubClient mqttClient;
DHT dht(DHTPIN, DHTTYPE);

long previousMillis;


void setup() {

  // setup serial communication
  Serial.begin(9600);
  while(!Serial) {};
  Serial.println(F("enc28j60 and Adafruit IO"));
  Serial.println();
  
  // setup mqtt client
  mqttClient.setClient(ethClient);
  mqttClient.setServer(mqttServer, 1883);
  mqttClient.setCallback(mqttCallback);
  Serial.println(F("MQTT client configured"));

  // setup DHT sensor
  dht.begin();
  Serial.println(F("DHT sensor initialized"));

  // setup relay PIN
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  Serial.println(F("Relay PIN configured"));
  mqttConnect();

  Serial.println();
  Serial.println(F("Ready!"));
  previousMillis = millis();
}

void loop() {

  // it's time to send new data?
  if(millis() - previousMillis > PUBLISH_DELAY) {
    sendData();
    previousMillis = millis();
  }

  mqttClient.loop();
}

void mqttConnect() {

  while(!mqttClient.connected()) {
    
    if(mqttClient.connect(CLIENT_ID)) {

      Serial.println(F("MQTT client connected"));
      mqttClient.subscribe(SUB_TOPIC);
      Serial.println(F("Topic subscribed"));
    } else {
      Serial.println(F("Unable to connect, retry in 5 seconds"));
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {

  if(strncmp((const char*)payload, "ON", 2) == 0) {
    Serial.println("ON message received, turning relay ON");
    digitalWrite(RELAY_PIN, HIGH);
  } else {
    Serial.println("OFF message received, turning relay OFF");
    digitalWrite(RELAY_PIN, LOW);
  }
}

void sendData() {

  char msgBuffer[20];
  float h = dht.readHumidity();
  float t = dht.readTemperature();
    
  if(!mqttClient.connected()) mqttConnect();
  mqttClient.publish(PUB_TOPIC, dtostrf(t, 6, 2, msgBuffer));
  mqttClient.publish(PUB_TOPIC_2, dtostrf(h, 6, 2, msgBuffer));
}
