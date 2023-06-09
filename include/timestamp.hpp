#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <esp_system.h>
#include <esp_log.h>
#include <esp_err.h>
#include <iostream> 

#include <scheduler.hpp>

/**
		 * Parse a custom time for testing
		 *
		 * @param dsmrTimestamp timestamp given by a DSMR.
		 * @param deviceUTC the given UTC timestamp, max diffrence from DSMR timestamp = 3599S + offset.
		 */
time_t parseDsmrTimestamp(const char* dsmrTimestamp, time_t deviceUTC);

/**
		 * Convert into utc
		 *
		 * @param timeStruct timestamp given by a DSMR.
		 * @param deviceUTC the given UTC timestamp, max diffrence from DSMR timestamp = 3599S + offset.
		 */
tm *setHours(tm *timeStruct, time_t deviceUTC);
time_t deviceTime();