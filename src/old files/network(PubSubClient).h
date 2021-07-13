#include <WiFi.h>
#include <time.h>
#include <sys/time.h>

#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <credentials.h>

const char NTP_SERVER[] = "at.pool.ntp.org";
const char TZ_INFO[] = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00"; // See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
const char daysOfTheWeek[7][11] = {"Sonntag", "Mondtag", "Dienstag", "Mitwoch", "Donnerstag", "Freitag", "Samstag"};

const char *mqtt_server = "broker.hivemq.com";
const String device_name = "WeatherStation_9ae544a6";

String STAT_TOPIC = "/devices/" + device_name + "/status";
String LOG_MEASUREMENT_TOPIC = "/logger/" + device_name + "/measurement";

WiFiClient espClient;
PubSubClient MQTTclient(espClient);



bool connectWiFi()
{
  Serial.println("----- connnect to WifFi -----");
  WiFi.begin(ssid, password);

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
    if (++counter > 60)
      break;

    Serial.print(".");
  }
  Serial.println(".");

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("WiFi connencted");
    return true;
  }
  else
  {
    Serial.println("WiFi connection faild");
    return false;
  }
}



void setNTP()
{
  tm timeinfo;

  Serial.println("----- set NTP time -----");

  configTzTime(TZ_INFO, NTP_SERVER);
  getLocalTime(&timeinfo, 60000);
  Serial.println(&timeinfo, "Datum: %d.%m.%y  Zeit: %H:%M:%S");
}



bool connectMQTT()
{
  Serial.println("----- connnect to MQTT server -----");
  MQTTclient.setServer(mqtt_server, 1883);
  MQTTclient.setKeepAlive(330);
  MQTTclient.setBufferSize(512);

  Serial.println("Attempting MQTT connection...");
  int counter = 0;

  while (!MQTTclient.connected())
  {
    // Attempt to connect
    //if (MQTTclient.connect(device_name.c_str(), STAT_TOPIC.c_str(), 1, 1, "disconnected"))
    if (MQTTclient.connect(device_name.c_str()))
    {
      Serial.println("connected");
      MQTTclient.publish(STAT_TOPIC.c_str(), "connected", true);
      return true;
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.println(MQTTclient.state());
      delay(200);

      if (++counter >= 3)
      {
        break;
      }
    }
  }
  return false;
}



void sendData(float temp, float hum, float press, float p_r, float batt) {
  StaticJsonDocument <256> measurement;
  char buffer[128];

  measurement["temp"] = temp;
  measurement["hum"] = hum;
  measurement["pres"] = press/100.0F;
  measurement["p_r"] = p_r;
  measurement["bat"] = batt;

  size_t n = serializeJson(measurement, buffer);

  bool messageSend = MQTTclient.publish(LOG_MEASUREMENT_TOPIC.c_str(), buffer, n);
  //Serial.println(buffer);
  //Serial.print("Message send sucessful: ");
  Serial.println(messageSend);
}