#include <timestampTestCopy.hpp>
#define MDBMeasurments 3599; //Max distance between measurements

time_t parseDsmrTimestamp(const char* dsmrTimestamp, time_t testUTC)
{
    const char * TIMEZONE = "Europe/Amsterdam";
    time_t t;
    char isoBuffer[sizeof "0000-00-00T00:00:00+0000"];
    ESP_LOGI("Time","Smart meter timestamp: %s", dsmrTimestamp);
    if(!(strlen(dsmrTimestamp) < 12))
    {
        struct tm timeStruct;
        char formattedString[19] = "00-00-00 00:00:00";

        //use foreach to format the dsmr timestamp to a readable string
        uint8_t stringLocation = 0;
        for (uint8_t i = 0; i < strlen(formattedString); i++)
        {
            if (formattedString[i] == '0')
            {
                formattedString[i] = dsmrTimestamp[stringLocation];
                stringLocation++;
            } 
        }

        strptime(formattedString, "%y-%m-%d %H:%M:%S", &timeStruct); //break up the string into a struct
        

        if (dsmrTimestamp[12] == 'S')
        {
            timeStruct.tm_isdst = 1;
        }
        else if (dsmrTimestamp[12] == 'W')
        {
            timeStruct.tm_isdst = 0;
        }
        
        

        strftime(isoBuffer,sizeof isoBuffer, "%FT%T", &timeStruct);
        uint8_t offset = timeStruct.tm_hour;
        
        if (timeStruct.tm_mon <= 12 && timeStruct.tm_mday <=31 && timeStruct.tm_hour < 24 && timeStruct.tm_min < 60 && timeStruct.tm_sec < 60)//if date has no overflow
        {
            t = mktime(setHours(&timeStruct, testUTC));  // t is now the desired time_t
        }
        else
        {
            return t = -1;
        }

       
        offset -= timeStruct.tm_hour;
        if(offset > 2)
        {
            offset /= 100;
        }
        ESP_LOGI("Time","Smart meter ISO8601 timestamp: %s+0%d00", isoBuffer, offset);
    

    }
    else
    {
        t = Scheduler::GetCurrentTaskTime();//use the scheduler time because theres no available time
    }


    strftime(isoBuffer, sizeof isoBuffer, "%FT%TZ", gmtime(&t));
    ESP_LOGI("Time","ISO8601 timestamp: %s", isoBuffer);
    
    ESP_LOGI("Time","Unix timestamp: %jd\n", (intmax_t)t);
    return t;
}



tm *setHours(tm *timeStruct, time_t testUTC)
{
    //create utc timestamp into a tm struct
    //time_t UTC = time(NULL);
    //time(&UTC);
    //instead of taking the current UTC time we use a custom UTC time for testing
    struct tm * utcTm;
    utcTm = gmtime(&testUTC);

    //get correct hours
    if(timeStruct->tm_isdst == 1)//summertime DSMR 4/5
    {
    timeStruct->tm_hour -= 2;
    }
    else if(timeStruct->tm_isdst == 0)//wintertime DSMR 4/5
    {
        timeStruct->tm_hour -= 1;
    }
    else if(timeStruct->tm_isdst < 0)//DSMR 2/3
    {
        if (timeStruct->tm_min >= utcTm->tm_min && timeStruct->tm_sec > utcTm->tm_sec)
        {
            timeStruct->tm_hour = utcTm->tm_hour-1;
        }
        else
        {
            timeStruct->tm_hour = utcTm->tm_hour;
        }
        
        //parse utc data into dsmr timestamps
        timeStruct->tm_year = utcTm->tm_year;
        timeStruct->tm_mon = utcTm->tm_mon;
        timeStruct->tm_mday = utcTm->tm_mday;
       
    }
    
    return timeStruct;
}

