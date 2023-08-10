#include <dsmr_timestamp.hpp>
#include <nvs_flash.h>
#include <esp_log.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <ctime> // Include this header for standard time functions

static int32_t lastTimestamp = TIME_UNKNOWN;       // Initialize to a default value
static bool initializedLastTimeStamp = false;

int32_t getLastTimestamp(const std::string &nvs_timestamp_key)
{
    if (!initializedLastTimeStamp) {
        // Get baudrate from NVS and store it in RAM
        nvs_handle_t nvsStorage;
        esp_err_t err;
        lastTimestamp = TIME_UNKNOWN;
        err = nvs_open("storage", NVS_READONLY, &nvsStorage);
        if (err != ESP_OK) {
            ESP_LOGD("NVS","Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        } else {
            ESP_LOGD("NVS","Opened");

            ESP_LOGD("NVS","Reading %s value from NVS ... ", nvs_timestamp_key.c_str());

            err = nvs_get_i32(nvsStorage, nvs_timestamp_key.c_str(), &lastTimestamp);
            switch (err) {
                case ESP_OK:
                    ESP_LOGD("NVS","Done\n");
                    ESP_LOGD("NVS", "%s value read: %s\n", nvs_timestamp_key.c_str(), std::to_string(lastTimestamp).c_str());
                    break;
                case ESP_ERR_NVS_NOT_FOUND:
                    ESP_LOGD("NVS","%s value is not initialized yet!\n", nvs_timestamp_key.c_str());
                    lastTimestamp = TIME_UNKNOWN;
                    break;
                default :
                    ESP_LOGD("NVS","Error (%s) reading!\n", esp_err_to_name(err));
                    break;
            }
        
            // Close
            nvs_close(nvsStorage);
        }
        // Set the flag to indicate baudrate has been initialized from NVS
        initializedLastTimeStamp = true;
    }
    return lastTimestamp;
}

void setLastTimestamp(const std::string &nvs_timestamp_key, int32_t newTimestamp) 
{
    // store newTimestamp in nvs if different from stored
    int32_t timestamp_stored = getLastTimestamp(nvs_timestamp_key);
    ESP_LOGI("P1", "P1 baudrate stored: %d; new: %d", timestamp_stored, newTimestamp);
    if(newTimestamp != timestamp_stored)
    {
        // write value to nvs
        nvs_handle_t nvsStorage;
        esp_err_t err;
        err = nvs_open("storage", NVS_READWRITE, &nvsStorage);
        if (err != ESP_OK) {
            ESP_LOGD("NVS","Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        } else {
            ESP_LOGD("NVS","Opened");

            if(newTimestamp == TIME_UNKNOWN)
            {
                err = nvs_erase_key(nvsStorage, nvs_timestamp_key.c_str());
                ESP_LOGD("NVS","The value %ss is removed from nvs\n", nvs_timestamp_key.c_str());
            }
            else
            {
                err = nvs_set_i32(nvsStorage, nvs_timestamp_key.c_str(), newTimestamp);                 
                ESP_LOGD("NVS","For key %s value % is stored in nvs\n", nvs_timestamp_key.c_str(), newTimestamp);
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
    lastTimestamp = newTimestamp;
    // Set the flag to indicate baudrate has been initialized in RAM
    initializedLastTimeStamp = true;
}

time_t parseDsmrTimestamp(const std::string &nvs_timestamp_key, const std::string& timestampStr, time_t currentDeviceTime) {
  const char* timeZone = "CET-1CEST,M3.5.0/2,M10.5.0/3"; // Europe/Amsterdam time zone
  time_t unixTime = TIME_UNKNOWN;
  // Parse input timestamp string
  ESP_LOGD("parseDsmrTimestamp","dsmr timestamp: %s; current device Unix time: %ld", timestampStr.c_str(), currentDeviceTime);

  std::string truncatedStr = timestampStr.substr(0, 12);

  // Convert the truncated timestamp string to a time structure
  struct tm timeStruct;
  if (sscanf(truncatedStr.c_str(), "%2d%2d%2d%2d%2d%2d", &timeStruct.tm_year, &timeStruct.tm_mon,
              &timeStruct.tm_mday, &timeStruct.tm_hour, &timeStruct.tm_min, &timeStruct.tm_sec) != 6) {
    // Handle invalid timestamp format or parsing error
    ESP_LOGE("parseDsmrTimestamp"," invalid timestamp format or parsing error for: %s", truncatedStr.c_str());
    return TIME_UNKNOWN; // Return an error value
  }

  // Convert the year to tm_year format (since tm_year represents years since 1900)
  timeStruct.tm_year += 100; // Assuming the year format is YY (e.g., 23 for 2023)

  // Set the time zone using the provided time zone string
  setenv("TZ", timeZone, 1); // 1 for overwrite
  // Use localtime_r() to check for ambiguity
  localtime_r(&unixTime, &timeStruct);

  // Check for ambiguity in the .dst value
  if (timeStruct.tm_isdst == -1) {
    // Handle ambiguous local time (YYMMDDhhmmss format)

    if (timestampStr.length() == 13) {
        // Check the 'X' character to disambiguate
        char dstFlag = timestampStr[12]; // 'X' character indicating DST status
        if (dstFlag == 'S') {
            timeStruct.tm_isdst = 1;
        } else if (dstFlag == 'W') {
            timeStruct.tm_isdst = 0;
        } else {
            ESP_LOGE("parseDsmrTimestamp", "unexpected last character");
            return TIME_UNKNOWN; // Return an error value
        }
    } else {
      // Handle ambiguous hour & format: YYMMDDhhmmss (DSMR2/DSMR3)
      ESP_LOGE("parseDsmrTimestamp", "Ambiguous local time for: %s", truncatedStr.c_str());

      // Need to look at persistently stored timestamps to decide.
      // In very rare cases, we have to rely even on currentDeviceTime given in parameter
      int lastTimestamp = getLastTimestamp(nvs_timestamp_key);
      if (lastTimestamp == -1 && currentDeviceTime != 0) {
        // Edge case: no lastTimestamp stored yet in nvs; the only place where we use currentDeviceTime
        std::tm* tmCurrentDeviceTimeUTC = std::gmtime(&currentDeviceTime);
        if (tmCurrentDeviceTimeUTC->tm_hour == 0) {
            // The ambiguous hour is the first occurrence of 2-3 am
            timeStruct.tm_isdst = 1;
            if (timeStruct.tm_hour == 2) {
                int timestampInSeconds = timeStruct.tm_hour * 3600 + timeStruct.tm_min * 60 + timeStruct.tm_sec;
                setLastTimestamp(nvs_timestamp_key.c_str(), timestampInSeconds);
            }
        } else {
            timeStruct.tm_isdst = 0;
        }
      } else {
        // Ambiguous hour, set timeStruct.tm_isdst based on the value of lastTimestamp
        int timeInSeconds = timeStruct.tm_hour * 3600 + timeStruct.tm_min * 60 + timeStruct.tm_sec;
        if (timeInSeconds < lastTimestamp) {
            // The ambiguous hour is the second occurrence of 2-3 am, so no DST
            timeStruct.tm_isdst = 0;
        } else {
            timeStruct.tm_isdst = 1;
            int timestampInSeconds = timeStruct.tm_hour * 3600 + timeStruct.tm_min * 60 + timeStruct.tm_sec;
            setLastTimestamp(nvs_timestamp_key.c_str(), timestampInSeconds);
        }
      }
    }
  
  }
  // Convert the struct tm to a Unix timestamp (Unix time)
  unixTime = mktime(&timeStruct);
  // Return the Unix time
  ESP_LOGD("parseDsmrTimestamp","converted Unix time: %ld", unixTime);
  return unixTime;
}

time_t deviceTime()
{
    time_t UTC = time(NULL);
    time(&UTC);
    return UTC;
}