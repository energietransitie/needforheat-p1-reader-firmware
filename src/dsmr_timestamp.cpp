#include <dsmr_timestamp.hpp>
#include <nvs_flash.h>
#include <esp_log.h>
#include <iomanip>
#include <sstream>
#include <mbedtls/platform_time.h>

static int32_t lastTimestamp = TIME_UNKNOWN;       // Initialize to a default value
static bool initializedLastTimeStamp = false;

int32_t getLastTimestamp()
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

            ESP_LOGD("NVS","Reading %s value from NVS ... ", NVS_KEY_LASTTIMESTAMP);

            err = nvs_get_i32(nvsStorage, NVS_KEY_LASTTIMESTAMP, &lastTimestamp);
            switch (err) {
                case ESP_OK:
                    ESP_LOGD("NVS","Done\n");
                    ESP_LOGD("NVS", "%s value read: %s\n", NVS_KEY_LASTTIMESTAMP, std::to_string(lastTimestamp).c_str());
                    break;
                case ESP_ERR_NVS_NOT_FOUND:
                    ESP_LOGD("NVS","%s value is not initialized yet!\n", NVS_KEY_LASTTIMESTAMP);
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

void setLastTimestamp(int32_t newTimestamp) 
{
    // store newTimestamp in nvs if different from stored
    int32_t timestamp_stored = getLastTimestamp();
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
                err = nvs_erase_key(nvsStorage, NVS_KEY_LASTTIMESTAMP);
                ESP_LOGD("NVS","The value %ss is removed from nvs\n", NVS_KEY_LASTTIMESTAMP);
            }
            else
            {
                err = nvs_set_i32(nvsStorage, NVS_KEY_LASTTIMESTAMP, newTimestamp);                 
                ESP_LOGD("NVS","For key %s value % is stored in nvs\n", NVS_KEY_LASTTIMESTAMP, newTimestamp);
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

time_t parseDsmrTimestamp(const std::string& timestampStr, time_t currentDeviceTime) {
  // Parse input timestamp string
  // Set the time zone using the provided time zone string
  ESP_LOGD("parseDsmrTimestamp","dsmr timestamp: %s; current: %ld", timestampStr.c_str(), currentDeviceTime);

  const char* timeZone = "CET-1CEST,M3.5.0/2,M10.5.0/3";  // Europe/Amsterdam time zone
  mbedtls_platform_gmtime_time_t timeStruct;
  mbedtls_platform_gmtime_init(&timeStruct);

  try {
    // Create a truncated C-string with the first 12 characters (YYMMDDhhmmss)
    char truncatedStr[13];
    strncpy(truncatedStr, timestampStr.c_str(), 12);
    truncatedStr[12] = '\0'; // Null-terminate the truncated C-string

    // Parse the truncated C-string to extract the components
    int year, month, day, hour, minute, second;
    if (sscanf(truncatedStr, "%2d%2d%2d%2d%2d%2d", &year, &month, &day, &hour, &minute, &second) != 6) {
        // Handle parsing error
        ESP_LOGE("parseDsmrTimestamp", "Failed to parse timestamp");
        return TIME_UNKNOWN; // Return an error value
    }

    // Set the parsed values in the timeStruct
    timeStruct.year = year + 2000; // Assuming the timestamp is in YY format
    timeStruct.mon = month;
    timeStruct.day = day;
    timeStruct.hour = hour;
    timeStruct.min = minute;
    timeStruct.sec = second;

    // Use localtime_r() to check for ambiguity
    mbedtls_platform_localtime_r(mbedtls_platform_gmtime(&timeStruct), &timeStruct);

    // Check for ambiguity in the .dst value
    if (timeStruct.dst == -1) {
      // Handle ambiguous local time (YYMMDDhhmmss format)

      if (timestampStr.length() == 13) {
          // Check the 'X' character to disambiguate
          char dstFlag = timestampStr[12]; // 'X' character indicating DST status
          if (dstFlag == 'S') {
              timeStruct.dst = MBEDTLS_PLATFORM_TIME_DST_ON;
          } else if (dstFlag == 'W') {
              timeStruct.dst = MBEDTLS_PLATFORM_TIME_DST_OFF;
          } else {
              ESP_LOGE("parseDsmrTimestamp", "unexpected last character");
              return TIME_UNKNOWN; // Return an error value
          }
      } else {
        // Handle ambiguous hour & format: YYMMDDhhmmss (DSMR2/DSMR3)
        ESP_LOGE("parseDsmrTimestamp", "Ambiguous local time for: %s", truncatedStr);

        // Need to look at persistently stored timestamps to decide.
        // In very rare cases, we have to rely even on currentDeviceTime given in parameter
        int lastTimestamp = getLastTimestamp();
        if (lastTimestamp == -1 && currentDeviceTime != 0) {
            // Edge case: no lastTimestamp stored yet in nvs; the only place where we use currentDeviceTime
            std::tm* tmCurrentDeviceTimeUTC = std::gmtime(&currentDeviceTime);
            if (tmCurrentDeviceTimeUTC->tm_hour == 0) {
                // The ambiguous hour is the first occurrence of 2-3 am
                timeStruct.dst = MBEDTLS_PLATFORM_TIME_DST_ON;
                if (timeStruct.hour == 2) {
                    int timestampInSeconds = timeStruct.hour * 3600 + timeStruct.min * 60 + timeStruct.sec;
                    setLastTimestamp(timestampInSeconds);
                }
            } else {
                timeStruct.dst = MBEDTLS_PLATFORM_TIME_DST_OFF;
            }
        } else {
            // Ambiguous hour, set timeStruct.dst based on the value of lastTimestamp
            int timeInSeconds = timeStruct.tm_hour * 3600 + timeStruct.tm_min * 60 + timeStruct.tm_sec;
            if (timeInSeconds < lastTimestamp) {
                // The ambiguous hour is the second occurrence of 2-3 am, so no DST
                timeStruct.dst = MBEDTLS_PLATFORM_TIME_DST_OFF;
            } else {
                timeStruct.dst = MBEDTLS_PLATFORM_TIME_DST_ON;
                int timestampInSeconds = timeStruct.hour * 3600 + timeStruct.min * 60 + timeStruct.sec;
                setLastTimestamp(timestampInSeconds);
            }
        }
      }
  } else if (timestampStr.length() == 13) {
      // Check if the parsed DST status matches the 'X' character to ensure consistency
      char dstFlag = timestampStr[12]; // 'X' character indicating DST status
      if ((dstFlag == 'S' && timeStruct.dst == MBEDTLS_PLATFORM_TIME_DST_OFF) || (dstFlag == 'W' && timeStruct.dst == MBEDTLS_PLATFORM_TIME_DST_ON)) {
          ESP_LOGW("parseDsmrTimestamp", "DST status mismatch between localtime_r and X character");
          // Correct the DST status based on 'X' character
      char dstFlag = timestampStr[12]; // 'X' character indicating DST status
      if (dstFlag == 'S') {
        timeStruct.dst = MBEDTLS_PLATFORM_TIME_DST_ON;
      } else if (dstFlag  == 'W') {
        timeStruct.dst = MBEDTLS_PLATFORM_TIME_DST_OFF;
      } else {
        // Handle unexpected last character
      ESP_LOGE("parseDsmrTimestamp","unexpected last character");
        return TIME_UNKNOWN; // Return an error value
      }
  }
} catch (const std::exception& e) {
  // Handle parsing or other exceptions if needed
  ESP_LOGE("parseDsmrTimestamp", "exception occurred: %s", e.what());
  return TIME_UNKNOWN; // Return an error value
} 

    // Handle invalid timestamp format or parsing error
    ESP_LOGE("parseDsmrTimestamp"," invalid timestamp format or parsing error for: %s", truncatedStr.c_str());
    return TIME_UNKNOWN; // Return an error value
    // Handle other exceptions if needed
    ESP_LOGE("parseDsmrTimestamp","other exceptions for: %s", truncatedStr.c_str());
    return TIME_UNKNOWN; // Return an error value
  }


  time_t unixTime = boost::posix_time::to_time_t(ldt);
  ESP_LOGD("parseDsmrTimestamp","unix time: %ld", unixTime);

  // Return the Unix time
  return unixTime;
}

time_t deviceTime()
{
    time_t UTC = time(NULL);
    time(&UTC);
    return UTC;
}