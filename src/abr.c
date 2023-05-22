#include "abr.h"
#include "P1Config.h"

#define INPUT_PIN 16
#define bufferSize 20

xQueueHandle interputQueue;

static void IRAM_ATTR gpio_interrupt_handler(void *args)
{
    long long int timeDeliverd = esp_timer_get_time();
    xQueueSendFromISR(interputQueue, &timeDeliverd, NULL);
}

void uartStartDetectBaudrate()
{    
    abrInit();
    //DRQ pin has inverter to pull up to 5V, which makes it active low:      
    gpio_set_level(PIN_DRQ, 0);
    //measure the baudrate

    int64_t timestamps[bufferSize];
    uint32_t baudrate = 0;

    ESP_LOGI("Timer", "Started timer, time since boot: %lld us", timestamps[0] = esp_timer_get_time());
    gpio_set_direction(INPUT_PIN, GPIO_MODE_INPUT);
    
    static int detected = 0;

    ESP_LOGI("Baudrate", "calculating baudrate");

    while (detected < bufferSize && (esp_timer_get_time() < (10000000 + timestamps[0]))) //10 seconds after boot
    {
        long long int timeDeliverd;
        if (xQueueReceive(interputQueue, &timeDeliverd, portTICK_PERIOD_MS))
        {
            ESP_LOGI("Uartdata","found %lld us", timeDeliverd);
            detected++;
            timestamps[detected] = timeDeliverd;
        }
        else
        {
            //ESP_LOGI("Baudrate", "waiting for uart signal");
        }
        
    }

    if(detected > 3)
    {
        for (uint8_t i = 0; i < detected; i++)
        {
            if(i < 1)
            {
                baudrate = timestamps[i+1]-timestamps[i];
            }
            else
            {
                if(timestamps[i+1]-timestamps[i] < baudrate)
                {
                    baudrate = timestamps[i+1]-timestamps[i];
                }
            }
        }
        
        ESP_LOGI("Baudrate", "found %d us", baudrate);
        if (baudrate < 100)// < 100 us
        {
            baudrate = 115200;
        }
        else if (baudrate >= 100 && baudrate < 1000)// > 100 us && < 1ms
        {
            baudrate = 9600;
        }
        else
        {
            baudrate = 0;
        }
        ESP_LOGI("Baudrate", "calculated %d", baudrate);
        uart_set_baudrate(P1PORT_UART_NUM, baudrate);
    }
    else
    {
        ESP_LOGI("Baudrate", "No baudrate detected, time: %lld", esp_timer_get_time());
    }
    uint32_t currentBaud = 0;
    uart_get_baudrate(P1PORT_UART_NUM, &currentBaud);
    ESP_LOGI("Baudrate", "%d in use", (currentBaud-1));
    //Write DRQ pin low again (otherwise P1 port keeps transmitting every second);
    gpio_set_level(PIN_DRQ, 1);
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
