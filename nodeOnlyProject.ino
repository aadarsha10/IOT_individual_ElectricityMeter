#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#define WLAN_SSID    "TP-Link_AMS"
#define WLAN_PASS    "@TP-Login#9893"
#define AIO_SERVER    "io.adafruit.com"
#define AIO_SERVERPORT  1883             //use 8883 for SSL
#define AIO_USERNAME    "A_shrestha10"
#define AIO_KEY          "aio_vdDu52GmiVyC2x84q2hibdHFpzXv"
// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/current");
const int sensorIn = A0;
int mVperAmp = 66; //185mV for 5Amp module , 100mV for 10A , 66mv for 20 & 30 Amp module
double Voltage = 0;
double Vp = 0;
double Vrms = 0;
double Irms = 0;
// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();
void setup() {
  Serial.begin(9600);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("connecting....");
    delay(1000);
  }
  Serial.println("Connected");
}
void loop() {
  MQTT_connect();
  Voltage = getVPP();
    Vrms = (Voltage / 2.0) * 0.707; // sq root
  Irms = ((Vrms * 1000) / mVperAmp) ;
  Serial.print(Irms);
  Serial.println(" Amps");
  if (! photocell.publish(Irms))
  {
    Serial.println("Failed");
  }
  else
  {
    Serial.println("OK!");
  }
  delay(2000);
}
double getVPP()
{
  float result;
  int readValue; //value read from the sensor
  int maxValue = 0; // store max value here
  int minValue = 1024; // store min value here
  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000) //sample for 1 Sec
  {
    readValue = analogRead(sensorIn);
    // see if you have a new maxValue
    if (readValue > maxValue)
    {
      /*record the menimum sensor value*/
      maxValue = readValue;
    }
    if (readValue < minValue)
    {
      /*record the menimum sensor value*/
      minValue = readValue;
    }
  }
  // Subtract min from max
  result = ((maxValue - minValue) * 5) / 1024.0;
  return result;
}
void MQTT_connect() {
  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected()) {
    Serial.println("MQTT_Already_connected");
    return;
  }
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset 
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}
