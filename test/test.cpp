#include <unity.h>

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void DSMR23TestCases(void) {
  // test stuff
}

void DSMR45TestCases(void) {
  // more test stuff
}

int runUnityTests(void) {
  UNITY_BEGIN();
 // RUN_TEST(test_function_should_doBlahAndBlah);
  //RUN_TEST(test_function_should_doAlsoDoBlah);
  return UNITY_END();
}

/**
  * For ESP-IDF framework
  */
extern "C" void app_main() {
  runUnityTests();
}