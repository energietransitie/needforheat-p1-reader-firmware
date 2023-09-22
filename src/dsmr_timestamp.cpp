#include <dsmr_timestamp.hpp>
#include <nvs_flash.h>
#include <esp_log.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <ctime> // Include this header for standard time functions

static int32_t latestCorrectUnixTime[2] = {TIME_UNKNOWN, TIME_UNKNOWN};   // Initialize to a default values
static bool initializedLatestCorrectUnixTime[2] = {false, false};
static std::string nvs_timestamp_key[2] = {NVS_KEY_LATEST_E_TIMESTAMP, NVS_KEY_LATEST_G_TIMESTAMP};

static int32_t latestCorrectTimeRepeats[2] = {UNKNOWN_VALUE, UNKNOWN_VALUE};   // Initialize to a default value
static bool initializedLatestCorrectTimeRepeats[2] = {false, false};
static std::string nvs_timestamp_repeat_key[2] = {NVS_KEY_LATEST_E_TIMESTAMP_REPEAT, NVS_KEY_LATEST_E_TIMESTAMP_REPEAT};



std::string formatTimeISO8601(std::tm timeInfo) {

    std::ostringstream oss;
    if (timeInfo.tm_isdst==-1) {
        oss << std::put_time(&timeInfo, "%Y-%m-%dT%H:%M:%S");
    } else {
        oss << std::put_time(&timeInfo, "%Y-%m-%dT%H:%M:%S%z");
    }

    return oss.str();
}

std::string formatTimeISO8601(time_t unixTime) {
    // Set the specified time zone
    setenv("TZ", TIMEZONE_DEFINITION, 1); // 1 for overwrite
    tzset();
    std::tm* timeInfo = std::localtime(&unixTime); // Convert to local time

    return formatTimeISO8601(*timeInfo); // Pass by value (copy) of the tm struct
}

time_t getLatestCorrectUnixTime(const int8_t timestamp_key)
{
    if (!initializedLatestCorrectUnixTime[timestamp_key]) {
        // Get baudrate from NVS and store it in RAM
        nvs_handle_t nvsStorage;
        esp_err_t err;
        latestCorrectUnixTime[timestamp_key] = TIME_UNKNOWN;
        err = nvs_open("storage", NVS_READONLY, &nvsStorage);
        if (err != ESP_OK) {
            ESP_LOGD("NVS","Error (%s) opening NVS handle!", esp_err_to_name(err));
        } else {
            ESP_LOGD("NVS","Opened");

            ESP_LOGD("NVS","Reading %s value from NVS ... ", nvs_timestamp_key[timestamp_key].c_str());

            err = nvs_get_i32(nvsStorage, nvs_timestamp_key[timestamp_key].c_str(), &latestCorrectUnixTime[timestamp_key]);
            switch (err) {
                case ESP_OK:
                    ESP_LOGD("NVS","Done");
                    ESP_LOGD("NVS", "%s value read: %s", nvs_timestamp_key[timestamp_key].c_str(), std::to_string(latestCorrectUnixTime[timestamp_key]).c_str());
                    break;
                case ESP_ERR_NVS_NOT_FOUND:
                    ESP_LOGD("NVS","%s value is not initialized yet!", nvs_timestamp_key[timestamp_key].c_str());
                    latestCorrectUnixTime[timestamp_key] = TIME_UNKNOWN;
                    break;
                default :
                    ESP_LOGD("NVS","Error (%s) reading!", esp_err_to_name(err));
                    break;
            }
        
            // Close
            nvs_close(nvsStorage);
        }
        // Set the flag to indicate baudrate has been initialized from NVS
        initializedLatestCorrectUnixTime[timestamp_key] = true;
    }
    return latestCorrectUnixTime[timestamp_key];
}

