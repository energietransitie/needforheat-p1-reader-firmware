//To create the JSON and read the P1 port
#include <string.h>
#include <p1.hpp>
#include <P1Config.h>
#include <dsmr_timestamp.hpp>
#include <dsmr_timestampTest.hpp>

//ESP-IDF drivers:
#include <scheduler.hpp>
#include <secure_upload.hpp>
#include <measurements.hpp>
#include <util/error.hpp> 

auto secureUploadQueue = SecureUpload::Queue::GetInstance();

//function to read P1 port and store message in a buffer
void readP1Task(void *taskInfo) {
	// Add formatters for all the measurements.
	Measurements::Measurement::AddFormatter("e_use_lo_cum__kWh", "%.3f");
	Measurements::Measurement::AddFormatter("e_use_hi_cum__kWh", "%.3f");
	Measurements::Measurement::AddFormatter("e_ret_lo_cum__kWh", "%.3f");
	Measurements::Measurement::AddFormatter("e_ret_hi_cum__kWh", "%.3f");
	Measurements::Measurement::AddFormatter("g_use_cum__m3", "%.3f"); 

	//offline testing for dsmr timestamps
	//parseDsmrTimestamp("230530161324S", deviceTime());


	//call this function if you want to use a wide variety of test cases
	//Go to the function to modify or change the cases
	//parsedTimestampsTests();

    P1Data result = p1Read();

	/**
	 * to test discard erroneous gas meter readings give a wrong value to result.timeGasMeasurment
	 * example:
	 * result.timeGasMeasurement = "632525252525S";
	*/

	if (result.dsmrVersion == P1_UNKNOWN) {
		ESP_LOGI("P1", "incorrect p1 message");
	} else {
		std::string eTimestampStr(result.timeElecMeasurement);
		std::string gTimestampStr(result.timeGasMeasurement);
		time_t eTimestamp = parseDsmrTimestamp(LAST_E_TIMESTAMP, eTimestampStr, deviceTime());
		time_t gTimestamp = parseDsmrTimestamp(LAST_G_TIMESTAMP, gTimestampStr, deviceTime());

		if  (result.dsmrVersion < 4.0  || eTimestamp == TIME_UNKNOWN) {
			// for smart meters with version DSMR 2 and 3, no timestamps are available for the electricity meter readings; 
			// also, if for some reason, timestamp parsing for DSMR4 and up is not going ok, then upload with device timestamps (second best for accuracy) 
			// so, make the Measurement use the device timetamp by NOT specifying a timestamp
			Measurements::Measurement e_use_lo_cum__kWh("e_use_lo_cum__kWh", result.elecUsedT1);
			Measurements::Measurement e_use_hi_cum__kWh("e_use_hi_cum__kWh", result.elecUsedT2);
			Measurements::Measurement e_ret_lo_cum__kWh("e_ret_lo_cum__kWh", result.elecReturnedT1);
			Measurements::Measurement e_ret_hi_cum__kWh("e_ret_hi_cum__kWh", result.elecReturnedT2);
			secureUploadQueue.AddMeasurement(e_use_lo_cum__kWh);
			secureUploadQueue.AddMeasurement(e_use_hi_cum__kWh);
			secureUploadQueue.AddMeasurement(e_ret_lo_cum__kWh);
			secureUploadQueue.AddMeasurement(e_ret_hi_cum__kWh);
		} else {
			// for smart meters with version DSMR 4 and up, up timestamps are availeble for the electricity meter readings;
			// so, make the Measurement use the smart meter timetamp by specifying its parsed timestamp
			Measurements::Measurement e_use_lo_cum__kWh("e_use_lo_cum__kWh", result.elecUsedT1, eTimestamp);
			Measurements::Measurement e_use_hi_cum__kWh("e_use_hi_cum__kWh", result.elecUsedT2, eTimestamp);
			Measurements::Measurement e_ret_lo_cum__kWh("e_ret_lo_cum__kWh", result.elecReturnedT1, eTimestamp);
			Measurements::Measurement e_ret_hi_cum__kWh("e_ret_hi_cum__kWh", result.elecReturnedT2, eTimestamp);
			secureUploadQueue.AddMeasurement(e_use_lo_cum__kWh);
			secureUploadQueue.AddMeasurement(e_use_hi_cum__kWh);
			secureUploadQueue.AddMeasurement(e_ret_lo_cum__kWh);
			secureUploadQueue.AddMeasurement(e_ret_hi_cum__kWh);
		}
		if(gTimestamp != TIME_UNKNOWN) {
			// only upload gas meter values if the timestamp is proper (e.g., not when result.timeGasMeasurement == "632525252525S")
			Measurements::Measurement g_use_cum__m3("g_use_cum__m3", result.gasUsage, gTimestamp);
			secureUploadQueue.AddMeasurement(g_use_cum__m3);
		} else if (result.dsmrVersion >= 5.0) {
			// for DSMR5, device timestamps are at most 5 minutes old; using device timestamps as second best seems acceptible
			// for DSMR4 and lower, they can be an hour old; using device timestamps would require too much post processing 
			Measurements::Measurement g_use_cum__m3("g_use_cum__m3", result.gasUsage);
			secureUploadQueue.AddMeasurement(g_use_cum__m3);
		}
	}
}

