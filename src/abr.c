#include "abr.h"
#include "P1Config.h"

#define INPUT_PIN 16
#define bufferSize 20
#define BIT_INTERVAL_THRESHOLD__us ((1e6 / 9600 + 1e6 / 115200) / 2)

xQueueHandle interputQueue;

static void IRAM_ATTR gpio_interrupt_handler(void *args)
{
    long long int timeDelivered = esp_timer_get_time();
    xQueueSendFromISR(interputQueue, &timeDelivered, NULL);
}

// gets baudrate to use from nvs
int32_t getBaudrate__b_s_1()
{    
    nvs_handle_t nvsStorage;
    esp_err_t err;
    int32_t baudrate__b_s_1 = P1_UNKNOWN;
    err = nvs_open("storage", NVS_READWRITE, &nvsStorage);
    if (err != ESP_OK) {
        ESP_LOGD("NVS","Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        ESP_LOGD("NVS","Opened");

        ESP_LOGD("NVS","Reading %s value from NVS ... ", NVS_KEY_BAUDRATE);

        err = nvs_get_i32(nvsStorage, NVS_KEY_BAUDRATE, &baudrate__b_s_1);
        switch (err) {
            case ESP_OK:
                ESP_LOGD("NVS","Done\n");
                ESP_LOGD("NVS","%s value read: %" PRId32 "\n", NVS_KEY_BAUDRATE, baudrate__b_s_1);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGD("NVS","%s value is not initialized yet!\n", NVS_KEY_BAUDRATE);
                baudrate__b_s_1 = P1_UNKNOWN;
                break;
            default :
               ESP_LOGD("NVS","Error (%s) reading!\n", esp_err_to_name(err));
        }
    
        // Close
        nvs_close(nvsStorage);
    }
    return baudrate__b_s_1;
}

// detects baudrate 
int32_t detectBaudrate__b_s_1()
{    
    abrInit();
    //DRQ pin has inverter to pull up to 5V, which makes it active low:      
    gpio_set_level(PIN_DRQ, 0);
    //measure the baudrate

    int64_t bitFlankTimestamps__us[bufferSize];
    int32_t bitFlankInterval__us = 0;
    int32_t smallestBitFlankInterval__us = 0;
    int32_t baudrate__b_s_1 = P1_UNKNOWN;

    bitFlankTimestamps__us[0] = esp_timer_get_time();
    ESP_LOGI("detectBaudrate__b_s_1", "Started timer %.6f since boot", bitFlankTimestamps__us[0]/1e6);
    gpio_set_direction(INPUT_PIN, GPIO_MODE_INPUT);

    int detected = 0;

    ESP_LOGI("detectBaudrate__b_s_1", "detecting bit flanks...");

    while (detected < bufferSize && (esp_timer_get_time() < (5*1e6 + bitFlankTimestamps__us[0]))) //5 seconds after boot
    {
        long long int timeDelivered;
        if (xQueueReceive(interputQueue, &timeDelivered, portTICK_PERIOD_MS))
        {
            detected++;
            bitFlankTimestamps__us[detected] = timeDelivered;
        }
    }


    if(detected > 10)
    {
        smallestBitFlankInterval__us = bitFlankTimestamps__us[1]-bitFlankTimestamps__us[0];
        ESP_LOGI("detectBaudrate__b_s_1", "Bit flank interval found: %d us", bitFlankInterval__us);
        for (uint8_t i = 1; i < detected; i++)
        {
            bitFlankInterval__us = bitFlankTimestamps__us[i+1]-bitFlankTimestamps__us[i];
            ESP_LOGI("detectBaudrate__b_s_1", "Bit flank interval found: %d us", bitFlankInterval__us);
            if(bitFlankInterval__us < smallestBitFlankInterval__us)
            {
                smallestBitFlankInterval__us = bitFlankInterval__us;
            }
        }

        ESP_LOGI("detectBaudrate__b_s_1", "Smallest bit flank interval found: %d us", smallestBitFlankInterval__us);
        
        int32_t candidateBaudRates__b_s_1[3] = {9600, 115200, 0}; // must be 0-terminated!
        baudrate__b_s_1 = findNearestBaudRate__b_s_1(candidateBaudRates__b_s_1, smallestBitFlankInterval__us);
        ESP_LOGI("detectBaudrate__b_s_1", "Baudrate found: %d b/s", baudrate__b_s_1);

    }
    else
    {
        ESP_LOGI("detectBaudrate__b_s_1", "No baudrate detected until %.6f s since timer start", (esp_timer_get_time()-bitFlankTimestamps__us[0])/1e6);
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

int32_t calculateBitInterval__us(int32_t baudrate__b_s_1)
{
    return 1e6 / baudrate__b_s_1;
}

// candidateBaudRates__b_s_1 must be 0-terminated!
int32_t findNearestBaudRate__b_s_1(int32_t * candidateBaudRates__b_s_1, int32_t smallestBitFlankInterval__us)
{

    int32_t nearest__b_s_1 = candidateBaudRates__b_s_1[0];
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

// sets the UART config for the P1 port according to the baudrate__b_s_1
// stores baudrate__b_s_1 (removes the variable when baudrate__b_s_1 == P1_UNKNOWN)
// only performs writes on nvs if the information to be stored is different from what is aloready stored
void setBaudrate__b_s_1(int32_t baudrate__b_s_1)
{
    nvs_handle_t nvsStorage;
    esp_err_t err;
    
    // set the UART config for the P1 port according to the baudrate__b_s_1
    switch (baudrate__b_s_1) {
        case 9600:
            ESP_LOGI("UART_SETUP", "Setting UART config based on baudrate: %d b/s", (baudrate__b_s_1));
            setP1UARTConfigDSMR2or3();
            break;
        case 115200:
            ESP_LOGI("UART_SETUP", "Setting UART config based on baudrate: %d b/s", (baudrate__b_s_1));
            setP1UARTConfigDSMR4or5();
            break;
        case P1_UNKNOWN:
            ESP_LOGD("UART_SETUP", "P1 baudrate unknown; setting UART to safest mode for detection");
            setP1UARTConfigDSMR4or5();
            break;
        default:
            ESP_LOGE("UART_SETUP", "No P1 UART config available for this baudrate.");
    }

    // store baudrate__b_s_1 in nvs if different from stored
    int32_t baudrate_stored__b_s_1 = getBaudrate__b_s_1();
    ESP_LOGI("P1", "P1 baudrate stored: %d; new: %d", baudrate_stored__b_s_1, baudrate__b_s_1);
    if(baudrate__b_s_1 != baudrate_stored__b_s_1)
    {
        // write value to nvs
        err = nvs_open("storage", NVS_READWRITE, &nvsStorage);
        if (err != ESP_OK) {
            ESP_LOGD("NVS","Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        } else {
            ESP_LOGD("NVS","Opened");

            if(baudrate__b_s_1 == P1_UNKNOWN)
            {
                err = nvs_erase_key(nvsStorage, NVS_KEY_BAUDRATE);
                ESP_LOGD("NVS","The value %ss is removed from nvs\n", NVS_KEY_BAUDRATE);
            }
            else
            {
                err = nvs_set_i32(nvsStorage, NVS_KEY_BAUDRATE, baudrate__b_s_1);                 
                ESP_LOGD("NVS","For key %s value % is stored in nvs\n", NVS_KEY_BAUDRATE, baudrate__b_s_1);
            }

            // Commit written value.
            // After setting any values, nvs_commit() must be called to ensure changes are written
            // to flash storage. Implementations may write to storage at other times,
            // but this is not guaranteed.
            ESP_LOGD("NVS", "Committing updates in NVS ... ");
            err = nvs_commit(nvsStorage);
        }
        // Close
        nvs_close(nvsStorage);
    }
}