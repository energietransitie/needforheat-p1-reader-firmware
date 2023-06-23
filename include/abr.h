#ifndef _DBR_H
#define _DBR_H


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
/**
 * --------LIBRARIES--------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_system.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_wifi.h>
#include <driver/uart.h>
#include <driver/gpio.h>
#include <esp_now.h>
#include <esp_err.h>
#include <esp_timer.h>

int32_t getBaudrate__b_s_1();
int32_t detectBaudrate__b_s_1();
void abrInit();
int32_t decideBaudrate__b_s_1(int32_t smallestBitFlankInterval__us);
int32_t calculateBitInterval__us(int32_t baudrate__b_s_1);
int32_t findNearestBaudRate__b_s_1(int32_t * candidateBaudRates__b_s_1, int32_t measuredBaudRate_us);
void setBaudrate__b_s_1(int32_t baudrate__b_s_1);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif //ifndef _P1CONFIG_H