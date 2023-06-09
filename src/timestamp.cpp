#include <timestamp.hpp>

time_t parseDsmrTimestamp(const char* dsmrTimestamp, time_t deviceTime)
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
            t = mktime(setHours(&timeStruct, deviceTime));  // t is now the desired time_t
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



tm *setHours(tm *timeStruct, time_t deviceTime)
{
    //parse the utc timestamp into a tm struct
    struct tm * utcTm;
    utcTm = gmtime(&deviceTime);

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

time_t deviceTime()
{
    time_t UTC = time(NULL);
    time(&UTC);
    return UTC;
}