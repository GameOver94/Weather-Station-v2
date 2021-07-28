#include <WiFi.h>

#include <EspMQTTClient.h>
#include <ArduinoJson.h>

#include <credentials.h>

const char *device_name = DEVICE_NAME;
const String STAT_TOPIC = "/devices/" + String(device_name) + "/status";
const String STAT_ERROR_TOPIC = "/devices/" + String(device_name) + "/errors";
const String LOG_MEASUREMENT_TOPIC = "/logger/" + String(device_name) + "/measurement";

EspMQTTClient MQTTclient(
    ssid,
    password,
    BROKER_IP,   // MQTT Broker server ip
    device_name, // Client name that uniquely identify your device
    1883         // The MQTT port, default to 1883. this line can be omitted
);

//-------------------------------------------------------------------------------------------------------------------
void sendStatus()
{
  Serial.println();
  bool MQTTstatus = MQTTclient.publish(STAT_TOPIC, "connected", true);

  if (MQTTstatus)
  {
    Serial.println("Publishing status sucessfull.");
  }
  else
  {
    Serial.println("Publishing status failed.");
  }
}

//-------------------------------------------------------------------------------------------------------------------
void sendData(float temp, float hum, float press, float p_r, float bat)
{
  StaticJsonDocument<256> measurement;
  char buffer[256];

  Serial.println("----- send mssage -----");

  measurement["temperature"] = temp;
  measurement["pressure"] = press / 100.0F;
  #ifdef BME280
    measurement["humidity"] = hum;
  #endif
  measurement["reduced pressure"] = p_r;
  measurement["battery voltage"] = bat;

  serializeJson(measurement, buffer);
  bool MQTTstatus = MQTTclient.publish(LOG_MEASUREMENT_TOPIC, buffer);

  if (MQTTstatus)
  {
    Serial.printf("Message send sucessful after %fs \n", millis() / 1000.0);

    shutdown_delay = millis() + 500;
    max_ontime = shutdown_delay + 500;
  }
  else
  {
    Serial.println("Faild to send message.");
  }
}

//-------------------------------------------------------------------------------------------------------------------
void sendSensorError()
{
  bool MQTTstatus = MQTTclient.publish(STAT_ERROR_TOPIC, "Sensor Error");

  if (MQTTstatus)
  {
    Serial.printf("Error Message send sucessful after %fs \n", millis() / 1000.0);
  }
  else
  {
    Serial.println("Faild to send message.");
  }
}