#ifndef _P1CONFIG_H
#define _P1CONFIG_H


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
#include <nvs.h>
#include <esp_wifi.h>
#include <driver/uart.h>
#include <driver/gpio.h>
#include <esp_now.h>
#include <esp_err.h>

 /**
  * --------DEFINES--------
  */
#define P1CONFIG_VERSION "V1.0.0"
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#pragma message "Using P1Config version "  STR(P1CONFIG_VERSION)

  //Pin definitions:
#define BUTTON_GPIO12_SW3 GPIO_NUM_12
#define RED_LED_D1_ERROR GPIO_NUM_13
#define GREEN_LED_D2_STATUS GPIO_NUM_14
#define PIN_DRQ GPIO_NUM_17
#define OUTPUT_BITMASK ((1ULL << RED_LED_D1_ERROR) | (1ULL << GREEN_LED_D2_STATUS) | (1ULL << PIN_DRQ))
#define INPUT_BITMASK ((1ULL << BUTTON_GPIO12_SW3))

//UART defines
#define P1_BUFFER_SIZE 10240
#define P1_MESSAGE_SIZE 2048
#define P1PORT_UART_NUM UART_NUM_2


//Error types for P1 data reading:
#define P1_READ_OK 0
#define P1_ERROR_DSMR_NOT_FOUND 1
#define P1_ERROR_ELECUSEDT1_NOT_FOUND 2
#define P1_ERROR_ELECUSEDT2_NOT_FOUND 3
#define P1_ERROR_ELECRETURNT1_NOT_FOUND 4
#define P1_ERROR_ELECRETURNT2_NOT_FOUND 5
#define P1_ERROR_GAS_READING_NOT_FOUND 6
#define P1_ERROR_ELEC_TIMESTAMP_NOT_FOUND 7

// Constant to indicate P1 port parameters unknown
#define P1_UNKNOWN -1
#define NVS_KEY_BAUDRATE "P1baudrate"
#define NVS_KEY_ISATLEASTDSMR5 "isAtLeastDSMR5"

//Struct for holding the P1 Data:
typedef struct P1Data {
    float dsmrVersion;         // DSMR version (x.y, where x is the major version and y is the first minor verrion after the decimal point)
    double e_use_lo_cum__kWh;  // Electrical Energy used against Tariff 1 [kWh]
    double e_use_hi_cum__kWh;  // Electrical Energy used against Tariff 2 [kWh]
    double e_ret_lo_cum__kWh;  // Electrical Returned against Tariff 1 [kWh]
    double e_ret_hi_cum__kWh;  // Electrical Returned against Tariff 2 [kWh]
    char dsmrTimestamp_e[14];  // DSMR timestamp string for the electricity meter readings [14 positions needed for YYMDDhhmssX string and '\0' terminator]
    double g_use_cum__m3;      // Gas meter reading [m3]
    char dsmrTimestamp_g[14];  // DSMR timestamp string for gas meter reading [14 positions needed for YYMDDhhmssX or YYMDDhhmss string and '\0' terminator]
} P1Data;


/** ====== GLOBAL VARIABLES ============== */

/**
 *  ========== FUNCTIONS ================
 */

void initP1UART();
void setP1UARTConfigDSMR4or5();
void setP1UARTConfigDSMR2or3();
void initGPIO_P1();


//P1 port read and parsing

unsigned int CRC16(unsigned int crc, unsigned char *buf, int len);
int p1StringToStruct(const char *p1String, P1Data *p1Struct);
void printP1Error(int errorType);
P1Data p1Read();
int8_t getIsAtLeastDSMR5();
void setIsAtLeastDSMR5(int8_t dsmrVersion);


//JSON
void printP1Data(P1Data *data);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //ifndef _P1CONFIG_H