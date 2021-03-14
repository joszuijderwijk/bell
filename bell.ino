#include <ESP8266WiFi.h>        // Wifi library
#include <PubSubClient.h>       // MQTT library

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic 

WiFiClient wifiClient;               // WiFi
PubSubClient client(wifiClient);     // MQTT

// PINS
const int PIN_BELL = 0;

// MQTT settings
const char* mqtt_server = "***";
const int mqtt_port = 1883;
const char* mqtt_username = "***";
const char* mqtt_password = "***";
const char* mqtt_client_name = "***";

bool isConnected = false;
bool bell_active = false;

unsigned long bellTimer;
const int bellInterval = 50;

void setup() {
  
  pinMode(PIN_BELL, OUTPUT);
  digitalWrite(PIN_BELL, LOW);
  
  WiFiManager wifiManager;
  WiFiManagerParameter custom_text("<p>(c) 2021 by <a href=\"maito:hoi@joszuijderwijk.nl\">Jos Zuijderwijk</a></p>");
  wifiManager.addParameter(&custom_text);

  if (wifiManager.autoConnect("Bel", "")){
    isConnected = true;
  }
  
  // MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}


// Try reestablishing the MQTT connection
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    if (client.connect(mqtt_client_name, mqtt_username, mqtt_password, "connection/bell", 0, 1, "0")) {
      // Send Hello World!
      client.publish("connection/bell", "1", 1);
      client.subscribe("bell/input"); 
    }
  }
}

// Handle incoming messages
void callback(char* topic, byte* payload, unsigned int len) {
    
    String msg = ""; // payload
    for (int i = 0; i < len; i++) {
      msg += ((char)payload[i]);
    }


  if ( strcmp(topic, "bell/input") == 0 ){
    if (msg == "tring"){
      ring_bell();
    }
  }
   
}

void ring_bell(){
  digitalWrite(PIN_BELL, HIGH);
  bell_active = true;
  bellTimer = millis();
}

void loop() {
    
    if (!client.connected() && isConnected){
     reconnect();
    }

    // deactivate solenoid
    if (bell_active && millis() - bellTimer > bellInterval){
      bell_active = false;
      digitalWrite(PIN_BELL, LOW);
    }
    
    client.loop();
}
