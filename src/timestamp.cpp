#include <timestamp.hpp>

time_t timestampFormatted(const char* timeString)
{
    uint8_t offset = 0;
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
   
    time_t t = mktime(setHours(&timeStruct));  // t is now the desired time_t

    ESP_LOGI("Time","Parsed time: %02d:%02d:%02d", timeStruct.tm_hour, timeStruct.tm_min, timeStruct.tm_sec);

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

    if(!timeStruct->tm_mday)//there is no time use utc time
    {
        timeStruct->tm_min = utcTm->tm_min;
        timeStruct->tm_sec = utcTm->tm_sec;
    }
    
    return timeStruct;
}