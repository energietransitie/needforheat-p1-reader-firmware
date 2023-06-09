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

void uartStartDetectBaudrate();
void abrInit();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //ifndef _P1CONFIG_H