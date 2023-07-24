#include <dsmr_timestamp.hpp>
#include <nvs_flash.h>
#include <esp_log.h>
#include <iomanip>
#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>

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
  boost::local_time::time_zone_ptr tz(new boost::local_time::posix_time_zone(timeZone));
  boost::local_time::local_date_time ldt(boost::local_time::not_a_date_time);
  boost::posix_time::time_duration td;

  try {
    // Parse the timestamp string with the given format and time zone
    boost::posix_time::ptime parsedTime = boost::posix_time::from_string(timestampStr);
    ldt = boost::local_time::local_date_time(parsedTime, tz);
    // Check for ambiguous local time
    if (ldt.is_dst() == boost::local_time::ambiguous) {
      // Handle ambiguous local time
      ESP_LOGE("parseDsmrTimestamp","Ambiguous local time for: %s", truncatedStr.c_str());
      // Timestamp in ambiguous hour!
      // First check if the timestamp format is YYMMDDhhmmssX
      if (timestampStr.length() == 13) {
        // Timestamp format: YYMMDDhhmmssX (DSMR4 or higher)
        // Just look at the character in the X position.
        if (timestampStr[12] == 'S') {
          ldt.set_dst(boost::local_time::dst_status::daylight_time);
        } else if (timestampStr[12] == 'W') {
          ldt.set_dst(boost::local_time::dst_status::standard_time);
        } else {
          // Handle unexpected last character
        ESP_LOGE("parseDsmrTimestamp","unexpected last character");
          return TIME_UNKNOWN; // Return an error value
        }
      } else {
        // Timestamp in ambiguous hour & format: YYMMDDhhmss (DSMR2/DSMR3)
        // Need to look at persistently stored timestamps to decide.
        // In very rare cases, we have to rely even on currentDeviceTime given in parameter
        int lastTimestamp = getLastTimestamp();
        if (lastTimestamp == -1 && currentDeviceTime != 0) {
          // Edge case: no lastTimestamp stored yet in nvs; the only place where we use currentDeviceTime
          std::tm* tmCurrentDeviceTimeUTC = std::gmtime(&currentDeviceTime);
          if (tmCurrentDeviceTimeUTC->tm_hour == 0) {
            // The ambiguous hour is the first occurrence of 2-3 am,
            ldt.set_dst(boost::local_time::dst_status::daylight_time);
            if (ldt.time_of_day().hours() == 2) {
              int timestampInSeconds = ldt.time_of_day().hours() * 3600 + ldt.time_of_day().minutes() * 60 + ldt.time_of_day().seconds();
              setLastTimestamp(timestampInSeconds);
            }
          } else {
            ldt.set_dst(boost::local_time::dst_status::standard_time);
          }
        } else {
          // Ambiguous hour, set isDstInEffect based on value of lastTimestamp
          if (ldt.time_of_day().hours() == 2 && lastTimestamp == ldt.time_of_day().hours() * 3600) {
            // The ambiguous hour is the second occurrence of 2-3 am, so no DST
            ldt.set_dst(boost::local_time::dst_status::standard_time);
          } else {
            ldt.set_dst(boost::local_time::dst_status::daylight_time);
            int timestampInSeconds = ldt.time_of_day().hours() * 3600 + ldt.time_of_day().minutes() * 60 + ldt.time_of_day().seconds();
            setLastTimestamp(timestampInSeconds);
          }
        }
      }
    }
  } catch (const boost::bad_lexical_cast& e) {
    // Handle invalid timestamp format or parsing error
    ESP_LOGE("parseDsmrTimestamp"," invalid timestamp format or parsing error for: %s", truncatedStr.c_str());
    return TIME_UNKNOWN; // Return an error value
  } catch (const boost::exception& e) {
    // Handle other exceptions if needed
    ESP_LOGE("parseDsmrTimestamp","other exceptions for: %s", truncatedStr.c_str());
    return TIME_UNKNOWN; // Return an error value
  }


  if (timeInfo.tm_isdst == -1) {
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