#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

/* barometrische Höhenformel */
const float g = 9.80665;
const float R = 287.05;
const float alpha = 0.0065;
const float C_h = 0.12;
const float h = 465;           // Change to your height above seelevel

/* Antonie Parameter */
const float A = 5.20389;
const float B = 1733.926;
const float C = 39.485;

#define SEALEVELPRESSURE_HPA (1013.25)

//global Variables
Adafruit_BME280 bme; // I2C

bool setupSensor() {
  Serial.println("---- Get Sensor Data ----");

  unsigned status;
  status = bme.begin();

  if (!status)
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    return false;
  } else {
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1, // temperature
                    Adafruit_BME280::SAMPLING_X1, // pressure
                    Adafruit_BME280::SAMPLING_X1, // humidity
                    Adafruit_BME280::FILTER_OFF   );
    return true;
  }
}

void printSensorValues(float temp, float hum, float pres, float p_r)
{
  Serial.print("Temperature = ");
  Serial.print(temp);
  Serial.println(" °C");

  Serial.print("Humidity = ");
  Serial.print(hum);
  Serial.println(" %");

  Serial.print("Pressure = ");
  Serial.print(pres/100.0F);
  Serial.println(" hPa");

   Serial.print("reduced Pressure = ");
  Serial.print(p_r);
  Serial.println(" hPa");
}

float compensateAltitude(float temp, float hum, float pres) {
  float p_s(NAN), E(NAN), p_r(NAN);
  p_s = pow(10, A-B/(C+temp));
  E = hum/100* p_s;

  p_r = pres/100 * exp(g*h/(R*(temp+273.15+C_h*E+alpha*h/2)));
  return p_r;
}

void getSensorData(float &temp, float &hum, float &pres, float &p_r)
{
  temp = bme.readTemperature();
  hum = bme.readHumidity();
  pres = bme.readPressure();

  p_r = compensateAltitude(temp, hum, pres);

}



