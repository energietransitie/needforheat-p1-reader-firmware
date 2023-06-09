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

#include <assert.h>

/**
		 * Parse a custom time for testing
		 *
		 * @param dsmrTimestamp timestamp given by a DSMR.
		 * @param testUTC the given UTC timestamp used by for testing, max diffrence from DSMR timestamp = 3599S + offset.
		 */
time_t parseDsmrTimestamp(const char* dsmrTimestamp, time_t testUTC);

tm *setHours(tm *timeStruct, time_t testUTC);

/**
		 * Run trough all the available test cases
		 */
void parsedTimestampsTests();