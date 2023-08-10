#ifndef TIMESTAMP_HPP
#define TIMESTAMP_HPP

#include <string>

// Define the time unknown constant
constexpr int32_t TIME_UNKNOWN = -1;

// Key for storing last timestamp in NVS
constexpr const char* NVS_KEY_LAST_E_TIMESTAMP = "lastElecTimestamp";
constexpr const char* NVS_KEY_LAST_G_TIMESTAMP = "lastGasTimestamp";
const int8_t LAST_E_TIMESTAMP = 0;
const int8_t LAST_G_TIMESTAMP = 1;

// Function declarations
int8_t getLastLocalTimeMinutes(const int8_t timestamp_key);
void storeLastLocalTimeMinutes(const int8_t timestamp_key, int8_t newLocalTimeMinutes);
time_t parseDsmrTimestamp(const int8_t timestamp_key, const std::string& timestampStr, time_t currentDeviceTime = 0);
time_t deviceTime();

#endif  // TIMESTAMP_HPP