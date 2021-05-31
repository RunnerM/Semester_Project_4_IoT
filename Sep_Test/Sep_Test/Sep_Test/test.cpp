#include "gtest/gtest.h"
#include "fff.h"
#include "pch.h"
DEFINE_FFF_GLOBALS;

extern "C"
{
#include <tsl2591.h>
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

FAKE_VALUE_FUNC(tsl2591_returnCode_t, tsl2591_initialise, cb);
FAKE_VALUE_FUNC(tsl2591_returnCode_t, tsl2591_enable);
FAKE_VALUE_FUNC(tsl2591_returnCode_t, tsl2591_fetchData);
FAKE_VALUE_FUNC(tsl2591_returnCode_t, tsl2591_getFullSpectrumRaw, uint16_t*);
FAKE_VALUE_FUNC(tsl2591_returnCode_t, tsl259_getVisibleRaw, uint16_t*);

// FreeRTOS fake functions
FAKE_VALUE_FUNC(BaseType_t, xTaskCreate, TaskFunction_t, const char*, configSTACK_DEPTH_TYPE, void*, UBaseType_t, TaskHandle_t*);
FAKE_VOID_FUNC(xTaskDelayUntil, TickType_t*, TickType_t);
FAKE_VALUE_FUNC(TickType_t, xTaskGetTickCount);

class Test_LightTask :public::testing::Test
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
		FFF_RESET_HISTORY();
	}
	void TearDown() override
	{

	}
};

TEST_F(Test_LightTask, Test_if_sensor_is_enabled)
{
	read_lux();
	tsl2591_initialise_fake.return_val = TSL2591_OK;

	EXPECT_EQ(1, tsl2591_enable_fake.call_count);
}

TEST_F(Test_LightTask, Test_if_light_driver_is_called) 
{
	get_lux_value();

	EXPECT_EQ(1, tsl2591_fetchData_fake.call_count);
}