/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface. The device's I2C address is either 0x76 or 0x77.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
  See the LICENSE file for details.
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>

#include <network.h> // handling of WiFi and NTP
#include <sensor.h>  // handling of de BME680

#include <driver/adc.h>
#include <cmath>

#define uS_TO_S_FACTOR 1000000 //Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP 300      //Time ESP32 will go to sleep (in seconds)

// Globale Variabeln RTC RAM
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int errorCount = 0;

// Globale Variablen

void print_wakeup_reason();
float batery_level();
void error_hander();
void setClockManually();
void readClock();

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ; // time to get serial running
  Serial.println();
  Serial.println("---- Weather Station v2 ----");

  // first boot

  if (bootCount == 0)
  {
    bool WiFi_status = connectWiFi();
    if (WiFi_status)
    {
      Serial.println();
      setNTP();
    }
    else
    {
      ESP.restart();
    }
  }

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  // connect to MQTT Server
  Serial.println();
  bool MQTT_status = false;

  if (bootCount > 0)
  {
    bool WiFi_status = connectWiFi();
    if (WiFi_status)
    {
      Serial.println();
      MQTT_status = connectMQTT();
    }
    else
    {
      ++errorCount;
      error_hander();
    }

    if (MQTT_status)
    {
      // Get Sensor Data
      float temp(NAN), hum(NAN), pres(NAN), p_r(NAN);

      Serial.println();
      bool Sensor_status = setupSensor();
      if (Sensor_status)
      {
        getSensorData(temp, hum, pres, p_r);
        float bat = batery_level();
        printSensorValues(temp, hum, pres, p_r);

        Serial.println();
        sendData(temp, hum, pres, p_r, bat);
        
      }
    }
    else
    {
      ++errorCount;
      error_hander();
    }
  }

  mqttClient.disconnect();
  WiFi.disconnect();
  delay(200);

  // Print current Time
  Serial.println();
  readClock();

  //Set timer to 120 seconds
  Serial.println();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");

  //Go to sleep now
  errorCount = 0;
  esp_deep_sleep_start();
}

void loop()
{
}

//Function that prints the reason by which ESP32 has been awaken from sleep
void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  default:
    Serial.println("Wakeup was not caused by deep sleep.");
    break;
  }
}

void setClockManually()
{

  // Für weitere Details der Lösung siehe:
  // https://github.com/esp8266/Arduino/blob/master/libraries/esp8266/examples/NTP-TZ-DST/NTP-TZ-DST.ino

#define RTC_UTC_TEST 1510592825 // 1510592825 = Monday 13 November 2017 17:07:05 UTC

  time_t rtc = RTC_UTC_TEST;
  timeval tv = {rtc, 0};
  settimeofday(&tv, nullptr);
}

void readClock()
{
  tm timeinfo;

  configTzTime(TZ_INFO, NTP_SERVER);
  getLocalTime(&timeinfo);

  Serial.println("---- Current Time ----");
  //Serial.println(&timeinfo, "Datum: %d.%m.%y  Zeit: %H:%M:%S");
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

float batery_level()
{

  analogSetAttenuation(ADC_11db);

  float measurement = analogRead(35);
  delay(20);
  measurement += analogRead(35);
  delay(20);
  measurement += analogRead(35);
  measurement /= 3;

  float battery_voltage = (measurement / 4095.0) * 1.0 * 3.55 * 2; // Vref * attenuation * voltage divider

  Serial.print("Battery Voltage: ");
  Serial.println(battery_voltage);
  Serial.print("ADC Count: ");
  Serial.println(measurement);

  return battery_voltage;
}

void error_hander()
{

  Serial.println();
  Serial.println("---- Error Occured ----");
  if (errorCount < 8)
  {
    long sleep_time = 60ULL * pow(errorCount,2);
    esp_sleep_enable_timer_wakeup(sleep_time * uS_TO_S_FACTOR);
    Serial.println("Setup ESP32 to sleep for every " + String(sleep_time) + " Seconds");
  }
  else
  {
    esp_sleep_enable_timer_wakeup(3600ULL * uS_TO_S_FACTOR);
    Serial.println("Setup ESP32 to sleep for every 3600 Seconds");
  }
  esp_deep_sleep_start();
}