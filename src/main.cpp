#include <Arduino.h>
#include "Wire.h"
#include <WiFi.h>
#include <Wire.h>
#include <BH1750.h>
#include <Ticker.h>
#include <DHTesp.h>
#include <PubSubClient.h>
 #include <Adafruit_GFX.h>
// for oled

// for mqtt
#define WIFI_SSID "LUMBALUMBA 4G"
#define WIFI_PASSWORD "Kupukupu"

#define MQTT_BROKER  "broker.emqx.io"
#define MQTT_TOPIC_PUBLISH   "esp32_iot/data"
#define MQTT_TOPIC_LUX "esp32_iot/LUX"
#define MQTT_TOPIC_SUBSCRIBE "esp32_iot/cmd"  

#define DHT_PIN 18
#define DHT_TYPE DHTesp::DHT11

BH1750 lightMeter ;
DHTesp dht;

WiFiClient espClient;

PubSubClient mqtt(espClient);
char g_szDeviceId[30];
char szData[999]; //char array to store data published to mqtt


float temp, hum, lux; // declare variable for sensor data
String StatusDisplay;  // mqtt connection status display
//xyz = 241

void humRead(void *pvParameters){ //Y
  while(1){

   hum = dht.getHumidity();
    
  vTaskDelay(4000 / portTICK_PERIOD_MS);}
   //}
  
}
void tempRead(void *pvParameters){//X

// using debounce to prevent data not showing
   int failedCount = 0;
  const int maxFailedCount = 5; // maximum number of consecutive failures before setting temp to NaN
  while(1) {
    float tempRead = dht.getTemperature();
    if(!isnan(tempRead)) { // check if data was successfully read
      temp = tempRead; // update temp if data was successfully read
      failedCount = 0; // reset failure count
    }
    else {
      failedCount++; // increment failure count
      if (failedCount >= maxFailedCount) {
        temp = NAN; // set temp to NaN if there have been too many consecutive failures
        failedCount = 0; // reset failure count
      }
    }
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  }}


void luxRead(void *pvParameters){//Z

  while(1){
  lux = lightMeter.readLightLevel();
  if(lux >=400 ){

     sprintf(szData,"BOX CONDITION: WARNING,2501994241 Lux Data %.2f lx\n",lux);
      mqtt.publish(MQTT_TOPIC_LUX, szData);
  }
  else
  { sprintf(szData,"BOX CONDITION: CLOSED,2501994241 Lux Data %.2f lx\n",lux);
      mqtt.publish(MQTT_TOPIC_LUX, szData);
   // to publish data to Topic Lux
   }


  vTaskDelay(1000 / portTICK_PERIOD_MS);}
}
void serialDisplay(void *pvParameters){
  while (1) {
 Serial.printf("Serial Monitor : Lux = %.2f lx || Temperature  : %.2f | Humidity : %.2f\n", lux, temp, hum);

  vTaskDelay(1000 / portTICK_PERIOD_MS);}
   //
}

void WifiConnect(void *pvParameters){ // connect to wifi
   
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
   
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart(); // if not connect, restart
  }  
  Serial.print("System connected with IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("RSSI: %d\n", WiFi.RSSI());
  vTaskDelete(NULL); // deleting the task
}


void msgRecieve(char* topSub,byte* payload, unsigned int len){

  Serial.print("Message arrived [");
  Serial.print(topSub);
  Serial.print("]: ");
  Serial.write(payload, len);
  Serial.println();
  delay(1000);
  
}// recieve message from mqtt broker

boolean connectionStatus = false;
void mqttConnect(void *pvParameters){
  
  Serial.println("Connecting to MQTT Broker...");
  Serial.print(MQTT_BROKER);
  Serial.print(g_szDeviceId);

    mqtt.setCallback(msgRecieve);


  for(int i=0; i<3 && !connectionStatus; i++){
    StatusDisplay = "connecting...";
    connectionStatus = mqtt.connect(g_szDeviceId); // to set it to true
    delay(5000);
    if (!connectionStatus){ // if not connected, keep try to reconnect
      StatusDisplay = "failed";

      Serial.print("connection failed, Rebooting \n");
    }
    else if (connectionStatus)
  {

    StatusDisplay = "connected";
    Serial.println("connected");
    Serial.print("Connected to :");
    Serial.print(MQTT_TOPIC_LUX);


    mqtt.subscribe(MQTT_TOPIC_LUX);
    mqtt.connected(); // to tell that the mqtt is connected 
   vTaskDelete(NULL); // deleting the task after connect
  }
 
  } 

vTaskDelay(2000 / portTICK_PERIOD_MS);

}
///////////////////////////////

void setup() {
Serial.begin(115200);
mqtt.setServer(MQTT_BROKER, 1883);
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  Wire.begin();
    lightMeter.begin();
  dht.setup(DHT_PIN, DHT_TYPE);

  xTaskCreatePinnedToCore(humRead, "sensorRead", 2044, NULL, 1, NULL, 0);
 xTaskCreatePinnedToCore(tempRead, "Temperature", 2044, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(luxRead, "luxRead", 2044, NULL, 1, NULL, 0);
 xTaskCreatePinnedToCore(serialDisplay, "sensorDisplay", 2044, NULL, 1, NULL, 0);
xTaskCreatePinnedToCore(WifiConnect, "WifiConnect", 4048, NULL, 1, NULL, 0);

//mqtt
  xTaskCreatePinnedToCore(mqttConnect, "mqttConnect", 4048, NULL, 1, NULL, 0);
 



}
void loop() {
   
 mqtt.loop();


}