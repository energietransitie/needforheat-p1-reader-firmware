#include <timestamp.hpp>

time_t parseDsmrTimestamp(const char* dsmrTimestamp, time_t deviceTime)
{
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
        else
        {
            timeStruct.tm_isdst = -1;
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
        ESP_LOGI("Time","ISO8601 timestamp (tz='Europe/Amsterdam'): %s+0%d00", isoBuffer, offset);
        
    }
    else
    {
        t = Scheduler::GetCurrentTaskTime();//use the scheduler time because theres no available time
    }

    strftime(isoBuffer, sizeof isoBuffer, "%FT%TZ", gmtime(&t));
    ESP_LOGI("Time","ISO8601 timestamp (tz=UTC): %s", isoBuffer);
    
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
        timeStruct = compareWithNVSTimestamp(timeStruct);
        // if (timeStruct->tm_min >= utcTm->tm_min && timeStruct->tm_sec > utcTm->tm_sec)
        // {
        //     timeStruct->tm_hour = utcTm->tm_hour-1;
        // }
        // else
        // {
        //     timeStruct->tm_hour = utcTm->tm_hour;
        // }
        
        // //parse utc data into dsmr timestamps
        // timeStruct->tm_year = utcTm->tm_year;
        // timeStruct->tm_mon = utcTm->tm_mon;
        // timeStruct->tm_mday = utcTm->tm_mday;   

        if(timeStruct->tm_isdst == 1)//summertime DSMR 4/5
        {
        timeStruct->tm_hour -= 2;
        }
        else if(timeStruct->tm_isdst == 0)//wintertime DSMR 4/5
        {
            timeStruct->tm_hour -= 1;
        }
       
    }
    
    return timeStruct;
}

time_t deviceTime()
{
    time_t UTC = time(NULL);
    time(&UTC);
    return UTC;
}

tm *compareWithNVSTimestamp(tm *dsmr23Time)
{
    //determine summer or wintertime
    dsmr23Time->tm_isdst = 1;
    
    struct tm NVStm;
    char nvsTimestamp[sizeof "00-00-00 00:00:00"];
    size_t nvsSize = sizeof(nvsTimestamp);
    nvs_handle_t nvsStorage;
    esp_err_t err;
    int8_t nvsIdst;

    err = nvs_open("storage", NVS_READWRITE, &nvsStorage);
    if (err != ESP_OK) {
        ESP_LOGD("NVS","Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        ESP_LOGD("NVS","Opened");

        ESP_LOGD("NVS","Reading previous timestamp from NVS ... ");

        err = nvs_get_i8(nvsStorage, "Timestamp_idst", &nvsIdst);

        switch (err)
        {
        case ESP_OK:
            switch (nvsIdst)
            {
            case 1:
                dsmr23Time->tm_isdst = 1;
                break;
            case 0:
                dsmr23Time->tm_isdst = 0;
                break;
            case -1:
                dsmr23Time->tm_isdst = -1;
                break;
            default:
                break;
            }
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            dsmr23Time->tm_isdst = 1;
        default:
            break;
        }

        err = nvs_get_str(nvsStorage, "Timestamp", nvsTimestamp, &nvsSize);
        switch (err) {
            case ESP_OK:
                ESP_LOGD("NVS","Done\n");
                ESP_LOGD("NVS","timestamp = %s"  "\n", nvsTimestamp);
                strptime(nvsTimestamp, "%y-%m-%d %H:%M:%S", &NVStm); //break up the string into a struct

                switch (dsmr23Time->tm_isdst)
                {
                case (1):
                    if(NVStm.tm_hour == dsmr23Time->tm_hour && NVStm.tm_min >= dsmr23Time->tm_min && NVStm.tm_sec >= dsmr23Time->tm_sec )//going into winter time
                    {
                        ESP_LOGD("Time","winter\n");
                        dsmr23Time->tm_isdst = 0;
                    }
                    else{
                        dsmr23Time->tm_isdst = 1;
                    }
                    break;
                case (0):
                    if(NVStm.tm_hour < dsmr23Time->tm_hour && NVStm.tm_min >= dsmr23Time->tm_min && NVStm.tm_sec > dsmr23Time->tm_sec)//going into summer time
                    {
                        ESP_LOGD("Time","summer\n");
                        dsmr23Time->tm_isdst = 1;
                    }
                    else{
                        dsmr23Time->tm_isdst = 0;
                    }
                    break;
                case (-1):
                        dsmr23Time->tm_isdst = -1;
                    break;
                default:
                    break;
                }

                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGD("NVS","The value is not initialized yet!\n");
                break;
            default :
            ESP_LOGD("NVS","Error (%s) reading!\n", esp_err_to_name(err));
            }

        
        strftime(nvsTimestamp ,sizeof nvsTimestamp, "%y-%m-%d %H:%M:%S", dsmr23Time);

        err = nvs_set_str(nvsStorage, "Timestamp", nvsTimestamp);
        nvsIdst = dsmr23Time->tm_isdst;
        err = nvs_set_i8(nvsStorage, "Timestamp_idst", nvsIdst);

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        ESP_LOGD("NVS", "Committing updates in NVS ... ");
        err = nvs_commit(nvsStorage);

                
        // Close
        nvs_close(nvsStorage);
    }

    return dsmr23Time;
}