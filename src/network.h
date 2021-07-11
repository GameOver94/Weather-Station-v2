#include <WiFi.h>
#include <time.h>
#include <sys/time.h>

#include <AsyncMqttClient.h>
#include <ArduinoJson.h>

#include <credentials.h>

const char NTP_SERVER[] = "at.pool.ntp.org";
const char TZ_INFO[] = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00"; // See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
const char daysOfTheWeek[7][11] = {"Sonntag", "Mondtag", "Dienstag", "Mitwoch", "Donnerstag", "Freitag", "Samstag"};

//const char *mqtt_server = "test.mosquitto.org";
#define MQTT_HOST IPAddress(192, 168, 1, 14)
const char *device_name = "WeatherStation_9ae544a6";

String STAT_TOPIC = "/devices/" + String(device_name) + "/status";
String LOG_MEASUREMENT_TOPIC = "/logger/" + String(device_name) + "/measurement";

WiFiClient espClient;
AsyncMqttClient mqttClient;

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

void onMqttConnect(bool sessionPresent)
{
  Serial.println();
  Serial.println("connected");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  
  uint16_t packetIdPub1 = mqttClient.publish(STAT_TOPIC.c_str(), 1, true, "connected");
  Serial.print("Publishing Status at QoS 1, packetId: ");
  Serial.println(packetIdPub1);
}

bool connectMQTT()
{
  Serial.println("----- connnect to MQTT server -----");
  //mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setServer(MQTT_HOST, 1883);
  mqttClient.setClientId(device_name);
  mqttClient.setKeepAlive(330);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.connect();

  int counter = 0;

  while (!mqttClient.connected())
  {
    Serial.print(".");

    if (++counter >= 50)
    {
      Serial.println();
      Serial.println("MQTT connection failed");
      return false;
    }

    delay(200);
  }
  return true;
}


void sendData(float temp, float hum, float press, float p_r, float bat)
{
  StaticJsonDocument<128> measurement;
  char buffer[128];

  Serial.println("----- send mssage -----");

  measurement["temperature"] = temp;
  measurement["humidity"] = hum;
  measurement["pressure"] = press / 100.0F;
  measurement["reduced pressure"] = p_r;
  measurement["battery voltage"] = bat;

  serializeJson(measurement, buffer);
  uint16_t messageID = mqttClient.publish(LOG_MEASUREMENT_TOPIC.c_str(), 2, false, buffer);

   if (messageID > 0)
  {
    Serial.print("Message send sucessful. ID: ");
    Serial.println(messageID);
  }
  else
  {
    Serial.println("Faild to send message.");
  }

  delay(200);
  delay(200);
  delay(200);
  delay(200);
  delay(200);

  delay(200);
  delay(200);
  delay(200);
  delay(200);
  delay(200);

  delay(200);
  delay(200);
  delay(200);
  delay(200);
  delay(200);
}