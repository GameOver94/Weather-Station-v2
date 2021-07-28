# Weather Station v2

This project uses a [TTGO-T-Energy](https://github.com/LilyGO/LILYGO-T-Energy) and an BME280 / BMP280 to record environtmental Data and send them over MQTT. This project focuses on low energy consumption and I get about one month out of a single 18650. I process the data with NodeRed, store it in a influxdb and dispay it with grafana.

In the platformio.ini under build_flags you can select to comple the project for the BME280 or the BMP280.