/**
 * DSMR2/3 timestamp (format=YYMMDDhhmmss)	|   DSMR4/5 timestamp (format=YYMMDDhhmmssX)	|   Unix timestamp	|   ISO8601 fully qualified (tz='Europe/Amsterdam')	|   ISO8601 fully qualified (tz='UTC')
 * ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * 231029012354	                            |   231029012354S	                            |   1698535434	    |   2023-10-29T01:23:54+0200	                    |   2023-10-28T23:23:54Z
 * 231029022354	                            |   231029022354S	                            |   1698539034	    |   2023-10-29T02:23:54+0200	                    |   2023-10-29T00:23:54Z
 * 231029022355	                            |   231029022355S	                            |   1698539035	    |   2023-10-29T02:23:55+0200	                    |   2023-10-29T00:23:55Z
 * 231029022354	                            |   231029022354W	                            |   1698542634	    |   2023-10-29T02:23:54+0100	                    |   2023-10-29T01:23:54Z
*/

void parsedTimestampsTests()
{
    /**
     * DSMR2/3
     * Max difference between meausrement and UTC time can ben 59m:59 = 3599 seconds
     * there fore you can add distance as an extra test
     * example: parseDsmrTimestamp("231029012354", 1698535434+distance)
    */
    uint8_t passed = 0, failed = 0, distance = MDBMeasurments;
    ESP_LOGI("Unit Test:", "Results below\n");
    if(1698535434 == parseDsmrTimestamp("231029012354", 1698535434))
    {
        ESP_LOGI("Unit Test:", "1 PASSED\n");
        passed += 1;
    }
    else
    {
        ESP_LOGI("Unit Test:", "1 FAILED\n");
        failed += 1;
    }

    if(1698539034 == parseDsmrTimestamp("231029022354", 1698539034))
    {
        ESP_LOGI("Unit Test:", "2 PASSED\n");
        passed += 1;
    }
    else
    {
        ESP_LOGI("Unit Test:", "2 FAILED\n");
        failed += 1;
    }
    if(1698539035 == parseDsmrTimestamp("231029022355", 1698539035))
     {
        ESP_LOGI("Unit Test:", "3 PASSED\n");
        passed += 1;
    }
    else
    {
        ESP_LOGI("Unit Test:", "3 FAILED\n");
        failed += 1;
    }
    if(1698542634 == parseDsmrTimestamp("231029022354", 1698542634))
    {
        ESP_LOGI("Unit Test:", "4 PASSED\n");
        passed += 1;
    }
    else
    {
        ESP_LOGI("Unit Test:", "4 FAILED\n");
        failed += 1;
    }

    //DSMR4/5
    if(1698535434 == parseDsmrTimestamp("231029012354S", 1698535434))
    {
        ESP_LOGI("Unit Test:", "5 PASSED\n");
        passed += 1;
    }
    else
    {
        ESP_LOGI("Unit Test:", "5 FAILED\n");
        failed += 1;
    }
    if(1698539034 == parseDsmrTimestamp("231029022354S", 1698539034))
     {
        ESP_LOGI("Unit Test:", "6 PASSED\n");
        passed += 1;
    }
    else
    {
        ESP_LOGI("Unit Test:", "6 FAILED\n");
        failed += 1;
    }
    if(1698539035 == parseDsmrTimestamp("231029022355S", 1698539035))
     {
        ESP_LOGI("Unit Test:", "7 PASSED\n");
        passed += 1;
    }
    else
    {
        ESP_LOGI("Unit Test:", "7 FAILED\n");
        failed += 1;
    }
    if(1698542634 == parseDsmrTimestamp("231029022354W", 1698542634))
     {
        ESP_LOGI("Unit Test:", "8 PASSED\n");
        passed += 1;
    }
    else
    {
        ESP_LOGI("Unit Test:", "8 FAILED\n");
        failed += 1;
    }

    ESP_LOGI("Unit Test:", "Total: %d PASSED, %d FAILED", passed, failed);
}