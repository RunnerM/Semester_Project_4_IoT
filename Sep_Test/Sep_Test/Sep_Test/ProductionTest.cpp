#include "fff.h"
#include "pch.h"
DEFINE_FFF_GLOBALS;

extern "C"
{
#include <tsl2591.h>
#include <hih8120.h>
#include <mh_z19.h>
#include "hih8120_reader.h"
#include "luxSensor_reader.h"
#include "mhz19_reader.h"
#include "ATMEGA_FreeRTOS.h"
#include "task.h"

	// Declarations
	void read_lux();
	float get_lux_value();
	uint16_t read_CO2_ppm();
	hih8120results readValueAll();
	
}

// CREATE FAKE FUNCTIONS

/*
* Bridging type for creating a function pointer.
* Ref: https://gist.github.com/michahoiting/5976702
*/
typedef void (*cb)(tsl2591_returnCode_t);
typedef void (*cb_2)(hih8120_driverReturnCode_t);
typedef void (*cb_3)(mh_z19_returnCode_t);

FAKE_VALUE_FUNC(hih8120_driverReturnCode_t, hih8120_wakeup);
FAKE_VALUE_FUNC(hih8120_driverReturnCode_t, hih8120_measure);
FAKE_VALUE_FUNC(hih8120_driverReturnCode_t, hih8120_isReady);
FAKE_VALUE_FUNC(hih8120_driverReturnCode_t, hih8120_getHumidity);
FAKE_VALUE_FUNC(hih8120_driverReturnCode_t, hih8120_getTemperature);

FAKE_VALUE_FUNC(mh_z19_returnCode_t, mh_z19_takeMeassuring);
FAKE_VALUE_FUNC(mh_z19_returnCode_t, mh_z19_getCo2Ppm, uint16_t*);

FAKE_VALUE_FUNC(tsl2591_returnCode_t, tsl2591_initialise, cb);
FAKE_VALUE_FUNC(tsl2591_returnCode_t, tsl2591_enable);
FAKE_VALUE_FUNC(tsl2591_returnCode_t, tsl2591_fetchData);
FAKE_VALUE_FUNC(tsl2591_returnCode_t, tsl2591_getFullSpectrumRaw, uint16_t*);
FAKE_VALUE_FUNC(tsl2591_returnCode_t, tsl259_getVisibleRaw, uint16_t*);

// FreeRTOS fake functions
FAKE_VALUE_FUNC(BaseType_t, xTaskCreate, TaskFunction_t, const char*, configSTACK_DEPTH_TYPE, void*, UBaseType_t, TaskHandle_t*);
FAKE_VOID_FUNC(xTaskDelayUntil, TickType_t*, TickType_t);
FAKE_VALUE_FUNC(TickType_t, xTaskGetTickCount);

class ProductionTest :public::testing::Test
{
protected:
	void SetUp() override
	{
		RESET_FAKE(tsl2591_initialise);
		RESET_FAKE(tsl2591_enable);
		RESET_FAKE(tsl2591_fetchData);
		RESET_FAKE(tsl2591_getFullSpectrumRaw);
		RESET_FAKE(tsl259_getVisibleRaw);
		RESET_FAKE(xTaskCreate);
		RESET_FAKE(xTaskDelayUntil);
		RESET_FAKE(xTaskGetTickCount);
		RESET_FAKE(hih8120_wakeup);
		RESET_FAKE(hih8120_measure);
		RESET_FAKE(hih8120_isReady);
		RESET_FAKE(hih8120_getHumidity);
		RESET_FAKE(hih8120_getTemperature);
		RESET_FAKE(mh_z19_takeMeassuring);
		RESET_FAKE(mh_z19_getCo2Ppm);
		FFF_RESET_HISTORY();
	}
	void TearDown() override
	{

	}
};

TEST(Test_Light_Sensor_reader, Sensor_enabled)
{
	read_lux();
	tsl2591_initialise_fake.return_val = TSL2591_OK;

	EXPECT_EQ(1, tsl2591_enable_fake.call_count);
}

TEST(Test_Light_Sensor_reader, Driver_called)
{
	get_lux_value();

	EXPECT_EQ(1, tsl2591_fetchData_fake.call_count);
}

TEST(TestCO2SensorReader, reading) {
	uint16_t testresult= read_CO2_ppm();

	EXPECT_TRUE(testresult>0);
}

TEST(TestHIH8120SensorReader, reading) {
	hih8120results result = readValueAll();

	EXPECT_TRUE(0<result.hum<100);
	EXPECT_TRUE(0 < result.temp < 100);
}