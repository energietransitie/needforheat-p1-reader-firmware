#include "abr.h"
#include "P1Config.h"

#define INPUT_PIN 16
#define bufferSize 20
#define BIT_INTERVAL_THRESHOLD__us ((1e6 / 9600 + 1e6 / 115200) / 2)
#define USE_NEAREST_BAUDRATE

xQueueHandle interputQueue;

static void IRAM_ATTR gpio_interrupt_handler(void *args)
{
    long long int timeDelivered = esp_timer_get_time();
    xQueueSendFromISR(interputQueue, &timeDelivered, NULL);
}

uint32_t uartStartDetectBaudrate()
{    
    abrInit();
    //DRQ pin has inverter to pull up to 5V, which makes it active low:      
    gpio_set_level(PIN_DRQ, 0);
    //measure the baudrate

    int64_t bitFlankTimestamps__us[bufferSize];
    uint32_t bitFlankInterval__us = 0;
    uint32_t smallestBitFlankInterval__us = 0;
    uint32_t baudrate__b_s_1 = 0;

    ESP_LOGI("Timer", "Started timer, time since boot: %lld us", bitFlankTimestamps__us[0] = esp_timer_get_time());
    gpio_set_direction(INPUT_PIN, GPIO_MODE_INPUT);

    static int detected = 0;

    ESP_LOGI("Baudrate", "calculating baudrate");

    while (detected < bufferSize && (esp_timer_get_time() < (5000000 + bitFlankTimestamps__us[0]))) //10 seconds after boot
    {
        long long int timeDelivered;
        if (xQueueReceive(interputQueue, &timeDelivered, portTICK_PERIOD_MS))
        {
            ESP_LOGI("Bit flank detected at","%lld us since boot", timeDelivered);
            detected++;
            bitFlankTimestamps__us[detected] = timeDelivered;
        }
        else
        {
            //ESP_LOGI("Baudrate", "waiting for uart signal");
        }
        
    }


    if(detected > 10)
    {
        smallestBitFlankInterval__us = bitFlankTimestamps__us[1]-bitFlankTimestamps__us[0];
        ESP_LOGI("Bit flank interval found", "%d us", bitFlankInterval__us);
        for (uint8_t i = 1; i < detected; i++)
        {
            bitFlankInterval__us = bitFlankTimestamps__us[i+1]-bitFlankTimestamps__us[i];
            ESP_LOGI("Bit flank interval found", "%d us", bitFlankInterval__us);
            if(bitFlankInterval__us < smallestBitFlankInterval__us)
            {
                smallestBitFlankInterval__us = bitFlankInterval__us;
            }
        }

        ESP_LOGI("Smallest bit flank interval found", "%d us", smallestBitFlankInterval__us);
        
        #ifdef USE_NEAREST_BAUDRATE
            uint32_t candidateBaudRates__b_s_1[3] = {9600, 115200, 0}; // must be 0-terminated!
            baudrate__b_s_1 = findNearestBaudRate__b_s_1(candidateBaudRates__b_s_1, smallestBitFlankInterval__us);
            ESP_LOGI("Baudrate found using find nearest:", "%d b/s", baudrate__b_s_1);
        #else
            baudrate__b_s_1 = decideBaudrate__b_s_1(smallestBitFlankInterval__us);
            ESP_LOGI("Baudrate found using decide algorithm:", "%d b/s", baudrate__b_s_1);
        #endif

        ESP_LOGI("Baudrate to set:", "%d b/s", (baudrate__b_s_1));
        switch (baudrate__b_s_1) {
            case 9600:
                setP1UARTConfigDSMR2or3();
                break;
            case 115200:
                setP1UARTConfigDSMR4or5();
                break;
            default:
                ESP_LOGE("UART_SETUP", "Invalid baud rate specified");
        }
       
    }
    else
    {
        ESP_LOGI("Baudrate", "No baudrate detected until: %lld us since boot", esp_timer_get_time());
    }
    //Write DRQ pin low again (otherwise P1 port keeps transmitting every second);
    gpio_set_level(PIN_DRQ, 1);

    return baudrate__b_s_1;
}

