#ifndef TIMESTAMP_HPP
#define TIMESTAMP_HPP

#include <string>

// Define the time unknown constant
constexpr int32_t TIME_UNKNOWN = -1;

// Key for storing last timestamp in NVS
constexpr const char* NVS_KEY_LAST_E_TIMESTAMP = "lastElecTimestamp";
constexpr const char* NVS_KEY_LAST_G_TIMESTAMP = "lastGasTimestamp";

// Function declarations
int32_t getLastTimestamp(const std::string &nvs_timestamp_key);
void setLastTimestamp(const std::string &nvs_timestamp_key, int32_t newTimestamp);
time_t parseDsmrTimestamp(const std::string &nvs_timestamp_key, const std::string& timestampStr, time_t currentDeviceTime = 0);
time_t deviceTime();

#endif  // TIMESTAMP_HPP