void storeLatestCorrectUnixTime(const int8_t timestamp_key, time_t newCorrectUnixTime) 
{
    // store newTimestamp in nvs if different from stored
    time_t latestCorrectUnixTime_stored = getLatestCorrectUnixTime(timestamp_key);
    ESP_LOGD("P1", "timestamp %s stored: %ld; new: %ld", nvs_timestamp_key[timestamp_key].c_str(), latestCorrectUnixTime_stored, newCorrectUnixTime);
    if(newCorrectUnixTime != latestCorrectUnixTime_stored)
    {
        // write value to nvs
        nvs_handle_t nvsStorage;
        esp_err_t err;
        err = nvs_open("storage", NVS_READWRITE, &nvsStorage);
        if (err != ESP_OK) {
            ESP_LOGD("NVS","Error (%s) opening NVS handle!", esp_err_to_name(err));
        } else {
            ESP_LOGD("NVS","Opened");

            if(newCorrectUnixTime == TIME_UNKNOWN)
            {
                err = nvs_erase_key(nvsStorage, nvs_timestamp_key[timestamp_key].c_str());
                ESP_LOGD("NVS","The value %ss is removed from nvs", nvs_timestamp_key[timestamp_key].c_str());
            }
            else
            {
                err = nvs_set_i32(nvsStorage, nvs_timestamp_key[timestamp_key].c_str(), newCorrectUnixTime);                 
                ESP_LOGD("NVS","For key %s value %ld (%s) is stored in nvs", nvs_timestamp_key[timestamp_key].c_str(), newCorrectUnixTime, formatTimeISO8601(newCorrectUnixTime).c_str());
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
    latestCorrectUnixTime[timestamp_key] = newCorrectUnixTime;
    // Set the flag to indicate baudrate has been initialized in RAM
    initializedLatestCorrectUnixTime[timestamp_key] = true;
}

int32_t getLatestCorrectTimeRepeats(const int8_t timestamp_key)
{
    if (!initializedLatestCorrectTimeRepeats[timestamp_key]) {
        // Get value from NVS and store it in RAM
        nvs_handle_t nvsStorage;
        esp_err_t err;
        latestCorrectTimeRepeats[timestamp_key] = UNKNOWN_VALUE;
        err = nvs_open("storage", NVS_READONLY, &nvsStorage);
        if (err != ESP_OK) {
            ESP_LOGD("NVS", "Error (%s) opening NVS handle!", esp_err_to_name(err));
        } else {
            ESP_LOGD("NVS", "Opened");

            ESP_LOGD("NVS", "Reading %s value from NVS ... ", nvs_timestamp_repeat_key[timestamp_key].c_str());

            err = nvs_get_i32(nvsStorage, nvs_timestamp_repeat_key[timestamp_key].c_str(), &latestCorrectTimeRepeats[timestamp_key]);
            switch (err) {
                case ESP_OK:
                    ESP_LOGD("NVS", "Done");
                    ESP_LOGD("NVS", "%s value read: %s", nvs_timestamp_repeat_key[timestamp_key].c_str(), std::to_string(latestCorrectTimeRepeats[timestamp_key]).c_str());
                    break;
                case ESP_ERR_NVS_NOT_FOUND:
                    ESP_LOGD("NVS", "%s value is not initialized yet!", nvs_timestamp_repeat_key[timestamp_key].c_str());
                    latestCorrectTimeRepeats[timestamp_key] = UNKNOWN_VALUE;
                    break;
                default:
                    ESP_LOGD("NVS", "Error (%s) reading!", esp_err_to_name(err));
                    break;
            }
        
            // Close
            nvs_close(nvsStorage);
        }
        // Set the flag to indicate value has been initialized from NVS
        initializedLatestCorrectTimeRepeats[timestamp_key] = true;
    }
    return latestCorrectTimeRepeats[timestamp_key];
}

void storeLatestCorrectTimeRepeats(const int8_t timestamp_key, int32_t newCorrectTimeRepeats) 
{
    // Store new value in NVS if different from stored
    int32_t latestCorrectTimeRepeats_stored = getLatestCorrectTimeRepeats(timestamp_key);
    ESP_LOGD("P1", "timestamp %s stored: %d; new: %d", nvs_timestamp_repeat_key[timestamp_key].c_str(), latestCorrectTimeRepeats_stored, newCorrectTimeRepeats);
    if (newCorrectTimeRepeats != latestCorrectTimeRepeats_stored)
    {
        // Write value to NVS
        nvs_handle_t nvsStorage;
        esp_err_t err;
        err = nvs_open("storage", NVS_READWRITE, &nvsStorage);
        if (err != ESP_OK) {
            ESP_LOGD("NVS", "Error (%s) opening NVS handle!", esp_err_to_name(err));
        } else {
            ESP_LOGD("NVS", "Opened");

            if (newCorrectTimeRepeats == UNKNOWN_VALUE)
            {
                err = nvs_erase_key(nvsStorage, nvs_timestamp_repeat_key[timestamp_key].c_str());
                ESP_LOGD("NVS", "The value %ss is removed from NVS", nvs_timestamp_repeat_key[timestamp_key].c_str());
            }
            else
            {
                err = nvs_set_i32(nvsStorage, nvs_timestamp_repeat_key[timestamp_key].c_str(), newCorrectTimeRepeats);
                ESP_LOGD("NVS", "For key %s value %d is stored in NVS", nvs_timestamp_repeat_key[timestamp_key].c_str(), newCorrectTimeRepeats);
            }

            // Commit written value
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
    latestCorrectTimeRepeats[timestamp_key] = newCorrectTimeRepeats;
    // Set the flag to indicate value has been initialized in RAM
    initializedLatestCorrectTimeRepeats[timestamp_key] = true;
}


bool isAmbiguous(const struct tm& timeStruct) {
    // Check if the given time is ambiguous
    return (timeStruct.tm_isdst == -1);
}

bool isApproachingAmbiguity(time_t givenTimestamp, int margin__s) {
    // Set the specified time zone
    setenv("TZ", TIMEZONE_DEFINITION, 1); // 1 for overwrite
    tzset();
    // Convert the given time to a Unix timestamp
    // Add the specified number of seconds to the Unix timestamp
    ESP_LOGD("isApproachingAmbiguity", "Checking for ambiguity of %ld (%s) with margin %i seconds", givenTimestamp, formatTimeISO8601(givenTimestamp).c_str(), margin__s);
    givenTimestamp += margin__s;

    // Convert the adjusted timestamp to a struct tm in local time
    struct tm adjustedTime;
    localtime_r(&givenTimestamp, &adjustedTime);

    // Set the tm_isdst field to -1 to indicate ambiguous time
    adjustedTime.tm_isdst = -1;

    // Convert the adjusted time back to a Unix timestamp
    mktime(&adjustedTime);
    bool isAmbiguousResult = isAmbiguous(adjustedTime);

    ESP_LOGD("isApproachingAmbiguity", "Checked future time local time %s is %s", formatTimeISO8601(adjustedTime).c_str(), isAmbiguousResult ? "AMBIGUOUS" : "not ambiguous");

    // Check if the adjusted time is approaching ambiguity
    return isAmbiguousResult;
}

time_t parseDsmrTimestamp(int8_t timestamp_key, const std::string& timestampStr, time_t currentDeviceTime, int meter_interval__s, int parse_interval__s) {
    time_t unixTime = TIME_UNKNOWN;
    // Parse input timestamp string
    ESP_LOGD("parseDsmrTimestamp","dsmr timestamp: %s; current device Unix time: %ld (%s)", timestampStr.c_str(), currentDeviceTime,  formatTimeISO8601(currentDeviceTime).c_str());

    std::string truncatedStr = timestampStr.substr(0, 12);

    // Convert the truncated timestamp YYMMDDhhmmss string to a time structure
    struct tm time{};
    struct tm latestCorrectlocalTime{};

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
        ESP_LOGE("parseDsmrTimestamp", "unexpected latest character");
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
    const char* timeZone = TIMEZONE_DEFINITION; // DSMR alsways uses Europe/Amsterdam time zone
    setenv("TZ", timeZone, 1); // 1 for overwrite
    tzset();

    // Use mktime() to create a Unix timestamp based on the time struct.
    // A negative value of time->tm_isdst causes mktime to attempt to determine if Daylight Saving Time was in effect.
    unixTime = mktime(&time);

    if (unixTime == TIME_UNKNOWN || time.tm_isdst == -1) {
        // Handle ambiguous local time: should be only two hours a year and only for DSMR2/3 timestamps (gas or electric))
        ESP_LOGD("parseDsmrTimestamp", "Ambiguous local time for: %s; time.tm_isdst = %i; unixTime: %ld (%s)", timestampStr.c_str(), time.tm_isdst, unixTime, formatTimeISO8601(unixTime).c_str());

        // Need to look at persistently stored timestamps to decide.
        time_t latestCorrectUnixTime = getLatestCorrectUnixTime(timestamp_key);
        int latestCorrectTimeRepeats = getLatestCorrectTimeRepeats(timestamp_key);
        int maxCorrectTimeRepeats = (meter_interval__s) / parse_interval__s;
        localtime_r(&latestCorrectUnixTime, &latestCorrectlocalTime);
        time.tm_isdst = latestCorrectlocalTime.tm_isdst;
        unixTime = mktime(&time); // first try conversion with the latest known DST setting.

        // In very rare cases, we have to rely even on currentDeviceTime given in parameter
        if (latestCorrectUnixTime == TIME_UNKNOWN && currentDeviceTime != 0) {
            // Edge case: no latestTimestamp stored yet in nvs; the only place where we use currentDeviceTime
            ESP_LOGD("parseDsmrTimestamp", "Edge case: ambiguous time and no history stored; currentDeviceTime: %ld (%s)", currentDeviceTime, formatTimeISO8601(currentDeviceTime).c_str());
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
            if (unixTime == latestCorrectUnixTime) {
                // seeing the same time can only happen so often...

                if (latestCorrectTimeRepeats == UNKNOWN_VALUE) {
                    latestCorrectTimeRepeats = 1;
                }
                latestCorrectTimeRepeats += 1;

                ESP_LOGD("parseDsmrTimestamp","This is occurrence no. %i we see this DSMR timestamp %s", latestCorrectTimeRepeats, timestampStr.c_str());

                storeLatestCorrectTimeRepeats(timestamp_key, latestCorrectTimeRepeats);
                if (latestCorrectTimeRepeats > maxCorrectTimeRepeats) {
                    // DST can no longer be in effect!
                    ESP_LOGD("parseDsmrTimestamp","DSMR timestamp is repeated more than %i, so DST can no longer be in effect", maxCorrectTimeRepeats);
                    time.tm_isdst = 0;
                } else {
                    // DST might still be in effect. But what if we skipped a measurement?
                    ESP_LOGD("parseDsmrTimestamp","DSMR timestamp is repeated less often than %i, so DST most likely still in effect", maxCorrectTimeRepeats);
                    time.tm_isdst = 1;
                }
                unixTime = mktime(&time);

            } else {
                // Predict the next Unix time based on the latest correct time
                time_t nextPredictedUnixTime =  latestCorrectUnixTime + meter_interval__s;
                ESP_LOGD("parseDsmrTimestamp","DSMR timestamp is not repeated; predicted timestamp is %s", formatTimeISO8601(nextPredictedUnixTime).c_str());

                //make two variants of time in order to compare
                struct tm timeDST0 = time;
                timeDST0.tm_isdst = 0;
                struct tm timeDST1 = time;
                timeDST1.tm_isdst = 1;

                // Convert both structures to Unix timestamps
                time_t unixTimeDST0 = mktime(&timeDST0);
                time_t unixTimeDST1 = mktime(&timeDST1);
 
                // Determine which conversion is closer to the predicted Unix time
                if (std::abs(unixTimeDST0 - nextPredictedUnixTime) < std::abs(unixTimeDST1 - nextPredictedUnixTime)) {
                    ESP_LOGD("parseDsmrTimestamp","Candidate %s is chosen over  %s, because it is closer to the predicted time", formatTimeISO8601(unixTimeDST0).c_str(), formatTimeISO8601(unixTimeDST1).c_str());
                    unixTime = unixTimeDST0;
                } else {
                    ESP_LOGD("parseDsmrTimestamp","Candidate %s is chosen over  %s, because it is closer to the predicted time", formatTimeISO8601(unixTimeDST1).c_str(), formatTimeISO8601(unixTimeDST0).c_str());
                    unixTime = unixTimeDST1;
                }
            }
        }
        storeLatestCorrectUnixTime(timestamp_key, unixTime);
        // make sure that the converted unixTime is stored
    } else {
        // Handle unambiguous local time: make sure LatestTimestamp is reset (for next year, unless we're approachting ambiguous times)
        int lookahead__s = std::max(meter_interval__s, parse_interval__s);
        if (isApproachingAmbiguity(unixTime, lookahead__s)) {
            storeLatestCorrectUnixTime(timestamp_key, unixTime);
        } else {
            // NB setLatestTimestamp only writes to NVS if it's not the same as before
            storeLatestCorrectUnixTime(timestamp_key, TIME_UNKNOWN);
            storeLatestCorrectTimeRepeats(timestamp_key, UNKNOWN_VALUE);
        }
    }

    // Return the Unix time
    ESP_LOGD("parseDsmrTimestamp","converted Unix time: %ld (%s)", unixTime, formatTimeISO8601(unixTime).c_str());
    return unixTime;
}

time_t deviceTime()
{
    time_t UTC = time(NULL);
    time(&UTC);
    return UTC;
}