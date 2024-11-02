/*
  ArduinoMqttClient - WiFi Simple Sender

  This example connects to a MQTT broker and publishes a message to
  a topic once in 10 seconds.

  The circuit:
  - Arduino MKR 1000, MKR 1010 or Uno WiFi Rev.2 board

  This example code is in the public domain.
  021124 18:33
*/

#include <ArduinoMqttClient.h>
#include <ESP8266WiFi.h>
#include "arduino_secrets.h"

#include <ArduinoJson.h>

/* BM688 Stuff*/
#include "bsec.h"
/*bsec zeugs*/


///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[]   = SECRET_SSID;        // your network SSID (name)
char pass[]   = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
char user[]   = SECRET_USER;
char passwd[] = SECRET_PASSWD;

// To connect with SSL/TLS:
// 1) Change WiFiClient to WiFiSSLClient.
// 2) Change port value from 1883 to 8883.
// 3) Change broker value to a server with a known SSL/TLS root certificate 
//    flashed in the WiFi module.

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "192.168.2.60";
int        port     = 1883;
const char topic[]  = "Airquality";

const long interval = 10000;
unsigned long previousMillis = 0;

int count = 0;

// functions declarations
void reconnect(void);

/*bsec zeugs*/
// Helper functions declarations
void checkIaqSensorStatus(void);
void errLeds(void);

// Create an object of the class Bsec
Bsec iaqSensor;
String output;
/*bsec zeugs*/

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    delay(1) ; // wait for serial port to connect. Needed for native USB port only
  }

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // You can provide a unique client ID, if not set the library uses Arduino-millis()
  // Each client must have a unique client ID
  mqttClient.setId("ESP2866_BM688");

  // You can provide a username and password for authentication
  mqttClient.setUsernamePassword("Werner_Holt", "die Abenteuer");
  mqttClient.setUsernamePassword(user, passwd);

  reconnect();
/*bsec zeugs*/
    pinMode(LED_BUILTIN, OUTPUT);
  iaqSensor.begin(BME68X_I2C_ADDR_HIGH, Wire);
  output = "\nBSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
  Serial.println(output);
  checkIaqSensorStatus();

  bsec_virtual_sensor_t sensorList[13] = {
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_STABILIZATION_STATUS,
    BSEC_OUTPUT_RUN_IN_STATUS,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    BSEC_OUTPUT_GAS_PERCENTAGE
  };

  iaqSensor.updateSubscription(sensorList, 13, BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();

  // Print the header
  output = "Timestamp [ms], IAQ, IAQ accuracy, Static IAQ, CO2 equivalent, breath VOC equivalent, raw temp[°C], pressure [hPa], raw relative humidity [%], gas [Ohm], Stab Status, run in status, comp temp[°C], comp humidity [%], gas percentage";
  Serial.println(output);
/*bsec zeugs*/  
}

void loop() {
  digitalWrite(LED_BUILTIN, LOW);
   DynamicJsonDocument doc(1024);
  doc["deviceId"] = "ESP2866";
  doc["siteId"] = "BME688";
  doc["iaq"] = iaqSensor.iaq;
  doc["iaqAccuracy"] = iaqSensor.iaqAccuracy;
  doc["staticIaq"] = iaqSensor.staticIaq;
  doc["4"] = iaqSensor.co2Equivalent;
  doc["5"] = iaqSensor.breathVocEquivalent;
  doc["6"] = iaqSensor.rawTemperature;
  doc["pressure"] = iaqSensor.pressure;
  doc["8"] = iaqSensor.rawHumidity;
  doc["9"] = iaqSensor.gasResistance;
  doc["10"] = iaqSensor.stabStatus;
  doc["11"] = iaqSensor.runInStatus;
  doc["temperature"] = iaqSensor.temperature;
  doc["humidity"] = iaqSensor.humidity;
  doc["14"] = iaqSensor.gasPercentage;

  

  char mqtt_message[384];
  serializeJson(doc, mqtt_message);
// publishMessage("Topic", "message", true);
//  publishMessage("esp8266_data", mqtt_message, true);
  // put your main code here, to run repeatedly:
  unsigned long time_trigger = millis();
  if (iaqSensor.run()) { // If new data is available
    output = String(time_trigger);
    output += ", " + String(iaqSensor.iaq);
    output += ", " + String(iaqSensor.iaqAccuracy);
    output += ", " + String(iaqSensor.staticIaq);
    output += ", " + String(iaqSensor.co2Equivalent);
    output += ", " + String(iaqSensor.breathVocEquivalent);
    output += ", " + String(iaqSensor.rawTemperature);
    output += ", " + String(iaqSensor.pressure);
    output += ", " + String(iaqSensor.rawHumidity);
    output += ", " + String(iaqSensor.gasResistance);
    output += ", " + String(iaqSensor.stabStatus);
    output += ", " + String(iaqSensor.runInStatus);
    output += ", " + String(iaqSensor.temperature);
    output += ", " + String(iaqSensor.humidity);
    output += ", " + String(iaqSensor.gasPercentage);
    Serial.println(output);
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    checkIaqSensorStatus();
    
  }
  delay(5000);  
  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();

  // avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
  // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;
    // digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Sending message to topic: ");
    Serial.println(topic);
    Serial.print("hello ");
    Serial.println(count);
    // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage(topic);
   // mqttClient.print("hello ");
   // mqttClient.print(count);
  //  mqttClient.print(output);
    mqttClient.print(mqtt_message);
    mqttClient.endMessage();
    Serial.println();
    count++;
    }
}

  void reconnect() {
  while (mqttClient.connected()==0) {
    // You can provide a unique client ID, if not set the library uses Arduino-millis()
    // Each client must have a unique client ID
    mqttClient.setId("ESP2866_BM688");
    Serial.print("Attempting to connect to the MQTT broker: ");
    Serial.println(broker);
    if (!mqttClient.connect(broker, port)) {
      Serial.print("MQTT connection failed! Error code = ");
      Serial.println(mqttClient.connectError());
      //  Serial.print("failed, rc=");
      Serial.println(" try again in 5 seconds");
      delay(5000);
      }
    else {
      Serial.println("Connected to the MQTT broker!");
      Serial.println();
      Serial.println("connected so subscribe");
      mqttClient.subscribe(topic);
      }
    }//end while
  } //end function

  // Helper function definitions
void checkIaqSensorStatus(void)
{
  if (iaqSensor.bsecStatus != BSEC_OK) {
    if (iaqSensor.bsecStatus < BSEC_OK) {
      output = "BSEC error code : " + String(iaqSensor.bsecStatus);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      output = "BSEC warning code : " + String(iaqSensor.bsecStatus);
      Serial.println(output);
    }
  } 

  if (iaqSensor.bme68xStatus != BME68X_OK) {
    if (iaqSensor.bme68xStatus < BME68X_OK) {
      output = "BME68X error code : " + String(iaqSensor.bme68xStatus);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      output = "BME68X warning code : " + String(iaqSensor.bme68xStatus);
      Serial.println(output);
    }
  }
}

void errLeds(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}
