#include <WiFi.h>
#include <time.h>
#include <sys/time.h>

#include "EspMQTTClient.h"
#include <ArduinoJson.h>

#include <credentials.h>

const char NTP_SERVER[] = "at.pool.ntp.org";
const char TZ_INFO[] = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00"; // See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
const char daysOfTheWeek[7][11] = {"Sonntag", "Mondtag", "Dienstag", "Mitwoch", "Donnerstag", "Freitag", "Samstag"};


const char* device_name = "WeatherStation_9ae54469";
const String STAT_TOPIC = "/devices/" + String(device_name) + "/status";
const String LOG_MEASUREMENT_TOPIC = "/logger/" + String(device_name) + "/measurement";

EspMQTTClient MQTTclient(
  ssid,
  password,
  "192.168.1.14",  // MQTT Broker server ip
  device_name,     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);


void setNTP()
{
  tm timeinfo;

  Serial.println("----- set NTP time -----");

  configTzTime(TZ_INFO, NTP_SERVER);
  getLocalTime(&timeinfo, 60000);
  Serial.println(&timeinfo, "Datum: %d.%m.%y  Zeit: %H:%M:%S");
}


void sendStatus()
{
  bool MQTTstatus = MQTTclient.publish(STAT_TOPIC, "connected", true);
  Serial.print("Publishing Status sucessfull: ");
  Serial.println(MQTTstatus);
}

void sendData(float temp, float hum, float press, float p_r, float bat)
{
  StaticJsonDocument<256> measurement;
  char buffer[256];

  Serial.println("----- send mssage -----");

  measurement["temperature"] = temp;
  measurement["humidity"] = hum;
  measurement["pressure"] = press / 100.0F;
  measurement["reduced pressure"] = p_r;
  measurement["battery voltage"] = bat;

  serializeJson(measurement, buffer);
  bool MQTTmessage = MQTTclient.publish(LOG_MEASUREMENT_TOPIC, buffer);

   if (MQTTmessage)
  {
    Serial.print("Message send sucessful.");
  }
  else
  {
    Serial.println("Faild to send message.");
  }
}