/*
  Sensor prototype for ESP8266 
  very very very basic prototype that reads a buttons state and sends an update to the mqtt broker anytime it changes

  sends button state changes to sensor1/state
  responds to "ping" with "pong" at sensor1/ping

  network config in config.h
*/

#include "EspMQTTClient.h"

// Net config
#include "secrets.h"

#define IN_PIN 14 // pin D5 on esp8266 MCU
#define SENSOR_ID "sensor1" // ideally will be something more unique in the future

// Currently mostly redundant 
struct BinarySensor {
  char macraw[6];
  char macstr[18];
  uint64_t state;  
};

// Initalise mqtt client
EspMQTTClient client(
  WIFI_SSID,
  WIFI_PASSWORD,
  HOST,  // MQTT Broker server ip
  "",   // Can be omitted if not needed (username)
  "",   // Can be omitted if not needed (password)
  "esp",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);

// Declare global 
struct BinarySensor sensor;

// Called once at startup
void setup()
{
  Serial.begin(9600);

  // Redundant as of right now but probably useful later
  WiFi.macAddress().toCharArray(sensor.macraw, 6);
  sprintf(sensor.macstr, "%x:%x:%x:%x:%x:%x", sensor.macraw[0], sensor.macraw[1], sensor.macraw[2], sensor.macraw[3], sensor.macraw[4], sensor.macraw[5]);

  // Set input pin for reading button state
  pinMode(IN_PIN, INPUT);

  // Optional functionalities of EspMQTTClient
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  // Will be sent if the client does not respond to pings from the broker, pings are only every ~
  client.enableLastWillMessage(SENSOR_ID "/lastwill", "unexpected disconnect");  // You can activate the retain flag by setting the third parameter to true
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
  // Get notified anytime someone sends "ping" to the ping topic
  client.subscribe(SENSOR_ID "/ping" , [](const String & topic, const String & payload) {
    // Respond to "ping" with "pong" 
    if (payload == "ping") client.publish(SENSOR_ID "/ping", "pong");
  });
}

// Called endlessly on repeat (like a loop)
void loop()
{
  // Get button state
  uint64_t currentState = digitalRead(IN_PIN);
  // If it has changed between now and the last loop
  if (sensor.state != currentState) {
    // Publish the new state
    client.publish(SENSOR_ID "/state", (const String) currentState);
    // Update the stored state
    sensor.state = currentState;
  }

  // Necessary for mqtt client 
  client.loop();
}
