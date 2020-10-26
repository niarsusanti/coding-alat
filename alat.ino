#include <PZEM004Tv30.h>
PZEM004Tv30 pzem(D5, D6);
const int RELAY_PIN = D1;
#include <EasyNTPClient.h>
#include <FS.h> 
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
//#include <SPI.h>
//#include "MFRC522.h"
#include <PubSubClient.h>
#include "config.h"
//var temp untuk menampung data json dari payload
char payloadTemp[200];

//var untuk PIN sensor
int soil_sensor= A0;
int powerPin = 15;
int Status;

WiFiUDP udp;
EasyNTPClient ntpClient(udp, "pool.ntp.org", 25200); 
StaticJsonBuffer<200> jsonBuffer;
//var untuk userID
String commandMessage1 = "1"; //message pesan subscribe
String commandMessage2 = "2"; //message pesan publish
String typeDevice = "Soil Sensor";
char routingkey1[40] = "SollarCelldata";
char routingkey2[40] = "SollarCell";
int relay1 = D1 ;
int relay2 = D2 ;
int relay3 = D3 ;
int relay4 = D4 ;
void setup_wifi() {
  WiFi.macAddress(MAC_array);
  for (int i = 0; i < sizeof(MAC_array) - 1; ++i) {
    sprintf(MAC_char, "%s%02x:", MAC_char, MAC_array[i]);
  }
  sprintf(MAC_char, "%s%02x", MAC_char, MAC_array[sizeof(MAC_array) - 1]);
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  String(mqtt_port).toCharArray(smqtt_port, 5);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", smqtt_port, 5);
  WiFiManagerParameter custom_mqtt_user("user", "mqtt user", mqtt_user, 40);
  WiFiManagerParameter custom_mqtt_password("password", "mqtt password", mqtt_password, 40);
  WiFiManagerParameter custom_mqtt_keywords1("keyword1", "mqtt keyword1", MAC_char, 40);
  //WiFiManagerParameter custom_mqtt_keywords2("keyword2", "mqtt keyword2", mqtt_keywords2, 40);
  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter( & custom_mqtt_server);
  wifiManager.addParameter( & custom_mqtt_port);
  wifiManager.addParameter( & custom_mqtt_user);
  wifiManager.addParameter( & custom_mqtt_password);
  wifiManager.addParameter(&custom_mqtt_keywords1);
  // wifiManager.addParameter( & custom_mqtt_keywords2);
  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoCon nectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect(MAC_char, "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(2000);
    //reset and try again, or maybe put it to deep sleep
//    ESP.reset();
    delay(2000);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


//calback
void callback(char * topic, byte * payload, unsigned int length) {
  char message [7] ;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  String convertMsg = String(message) ;
  String data = convertMsg.substring(5);
  int timer = data.toInt();

  //relay 1
  if (message[0] == '1') {
    digitalWrite(relay1, HIGH);
    Status='1';
    Serial.println("relay 1 mati");
    Serial.println(Status);
  } else if (message[0] == '0') {
    Status='0';
    digitalWrite(relay1, LOW); 
    Serial.println("relay 1 idup");
    Serial.println(Status);
  }
  //relay 2
  if (message[1] == '1') {
    digitalWrite(relay2, HIGH);
    Serial.println("relay 2 mati");
  } else {
    digitalWrite(relay2, LOW);
    Serial.println("relay 2 idup");
  }

  //relay 3
  if (message[2] == '1') {
    digitalWrite(relay3, HIGH);
    Serial.println("relay 3 mati");
  } else {
    digitalWrite(relay3, LOW);
    Serial.println("relay 3 idup");

  }

  //relay 4
  if (message[3] == '1') {
    digitalWrite(relay4, HIGH);
    Serial.println("relay 4 mati");
  } else {
    digitalWrite(relay4, LOW);
    Serial.println("relay 4 idup");

  }

  delay (timer);
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);
  Serial.println("relay mati");
 }
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...%s");
    Serial.println(mqtt_server);
    // Attempt to connect
    if (client.connect(MAC_char, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      Serial.println(MAC_char);
      client.subscribe(MAC_char);
//      client.subscribe(MAC_char+1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      if (client.state() == 4) ESP.restart();
      else {
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        //delay(10000);
      }
    }
  }
}

void setup() {
  //setup pin mode
  //  pinMode(soilSensor, INPUT_PULLUP);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(powerPin, OUTPUT);
//  digitalWrite(D1, HIGH);
//  digitalWrite(D2, HIGH);
//  digitalWrite(D3, HIGH);
//  digitalWrite(D4, HIGH);

  Serial.begin(9600);
//  EEPROM.begin(512);
  Serial.println(F("Booting...."));
  //read config wifi,mqtt dan yang lain
  ReadConfigFile();
  setup_wifi();
  SaveConfigFile();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(1000);
  sensor();
  Serial.println(Status);
  if(Status== '0'){
    digitalWrite(relay1, LOW);
    Serial.println("relay 1 idup");
//    Serial.println(Status);
    }else if(Status== '1'){
      digitalWrite(relay1, HIGH);
      Serial.println("relay 1 Mati");
//      Serial.println(Status);
    }
//    else{
//    digitalWrite(relay1, LOW);
//    Serial.println("relay 1 idup");
//    Serial.println(Status);
//      }
}

void sensor(){
 String postData, volt, amper, watt, kwh, frequency, powerfactor;
  float  v = pzem.voltage();
  float  a = pzem.current();
  float  w = pzem.power();
  float  kw = pzem.energy();
  float f = pzem.frequency();
  float pft = pzem.pf();
  volt = String(v);
  amper = String(a);
  watt = String(w);
  kwh = String(kw);
  frequency = String(f);
  powerfactor = String(pft);
  float unixtime = ntpClient.getUnixTime();
  Serial.println(unixtime);
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sn"] = MAC_char;
  root["volt"] =volt;
  root["ampere"] =amper;
  root["watt"] =watt;
  root["kwh"] =kwh;
  root["Status"] =Status;
//  root["sn"] = MAC_char;
//  root["volt"] ="220.10";
//  root["ampere"] ="32.10";
//  root["watt"] ="20.10";
//  root["kwh"] ="18.10";
  String pubmsg;
  root.printTo(pubmsg);
  Serial.println(pubmsg);
  Serial.println(MAC_char);
  client.publish(routingkey1, pubmsg.c_str());
//  delay(5000)
  client.publish(routingkey2, pubmsg.c_str());
  }
