//To create the JSON and read the P1 port
#include <string.h>
#include <p1.hpp>
#include <P1Config.h>
#include <timestamp.hpp>

//ESP-IDF drivers:
#include <scheduler.hpp>
#include <secure_upload.hpp>
#include <measurements.hpp>
#include <util/error.hpp> 

auto secureUploadQueue = SecureUpload::Queue::GetInstance();

//function to read P1 port and store message in a buffer
void readP1Task(void *taskInfo) {
	// Add formatters for all the measurements.
	Measurements::Measurement::AddFormatter("e_use_lo_cum__kWh", "%4.3f");
	Measurements::Measurement::AddFormatter("e_use_hi_cum__kWh", "%4.3f");
	Measurements::Measurement::AddFormatter("e_ret_lo_cum__kWh", "%4.3f");
	Measurements::Measurement::AddFormatter("e_ret_hi_cum__kWh", "%4.3f");
	Measurements::Measurement::AddFormatter("g_use_cum__m3", "%7.3f"); 

	//ofline testing
	//timestampFormatted("230524100224S");

    P1Data result = p1Read();

	Measurements::Measurement eMeterReadingSupplyLow("e_use_lo_cum__kWh", result.elecUsedT1, timestampFormatted(result.timeElecMeasurement));
	secureUploadQueue.AddMeasurement(eMeterReadingSupplyLow);

    Measurements::Measurement eMeterReadingSupplyHigh("e_use_hi_cum__kWh", result.elecUsedT2, timestampFormatted(result.timeElecMeasurement));
	secureUploadQueue.AddMeasurement(eMeterReadingSupplyHigh);

    Measurements::Measurement eMeterReadingReturnLow("e_ret_lo_cum__kWh", result.elecDeliveredT1, timestampFormatted(result.timeElecMeasurement));
	secureUploadQueue.AddMeasurement(eMeterReadingReturnLow);

    Measurements::Measurement eMeterReadingReturnHigh("e_ret_hi_cum__kWh", result.elecDeliveredT2, timestampFormatted(result.timeElecMeasurement));
	secureUploadQueue.AddMeasurement(eMeterReadingReturnHigh);

    Measurements::Measurement gMeterReadingSupply("g_use_cum__m3", result.gasUsage, timestampFormatted(result.timeGasMeasurement));
	secureUploadQueue.AddMeasurement(gMeterReadingSupply);

}

