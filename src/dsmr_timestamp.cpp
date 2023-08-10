#include <dsmr_timestamp.hpp>
#include <nvs_flash.h>
#include <esp_log.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <ctime> // Include this header for standard time functions

static int32_t lastLocalTimeMinutes[2] = {TIME_UNKNOWN, TIME_UNKNOWN};   // Initialize to a default values
static bool initializedLastTimeMinutes[2] = {false, false};
static std::string nvs_timestamp_key[2] = {NVS_KEY_LAST_E_TIMESTAMP, NVS_KEY_LAST_G_TIMESTAMP};

int8_t getLastLocalTimeMinutes(const int8_t timestamp_key)
{
    if (!initializedLastTimeMinutes[timestamp_key]) {
        // Get baudrate from NVS and store it in RAM
        nvs_handle_t nvsStorage;
        esp_err_t err;
        lastLocalTimeMinutes[timestamp_key] = TIME_UNKNOWN;
        err = nvs_open("storage", NVS_READONLY, &nvsStorage);
        if (err != ESP_OK) {
            ESP_LOGD("NVS","Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        } else {
            ESP_LOGD("NVS","Opened");

            ESP_LOGD("NVS","Reading %s value from NVS ... ", nvs_timestamp_key[timestamp_key].c_str());

            err = nvs_get_i32(nvsStorage, nvs_timestamp_key[timestamp_key].c_str(), &lastLocalTimeMinutes[timestamp_key]);
            switch (err) {
                case ESP_OK:
                    ESP_LOGD("NVS","Done\n");
                    ESP_LOGD("NVS", "%s value read: %s\n", nvs_timestamp_key[timestamp_key].c_str(), std::to_string(lastLocalTimeMinutes[timestamp_key]).c_str());
                    break;
                case ESP_ERR_NVS_NOT_FOUND:
                    ESP_LOGD("NVS","%s value is not initialized yet!\n", nvs_timestamp_key[timestamp_key].c_str());
                    lastLocalTimeMinutes[timestamp_key] = TIME_UNKNOWN;
                    break;
                default :
                    ESP_LOGD("NVS","Error (%s) reading!\n", esp_err_to_name(err));
                    break;
            }
        
            // Close
            nvs_close(nvsStorage);
        }
        // Set the flag to indicate baudrate has been initialized from NVS
        initializedLastTimeMinutes[timestamp_key] = true;
    }
    return lastLocalTimeMinutes[timestamp_key];
}

void storeLastLocalTimeMinutes(const int8_t timestamp_key, int8_t newLocalTimeMinutes) 
{
    // store newTimestamp in nvs if different from stored
    int8_t lastLocalTimeMinutes_stored = getLastLocalTimeMinutes(timestamp_key);
    ESP_LOGI("P1", "timestamp %s stored: %d; new: %d", nvs_timestamp_key[timestamp_key].c_str(), lastLocalTimeMinutes_stored, newLocalTimeMinutes);
    if(newLocalTimeMinutes != lastLocalTimeMinutes_stored)
    {
        // write value to nvs
        nvs_handle_t nvsStorage;
        esp_err_t err;
        err = nvs_open("storage", NVS_READWRITE, &nvsStorage);
        if (err != ESP_OK) {
            ESP_LOGD("NVS","Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        } else {
            ESP_LOGD("NVS","Opened");

            if(newLocalTimeMinutes == TIME_UNKNOWN)
            {
                err = nvs_erase_key(nvsStorage, nvs_timestamp_key[timestamp_key].c_str());
                ESP_LOGD("NVS","The value %ss is removed from nvs\n", nvs_timestamp_key[timestamp_key].c_str());
            }
            else
            {
                err = nvs_set_i32(nvsStorage, nvs_timestamp_key[timestamp_key].c_str(), newLocalTimeMinutes);                 
                ESP_LOGD("NVS","For key %s value % is stored in nvs\n", nvs_timestamp_key[timestamp_key].c_str(), newLocalTimeMinutes);
            }

            // Commit written value.
            // After setting any values, nvs_commit() must be called to ensure changes are written
            // to flash storage. Implementations may write to storage at other times,
            // but this is not guaranteed.
            ESP_LOGD("NVS", "Committing updates in NVS ... ");
            err = nvs_commit(nvsStorage);
        }
        // Close
        nvs_close(nvsStorage);
    }

    // Update the RAM value
    lastLocalTimeMinutes[timestamp_key] = newLocalTimeMinutes;
    // Set the flag to indicate baudrate has been initialized in RAM
    initializedLastTimeMinutes[timestamp_key] = true;
}

std::string formatTimeISO8601(time_t unixTime) {
    std::tm* timeInfo = std::localtime(&unixTime); // Convert to local time

    std::ostringstream oss;
    oss << std::put_time(timeInfo, "%Y-%m-%dT%H:%M:%S%z");

    return oss.str();
}

time_t parseDsmrTimestamp(int8_t timestamp_key, const std::string& timestampStr, time_t currentDeviceTime) {
    time_t unixTime = TIME_UNKNOWN;
    // Parse input timestamp string
    ESP_LOGD("parseDsmrTimestamp","dsmr timestamp: %s; current device Unix time: %ld", timestampStr.c_str(), currentDeviceTime);

    std::string truncatedStr = timestampStr.substr(0, 12);

    // Convert the truncated timestamp YYMMDDhhmmss string to a time structure
    struct tm time{};
    if (sscanf(truncatedStr.c_str(), "%2d%2d%2d%2d%2d%2d", &time.tm_year, &time.tm_mon, &time.tm_mday, &time.tm_hour, &time.tm_min, &time.tm_sec) != 6) {
        // Handle invalid timestamp format or parsing error
        ESP_LOGE("parseDsmrTimestamp"," invalid timestamp format or parsing error for: %s", truncatedStr.c_str());
        return TIME_UNKNOWN; // Return an error value
    }

    // Convert the year to tm_year format (since tm_year represents years since 1900)
    time.tm_year += 100; // Convert YY [years after 2000] to proper tm_year [years after 1900]
    time.tm_mon -= 1;    // Convert MM [month number] to proper tm_mon format [0-11; 0=Jan, 1=Feb, etc.]


    if (timestampStr.length() == 13) {
        // Check the 'X' character of the YYMMDDhhmmssX DSMR timestamp to disambiguate
        char dstFlag = timestampStr[12]; // 'X' character indicating DST status
        if (dstFlag == 'S') {
            time.tm_isdst = 1;
        } else if (dstFlag == 'W') {
            time.tm_isdst = 0;
        } else {
        ESP_LOGE("parseDsmrTimestamp", "unexpected last character");
        return TIME_UNKNOWN; // Return an error value
        }
    } else {
        // The DSMR timestamp is YYMMDDhhmmss (i.e. from DSMR2 or DSMR3)
        // Since the time object was obtained from the POSIX strptime, the value of tm_isdst is indeterminate
        // and needs to be set explicitly before calling mktime.
        time.tm_isdst = -1; // we don't know whether DST is in effect yet or not
    }

    // TODO: consider moving this to another place
    // Set the time zone using the provided time zone string
    const char* timeZone = "CET-1CEST,M3.5.0/2,M10.5.0/3"; // Europe/Amsterdam time zone
    setenv("TZ", timeZone, 1); // 1 for overwrite
    tzset();

    // Use mktime() to create a Unix timestamp based on the time struct.
    // A negative value of time->tm_isdst causes mktime to attempt to determine if Daylight Saving Time was in effect.
    unixTime = mktime(&time);

    if (unixTime == TIME_UNKNOWN || time.tm_isdst == -1) {
        // Handle ambiguous local time: should be only two hours a year and only for DSMR2/3 timestamps (gas or electric))
        ESP_LOGD("parseDsmrTimestamp", "Ambiguous local time for: %s; time.tm_isdst = %d; unixTime = %ld", timestampStr.c_str(), time.tm_isdst, unixTime);

        int8_t lastLocalTimeMinutes = getLastLocalTimeMinutes(timestamp_key);
        int8_t currentLocalTimeMinutes = time.tm_min;

        // Need to look at persistently stored timestamps to decide.
        // In very rare cases, we have to rely even on currentDeviceTime given in parameter
        if (lastLocalTimeMinutes == TIME_UNKNOWN && currentDeviceTime != 0) {
            // Edge case: no lastTimestamp stored yet in nvs; the only place where we use currentDeviceTime
            ESP_LOGD("parseDsmrTimestamp", "Ambiguous local time for: LastTimeStamp: %d; currentDeviceTime = %ld", getLastLocalTimeMinutes(timestamp_key), currentDeviceTime);
            std::tm* tmCurrentDeviceTimeUTC = std::gmtime(&currentDeviceTime);
            if (tmCurrentDeviceTimeUTC->tm_hour == 0) {
                // Between 0:00-1:00 UTC corresponds to first occurrence of 2:00-3:00 local time: i.e. DST is in effect
                time.tm_isdst = 1;
            } else if (tmCurrentDeviceTimeUTC->tm_hour == 1) {
                // Between 1:00-2:00 UTC corresponds to secondd occurrence of 2:00-3:00 local time, i.e. DST is no longer in effect
                time.tm_isdst = 0;
            } else {
                time.tm_isdst = -1; // we still don't know DST or not
                unixTime = TIME_UNKNOWN;
            }
        } else {
            // Ambiguous local time, set timeStruct.tm_isdst based on the value of lastTimestamp
            if (currentLocalTimeMinutes < lastLocalTimeMinutes) {
                // The ambiguous hour is the second occurrence of 2-3 am, so no DST
                // TODO: this algorithm only works if intervals are < 60 min. Fix for inteval = 60 min!
                // TODO: additional complexity: P1 measurement interval may be X minutes (X <60), while the DSMR2/3 timestamp is always the last hourly measurement
                // TODO: keep a counter of the # of simular measurements? if count > 60 DIV X, then apparently we're in winter time?
                time.tm_isdst = 0;
            } else {
                time.tm_isdst = 1;
            }
            
        }
        storeLastLocalTimeMinutes(timestamp_key, currentLocalTimeMinutes);
        // Try conversion again after handling the ambiguous time
        unixTime = mktime(&time);
    } else {
        // Handle unambiguous local time: make sure LastTimestamp is reset (for next year)
        // NB setLastTimestamp only writes to NVS if it's not already   
        storeLastLocalTimeMinutes(timestamp_key, TIME_UNKNOWN);
    }

    // Return the Unix time
    ESP_LOGD("parseDsmrTimestamp","converted Unix time: %ld = %s", unixTime, formatTimeISO8601(unixTime).c_str());
    return unixTime;
}

time_t deviceTime()
{
    time_t UTC = time(NULL);
    time(&UTC);
    return UTC;
}