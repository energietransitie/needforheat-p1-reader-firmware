
#include <generic_esp_32.hpp>
#include <generic_tasks.hpp>
#include <scheduler.hpp>
#include <p1.hpp>
#include <util/buttons.hpp>
#include <util/error.hpp>

#ifdef CONFIG_PERFORM_UNIT_TESTS
#include <dsmr_timestampTest.hpp>
#endif

constexpr const char *DEVICE_TYPE_NAME = "twomes-p1-reader-firmware";

constexpr const gpio_num_t BUTTON_WIFI_RESET = GPIO_NUM_12;
constexpr const gpio_num_t LED_WIFI_RESET = GPIO_NUM_14;

extern "C" void app_main(void)
{
#ifdef CONFIG_PERFORM_UNIT_TESTS
	performTestSequence();

	/**
	 * to test discard erroneous gas meter readings give a wrong value to result.timeGasMeasurment
	 * example:
	 * result.timeGasMeasurement = "632525252525S";
	 */

#endif

	// Configure Wi-Fi reset button.
	gpio_config_t gpioConfigIn = {};
	gpioConfigIn.intr_type = GPIO_INTR_NEGEDGE;
	gpioConfigIn.mode = GPIO_MODE_INPUT;
	gpioConfigIn.pin_bit_mask = 1 << BUTTON_WIFI_RESET;
	gpioConfigIn.pull_down_en = GPIO_PULLDOWN_DISABLE;
	gpioConfigIn.pull_up_en = GPIO_PULLUP_ENABLE;
	gpio_config(&gpioConfigIn);

	auto err = Buttons::ButtonPressHandler::AddButton(BUTTON_WIFI_RESET, "Wi-Fi reset", 0, nullptr, GenericESP32Firmware::ResetWireless);
	Error::CheckAppendName(err, "Main", "An error occured when adding BUTTON_WIFI_RESET to handler");

	// Configure Wi-Fi reset LED.
	gpio_config_t gpioConfigOut = {};
	gpioConfigOut.intr_type = GPIO_INTR_DISABLE;
	gpioConfigOut.mode = GPIO_MODE_OUTPUT;
	gpioConfigOut.pin_bit_mask = 1 << LED_WIFI_RESET;
	gpio_config(&gpioConfigOut);

	GenericESP32Firmware::SetResetWirelessLED(LED_WIFI_RESET);

#ifdef CONFIG_TWOMES_STRESS_TEST
	Scheduler::AddTask(readP1Task, "uart_read_p1", 16384, NULL, 10, Scheduler::Interval::MINUTES_2);
#else
	Scheduler::AddTask(readP1Task, "uart_read_p1", 16384, NULL, 10, Scheduler::Interval::MINUTES_10);
#endif

	// Add the generic tasks to the scheduler,
	// heartbeat, timesync and optionally presence detection and OTA updates.
	GenericTasks::AddTasksToScheduler();

	GenericESP32Firmware::Initialize(DEVICE_TYPE_NAME);

	// Start the scheduler.
	Scheduler::Start();
}
