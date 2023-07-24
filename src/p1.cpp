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

	if (result.dsmrVersion != P1_UNKNOWN)
	{
		std::string eTimestampStr(result.timeElecMeasurement);
		std::string gTimestampStr(result.timeElecMeasurement);

		// if(parseDsmrTimestamp(result.timeGasMeasurement, deviceTime()) != TIME_UNKNOWN)//if wrong gas values dont upload
		if(true)//if wrong gas values dont upload
		{
			Measurements::Measurement eMeterReadingSupplyLow("e_use_lo_cum__kWh", result.elecUsedT1, parseDsmrTimestamp(eTimestampStr, deviceTime()));
			secureUploadQueue.AddMeasurement(eMeterReadingSupplyLow);

			Measurements::Measurement eMeterReadingSupplyHigh("e_use_hi_cum__kWh", result.elecUsedT2, parseDsmrTimestamp(eTimestampStr, deviceTime()));
			secureUploadQueue.AddMeasurement(eMeterReadingSupplyHigh);

			Measurements::Measurement eMeterReadingReturnLow("e_ret_lo_cum__kWh", result.elecDeliveredT1, parseDsmrTimestamp(eTimestampStr, deviceTime()));
			secureUploadQueue.AddMeasurement(eMeterReadingReturnLow);

			Measurements::Measurement eMeterReadingReturnHigh("e_ret_hi_cum__kWh", result.elecDeliveredT2, parseDsmrTimestamp(eTimestampStr, deviceTime()));
			secureUploadQueue.AddMeasurement(eMeterReadingReturnHigh);

			Measurements::Measurement gMeterReadingSupply("g_use_cum__m3", result.gasUsage, parseDsmrTimestamp(gTimestampStr, deviceTime()));
			secureUploadQueue.AddMeasurement(gMeterReadingSupply);
		}
		else
		{
			ESP_LOGI("P1", "incorrect p1 message");
		}
	}
}

