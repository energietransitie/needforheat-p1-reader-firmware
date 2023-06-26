#include <timestamp.hpp>
#include <unity.h>


/**
 * DSMR2/3 timestamp (format=YYMMDDhhmmss)	|   DSMR4/5 timestamp (format=YYMMDDhhmmssX)	|   Unix timestamp	|   ISO8601 fully qualified (tz='Europe/Amsterdam')	|   ISO8601 fully qualified (tz='UTC')
 * ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * 231029012354	                            |   231029012354S	                            |   1698535434	    |   2023-10-29T01:23:54+0200	                    |   2023-10-28T23:23:54Z
 * 231029022354	                            |   231029022354S	                            |   1698539034	    |   2023-10-29T02:23:54+0200	                    |   2023-10-29T00:23:54Z
 * 231029022355	                            |   231029022355S	                            |   1698539035	    |   2023-10-29T02:23:55+0200	                    |   2023-10-29T00:23:55Z
 * 231029022354	                            |   231029022354W	                            |   1698542634	    |   2023-10-29T02:23:54+0100	                    |   2023-10-29T01:23:54Z
*/

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void DSMR23TestCases(void) {
  // test stuff
  TEST_ASSERT_EQUAL_STRING(1698535434, parseDsmrTimestamp("231029012354", 1698535434));
  
  TEST_ASSERT_EQUAL_STRING(1698539034, parseDsmrTimestamp("231029022354", 1698539034));
  
  TEST_ASSERT_EQUAL_STRING(1698539035, parseDsmrTimestamp("231029022355", 1698539035));
  
  TEST_ASSERT_EQUAL_STRING(1698542634, parseDsmrTimestamp("231029022354", 1698542634));
}

void DSMR45TestCases(void) {
  // more test stuff
  TEST_ASSERT_EQUAL_STRING(1698535434, parseDsmrTimestamp("231029012354S", 1698535434));
  
  TEST_ASSERT_EQUAL_STRING(1698539034, parseDsmrTimestamp("231029022354S", 1698539034));
  
  TEST_ASSERT_EQUAL_STRING(1698539035, parseDsmrTimestamp("231029022355S", 1698539035));
  
  TEST_ASSERT_EQUAL_STRING(1698542634, parseDsmrTimestamp("231029022354W", 1698542634));
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(DSMR23TestCases);
  RUN_TEST(DSMR45TestCases);
  return UNITY_END();
}

/**
  * For ESP-IDF framework
  */
extern "C" void app_main() {
  runUnityTests();
}