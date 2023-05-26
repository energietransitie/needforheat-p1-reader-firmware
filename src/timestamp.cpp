#include <timestamp.hpp>

time_t timestampFormatted(const char* timeString)
{
    time_t t;
    ESP_LOGI("Time","given time: %s", timeString);
    if(!(strlen(timeString) < 12))
    {
        struct tm timeStruct;
        char formattedString[18] = "00-00-00 00:00:00";

        //use foreach to format the dsmr timestamp to a readable string
        uint8_t stringLocation = 0;
        for (uint8_t i = 0; i < strlen(formattedString); i++)
        {
            if (formattedString[i] == '0')
            {
                formattedString[i] = timeString[stringLocation];
                stringLocation++;
            }   
        }

        strptime(formattedString, "%y-%m-%d %H:%M:%S", &timeStruct); //break up the string into a struct

        if (timeStruct.tm_hour < 24)
        {
            t = mktime(setHours(&timeStruct));  // t is now the desired time_t
        }
        else
        {
            t = Scheduler::GetCurrentTaskTime();//use the scheduler time because theres a incorrect time
        }
        
    }
    else
    {
        t = Scheduler::GetCurrentTaskTime();//use the scheduler time because theres no available time
    }

    ESP_LOGI("Time","Parsed time: %s", asctime(gmtime(&t)));
    
    ESP_LOGI("Time","unix time: %jd", (intmax_t)t);

    return t;
}



tm *setHours(tm *timeStruct)
{
    //create utc timestamp into a tm struct
    time_t UTC = time(NULL);
    time(&UTC);
    struct tm * utcTm;
    utcTm = gmtime(&UTC);

    //parse utc data into dsmr timestamps
    timeStruct->tm_year = utcTm->tm_year;
    timeStruct->tm_mon = utcTm->tm_mon;
    timeStruct->tm_mday = utcTm->tm_mday;
    timeStruct->tm_hour = utcTm->tm_hour;
    
    return timeStruct;
}