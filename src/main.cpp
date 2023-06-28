#include <generic_esp_32.hpp>
#include <generic_tasks.hpp>
#include <scheduler.hpp>
#include <p1.hpp>

constexpr const char *DEVICE_TYPE_NAME = "twomes-p1-reader-firmware";

extern "C" void app_main(void)
{
	#ifdef CONFIG_TWOMES_STRESS_TEST
		Scheduler::AddTask(readP1Task, "uart_read_p1", 16384, NULL, 10, Scheduler::Interval::MINUTES_1);
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
