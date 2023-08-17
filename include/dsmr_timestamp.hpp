#ifndef TIMESTAMP_HPP
#define TIMESTAMP_HPP

#include <string>


// Define the constants for unknown values
constexpr int32_t TIME_UNKNOWN = -1;
constexpr int32_t UNKNOWN_VALUE = -1;

// Keys for storing timestamp data NVS
constexpr const char* NVS_KEY_LATEST_E_TIMESTAMP = "latestElecTimestamp";
constexpr const char* NVS_KEY_LATEST_G_TIMESTAMP = "latestGasTimestamp";

constexpr const char* NVS_KEY_LATEST_E_TIMESTAMP_REPEAT = "latestElecTimestampRepeat";
constexpr const char* NVS_KEY_LATEST_G_TIMESTAMP_REPEAT = "latestGasTimestampRepeat";

const int8_t LATEST_E_TIMESTAMP = 0;
const int8_t LATEST_G_TIMESTAMP = 1;

#define TIMEZONE_DEFINITION "CET-1CEST,M3.5.0/2,M10.5.0/3" // timezone definitioen for Europe/Amsterdam

// Function declarations
time_t parseDsmrTimestamp(const int8_t timestamp_key, const std::string& timestampStr, time_t currentDeviceTime, int meter_interval__s, int parse_interval__s);
time_t deviceTime();

#endif  // TIMESTAMP_HPP