void abrInit()
{
    gpio_pad_select_gpio(INPUT_PIN);
    gpio_set_direction(INPUT_PIN, GPIO_MODE_INPUT);
    gpio_pulldown_en(INPUT_PIN);
    gpio_pullup_dis(INPUT_PIN);
    gpio_set_intr_type(INPUT_PIN, GPIO_INTR_NEGEDGE);

    interputQueue = xQueueCreate(bufferSize, sizeof(int));

    //gpio_install_isr_service(0);
    gpio_isr_handler_add(INPUT_PIN, gpio_interrupt_handler, (void *)INPUT_PIN);
}

uint32_t decideBaudrate__b_s_1(uint32_t smallestBitFlankInterval__us)
{
    uint32_t bitIntervalThreshold__us = BIT_INTERVAL_THRESHOLD__us;

    if (smallestBitFlankInterval__us < bitIntervalThreshold__us)
    {
        return 115200;
    }
    else if (smallestBitFlankInterval__us >= bitIntervalThreshold__us && smallestBitFlankInterval__us < 1000)//< 1ms
    {
        return 9600;
    }
    else
    {
        return 0;
    }
}

uint32_t calculateBitInterval__us(uint32_t baudrate__b_s_1)
{
    return 1e6 / baudrate__b_s_1;
}

// candidateBaudRates__b_s_1 must be 0-terminated!
uint32_t findNearestBaudRate__b_s_1(uint32_t * candidateBaudRates__b_s_1, uint32_t smallestBitFlankInterval__us)
{

    uint32_t nearest__b_s_1 = candidateBaudRates__b_s_1[0];
    int smallestDifference__us = (smallestBitFlankInterval__us - calculateBitInterval__us(candidateBaudRates__b_s_1[0]));
    if(smallestDifference__us < 0) {
        smallestDifference__us = smallestDifference__us * -1;
    }
    ESP_LOGI("Baudrate", "%d b/s", candidateBaudRates__b_s_1[0]);
    ESP_LOGI("Difference", "%d us\n", smallestDifference__us);

    for (uint8_t i = 1; candidateBaudRates__b_s_1[i] != 0 ; i++)
    {
        int difference__us = (smallestBitFlankInterval__us - calculateBitInterval__us(candidateBaudRates__b_s_1[i]));
        if(difference__us < 0) {
            difference__us = difference__us * -1;
        }
        ESP_LOGI("Baudrate", "%d b/s", candidateBaudRates__b_s_1[i]);
        ESP_LOGI("Difference", "%d us\n", difference__us);
        if (difference__us < smallestDifference__us)
        {
            nearest__b_s_1 = candidateBaudRates__b_s_1[i];
            smallestDifference__us = difference__us;
        }
    }
    ESP_LOGI("Nearest baudrate:", "%d b/s", nearest__b_s_1);
    return nearest__b_s_1;
}

void updateOrDetectBaudrate(uint8_t detect)
{
    nvs_handle_t nvsStorage;
    esp_err_t err;
    uint32_t P1baudrate = 0; // boolean
    err = nvs_open("storage", NVS_READWRITE, &nvsStorage);
    if (err != ESP_OK) {
        ESP_LOGD("NVS","Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        ESP_LOGD("NVS","Opened");

        ESP_LOGD("NVS","Reading P1baudrate from NVS ... ");

        err = nvs_get_u32(nvsStorage, "P1baudrate", &P1baudrate);
        switch (err) {
            case ESP_OK:
                ESP_LOGD("NVS","Done\n");
                ESP_LOGD("NVS","P1baudrate = %" PRIu32 "\n", P1baudrate);
                switch (P1baudrate) {
                case 9600:
                    setP1UARTConfigDSMR2or3();
                    break;
                case 115200:
                    setP1UARTConfigDSMR4or5();
                    break;
                }
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGD("NVS","The value is not initialized yet!\n");
                break;
            default :
               ESP_LOGD("NVS","Error (%s) reading!\n", esp_err_to_name(err));
        }

        if(detect || err == ESP_ERR_NVS_NOT_FOUND)
        {
            P1baudrate = uartStartDetectBaudrate();
            
            if(P1baudrate){

                err = nvs_set_u32(nvsStorage, "P1baudrate", P1baudrate);

                // Commit written value.
                // After setting any values, nvs_commit() must be called to ensure changes are written
                // to flash storage. Implementations may write to storage at other times,
                // but this is not guaranteed.
                ESP_LOGD("NVS", "Committing updates in NVS ... ");
                err = nvs_commit(nvsStorage);
            }
        }
    
        // Close
        nvs_close(nvsStorage);
    }

   

}