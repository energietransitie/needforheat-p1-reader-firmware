#include <dsmr_timestampTest.hpp>
#include <dsmr_timestamp.hpp>
#define MDBMeasurments 3599; //Max distance between measurements

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
    ESP_LOGI("Unit Test", "Results below");
    //DSMR2/3
    ESP_LOGI("Unit Test", "DSMR2/3:\n");
    if(1698535434 == parseDsmrTimestamp("231029012354", 1698535434))
    {
        ESP_LOGI("Unit Test", "1 PASSED\n");
        passed += 1;
    }
    else
    {
        ESP_LOGI("Unit Test", "1 FAILED\n");
        failed += 1;
    }

    if(1698539034 == parseDsmrTimestamp("231029022354", 1698539034))
    {
        ESP_LOGI("Unit Test", "2 PASSED\n");
        passed += 1;
    }
    else
    {
        ESP_LOGI("Unit Test", "2 FAILED\n");
        failed += 1;
    }
    if(1698539035 == parseDsmrTimestamp("231029022355", 1698539035))
     {
        ESP_LOGI("Unit Test", "3 PASSED\n");
        passed += 1;
    }
    else
    {
        ESP_LOGI("Unit Test", "3 FAILED\n");
        failed += 1;
    }
    if(1698542634 == parseDsmrTimestamp("231029022354", 1698542634))
    {
        ESP_LOGI("Unit Test", "4 PASSED\n");
        passed += 1;
    }
    else
    {
        ESP_LOGI("Unit Test", "4 FAILED\n");
        failed += 1;
    }

    //DSMR4/5
    ESP_LOGI("Unit Test", "DSMR4/5:\n");

    if(1698535434 == parseDsmrTimestamp("231029012354S", 1698535434))
    {
        ESP_LOGI("Unit Test", "5 PASSED\n");
        passed += 1;
    }
    else
    {
        ESP_LOGI("Unit Test", "5 FAILED\n");
        failed += 1;
    }
    if(1698539034 == parseDsmrTimestamp("231029022354S", 1698539034))
     {
        ESP_LOGI("Unit Test", "6 PASSED\n");
        passed += 1;
    }
    else
    {
        ESP_LOGI("Unit Test", "6 FAILED\n");
        failed += 1;
    }
    if(1698539035 == parseDsmrTimestamp("231029022355S", 1698539035))
     {
        ESP_LOGI("Unit Test", "7 PASSED\n");
        passed += 1;
    }
    else
    {
        ESP_LOGI("Unit Test", "7 FAILED\n");
        failed += 1;
    }
    if(1698542634 == parseDsmrTimestamp("231029022354W", 1698542634))
     {
        ESP_LOGI("Unit Test", "8 PASSED\n");
        passed += 1;
    }
    else
    {
        ESP_LOGI("Unit Test", "8 FAILED\n");
        failed += 1;
    }

    ESP_LOGI("Unit Test", "Total: %d PASSED, %d FAILED", passed, failed);
}