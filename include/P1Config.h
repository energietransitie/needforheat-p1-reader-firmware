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


#define JSON_BUFFER_SIZE 2048

//WIFI Scan
#define DEFAULT_SCAN_LIST_SIZE 25   //Amount of APs to scan
#define AMOUNT_WIFI_CHANNELS 13     //Amount of available WIFI channels

//MEASUREMENT TYPE STRUCTS:
#define MAX_BOILER_SAMPLES 60
typedef struct Boiler_message {
    uint8_t measurementType;  //Type of measurements
    uint8_t numberofMeasurements;                       //number of measurements in burst
    uint16_t index;                                     //Number identifying the message, only increments on receiving an ACK from Gateway. Could be uint8_t since overflows are ignored?
    uint16_t intervalTime;               //Interval between measurements, for timestamping in gateway
    uint16_t pipeTemps1[MAX_BOILER_SAMPLES];             //measurements of the first temperature sensor
    uint16_t pipeTemps2[MAX_BOILER_SAMPLES];             //measurements of the second temperature sensor
} Boiler_message;
#define MAX_TEMP_SAMPLES 120
typedef struct Roomtemp_Message {
    uint8_t measurementType;                            //Type of measurements
    uint8_t numberofMeasurements;                       //number of measurements in burst
    uint16_t index;                                     //Number identifying the message, only increments on receiving an ACK from Gateway. Could be uint8_t since overflows are ignored?
    uint16_t intervalTime;                              //Interval between measurements, for timestamping in gateway
    uint16_t roomTemps[MAX_TEMP_SAMPLES];                //measurements of the Si7051
} Roomtemp_Message;
#define MAX_CO2_SAMPLES 40
typedef struct CO2_Message {
    uint8_t measurementType;                            //Type of measurements
    uint8_t numberofMeasurements;                       //number of measurements in burst
    uint16_t index;                                     //Number identifying the message, only increments on receiving an ACK from Gateway. Could be uint8_t since overflows are ignored?
    uint16_t intervalTime;                              //Interval between measurements, for timestamping in gateway
    uint16_t co2ppm[MAX_CO2_SAMPLES];                    //measurements of the CO2 concentration
    uint16_t co2temp[MAX_CO2_SAMPLES];                   //measurements of the temperature by SCD41
    uint16_t co2humid[MAX_CO2_SAMPLES];                  //measurements of the humidity
} CO2_Message;

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
    int8_t dsmrVersion;          // DSMR version without decimal point
    double elecUsedT1;           // Electrical Energy used Tariff 1 in kWh
    double elecUsedT2;           // Electrical Energy used Tariff 2 in kWh
    double elecDeliveredT1;      // Electrical Delivered used Tariff 1 in kWh
    double elecDeliveredT2;      // Electrical Delivered used Tariff 2 in kWh
    char timeElecMeasurement[14]; //Timestamp for most recent Electricity measurement
    double gasUsage;             // Gasverbruik in dm3
    char timeGasMeasurement[14]; // Timestamp for most recent gas measurement YY:MM:DD:HH:MM:SS And S/W for summer or winter time
} P1Data;



//For getting channel list and amount of channels:
typedef struct channelListstruct {
    uint8_t amount;
    uint8_t channels[DEFAULT_SCAN_LIST_SIZE];
}channelList;





/** ====== GLOBAL VARIABLES ============== */
// uint16_t wifiQueue = 0;

/**
 *  ========== FUNCTIONS ================
 */

 //Init

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