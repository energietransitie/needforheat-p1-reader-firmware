#include <dsmr_timestampTest.hpp>
#include <dsmr_timestamp.hpp>
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <util/nvs.hpp>

#define TAG "dsmr_timestampTest"

std::string formatTimestampISO8601(std::tm timeInfo) {

    std::ostringstream oss;
    if (timeInfo.tm_isdst==-1) {
        oss << std::put_time(&timeInfo, "%Y-%m-%dT%H:%M:%S");
    } else {
        oss << std::put_time(&timeInfo, "%Y-%m-%dT%H:%M:%S%z");
    }

    return oss.str();
}

std::string formatTimestampISO8601(std::time_t unixTime) {
    // Set the specified time zone
    setenv("TZ", TIMEZONE_DEFINITION, 1); // 1 for overwrite
    tzset();
    std::tm* timeInfo = std::localtime(&unixTime); // Convert to local time

    return formatTimestampISO8601(*timeInfo); // Pass by value (copy) of the tm struct
}

std::string formatDsmrTimestamp(std::time_t timestamp, bool isAtLeastDsmr4) {
    setenv("TZ", TIMEZONE_DEFINITION, 1); // 1 for overwrite; DSMR always uses Europe/Amsterdam timezone
    tzset();
    struct tm timeInfo = {};
    localtime_r(&timestamp, &timeInfo);
    
    std::ostringstream oss;
    oss << std::put_time(&timeInfo, "%y%m%d%H%M%S");

    if (isAtLeastDsmr4) {
        if (timeInfo.tm_isdst == 1)
            oss << "S";
        else
            oss << "W";
    }

    return oss.str();
}

uint16_t getRandomSmallInteger(uint16_t max_value) {
    if (max_value == 0) {
        // Handle invalid input
        return 0;
    }
    
    uint32_t random_value = esp_random(); // Generate a random 32-bit value
    uint16_t random_integer = (random_value % max_value) + 1; // Ensure the result is within [1, max_value]
    
    return random_integer;
}

// Function to perform a test sequence
void performTestSequence() {

    struct TestSequence {
        //TODO: allow specification of a more human-fiendly start time
        time_t start_time_t = 1698530400;               // midight before the summer-wintertime switch in 2023
        uint16_t duration__h = 6;                       // 6 [h]
        uint16_t meter_interval__s = 1 * 60 * 60;       // 1 [h] * 60 [min/h] * 60 [s/min]
        // uint16_t meter_interval__s = 10;                // 10 [s]
        uint16_t parse_interval__s = 10 * 60;           // 10 [min] * 60 [s/min]
        // uint16_t parse_interval__s = 1 * 60 * 60;       // 1 [h] * 60 [min/h] * 60 [s/min]
        uint16_t maxRandomDevicetimeOffset__s = 5;
        uint16_t maxRandomMeasurementtimeOffset__s = 3;
        bool isAtLeastDsmr4 = false;
    } sequence;

    int passedCount = 0;
    int failedCount = 0;
    NVS::Initialize();


    ESP_LOGD(TAG, "Starting test sequence...");

    time_t startTime = sequence.start_time_t;
    uint16_t randomDevicetimeOffset__s = getRandomSmallInteger(sequence.maxRandomDevicetimeOffset__s);
    uint16_t randomMeasurementtimeOffset__s = getRandomSmallInteger(sequence.maxRandomMeasurementtimeOffset__s);

    ESP_LOGD(TAG, "Start Unix Time %ld (%s)", startTime, formatTimestampISO8601(startTime).c_str());
    ESP_LOGD(TAG, "Duration: %d hours", sequence.duration__h);
    ESP_LOGD(TAG, "Meter interval: %d seconds", sequence.meter_interval__s);
    ESP_LOGD(TAG, "Parsing interval: %d seconds", sequence.parse_interval__s);
    ESP_LOGD(TAG, "Max Random Device Time Offset: %d seconds", sequence.maxRandomDevicetimeOffset__s);
    ESP_LOGD(TAG, "Max Random Measurement Time Offset: %d seconds", sequence.maxRandomMeasurementtimeOffset__s);
    ESP_LOGD(TAG, "DSMR version is DSME %s", sequence.isAtLeastDsmr4 ? "4 or higher" : "3 or lower");
    
    time_t currentUnixTime;
    int number_of_meter_intervals_passed = 0;
    int elapsed_since_startTime__s;
    time_t mostRecentMeterUnixTime;

    // Perform the test sequence
    for (int i = 0; i < sequence.duration__h * 60 * 60  / sequence.parse_interval__s; ++i) {
        currentUnixTime = startTime + i * sequence.parse_interval__s;

        number_of_meter_intervals_passed = (i * sequence.parse_interval__s / sequence.meter_interval__s);
        elapsed_since_startTime__s = number_of_meter_intervals_passed * sequence.meter_interval__s;
        mostRecentMeterUnixTime = startTime + elapsed_since_startTime__s;
        //update randomMeasurementtimeOffset__s only when a new mostRecentMeterUnixTime occurs
        if (currentUnixTime - mostRecentMeterUnixTime < sequence.parse_interval__s) {
            randomMeasurementtimeOffset__s = getRandomSmallInteger(sequence.maxRandomMeasurementtimeOffset__s);
        }
        mostRecentMeterUnixTime += randomMeasurementtimeOffset__s;
        ESP_LOGD(TAG, "Test %d: number of meter intervals passed: %d, meter time: %ld (%s)", i, number_of_meter_intervals_passed, mostRecentMeterUnixTime, formatTimestampISO8601(mostRecentMeterUnixTime).c_str());

        std::string currentDsmrTimestamp = formatDsmrTimestamp(mostRecentMeterUnixTime, sequence.isAtLeastDsmr4);

        time_t parsedUnixTime = parseDsmrTimestamp(LATEST_G_TIMESTAMP, currentDsmrTimestamp, currentUnixTime + randomDevicetimeOffset__s, sequence.meter_interval__s, sequence.parse_interval__s);

        if (parsedUnixTime == mostRecentMeterUnixTime) {
            passedCount++;
            ESP_LOGI(TAG, "Test %d passed\r\n", i);
        } else {
            failedCount++;
            ESP_LOGE(TAG, "Test %d failed - Expected: %ld, Parsed: %ld", i, mostRecentMeterUnixTime, parsedUnixTime);
            ESP_LOGE(TAG, "   DSMR Timestamp: %s", currentDsmrTimestamp.c_str());
            ESP_LOGE(TAG, "   Expected (ISO8601 format): %s", formatTimestampISO8601(mostRecentMeterUnixTime).c_str());
            ESP_LOGE(TAG, "   Parsed (ISO8601 format): %s\r\n", formatTimestampISO8601(parsedUnixTime).c_str());
        }
    }

    ESP_LOGD(TAG, "Test sequence completed.");
    ESP_LOGD(TAG, "Total tests: %d", passedCount + failedCount);
    ESP_LOGD(TAG, "Tests passed: %d", passedCount);
    if (failedCount > 0) {
        ESP_LOGE(TAG, "Tests failed: %d", failedCount);
    } else {
        ESP_LOGD(TAG, "Tests failed: %d", failedCount);
    }
}

