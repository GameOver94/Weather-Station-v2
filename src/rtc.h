#include <time.h>
#include <sys/time.h>

const char NTP_SERVER[] = "at.pool.ntp.org";
const char TZ_INFO[] = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00"; // See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
const char daysOfTheWeek[7][11] = {"Sonntag", "Mondtag", "Dienstag", "Mitwoch", "Donnerstag", "Freitag", "Samstag"};

//-------------------------------------------------------------------------------------------------------------------
void setClockManually()
{

    // Für weitere Details der Lösung siehe:
    // https://github.com/esp8266/Arduino/blob/master/libraries/esp8266/examples/NTP-TZ-DST/NTP-TZ-DST.ino

#define RTC_UTC_TEST 1510592825 // 1510592825 = Monday 13 November 2017 17:07:05 UTC

    time_t rtc = RTC_UTC_TEST;
    timeval tv = {rtc, 0};
    settimeofday(&tv, nullptr);
}

//-------------------------------------------------------------------------------------------------------------------
void setNTP()
{
    tm timeinfo;

    Serial.println("----- set NTP time -----");

    configTzTime(TZ_INFO, NTP_SERVER);
    getLocalTime(&timeinfo, 60000);
    Serial.println(&timeinfo, "Datum: %d.%m.%y  Zeit: %H:%M:%S");
}

//-------------------------------------------------------------------------------------------------------------------
void readClock(int bootCount)
{
    if (bootCount == 1)
    {
        return;
    }

    tm timeinfo;

    configTzTime(TZ_INFO, NTP_SERVER);
    getLocalTime(&timeinfo);

    Serial.println();
    Serial.println("---- Current Time ----");
    //Serial.println(&timeinfo, "Datum: %d.%m.%y  Zeit: %H:%M:%S");
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}