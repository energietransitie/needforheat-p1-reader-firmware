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

time_t parseDsmrTimestamp(const char* dsmrTimestamp);
tm *setHours(tm *timeStruct);