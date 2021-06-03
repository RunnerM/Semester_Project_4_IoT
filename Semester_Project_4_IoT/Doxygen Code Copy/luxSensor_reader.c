/**
* @file luxSensor_reader.c
* Lux Sensor reader class
*/

#include "luxSensor_reader.h"
#include <tsl2591.h>
#include <stdio.h>

#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <event_groups.h>

/**
* Function to set event bits in the system
* @param[in] xEventGroup
* @param[in] bit_No
*/
void aFunctionToSetBits( EventGroupHandle_t xEventGroup , int bit_No);

/// Get event group
EventGroupHandle_t getEventGroup();

/// CO2 Sensor Return Code
float _lux;

/**
* Callback function injected into the sensor at runtime start
* @parma[in] rc
*/
void tsl2591Callback(tsl2591_returnCode_t rc){
	{
		uint16_t _tmp;
		switch (rc)
		{
			case TSL2591_DATA_READY:
			if (TSL2591_OK == (rc = tsl2591_getFullSpectrumRaw(&_tmp)))
			{
				//printf("\nFull Raw:%04X\n", _tmp);
			}
			else if (TSL2591_OVERFLOW == rc)
			{
				//printf("\nFull spectrum overflow - change gain and integration time\n");
			}

			if (TSL2591_OK == (rc = tsl259_getVisibleRaw(&_tmp)))
			{
				//printf("Visible Raw:%04X\n", _tmp);
			}
			else if (TSL2591_OVERFLOW == rc)
			{
				//printf("Visible overflow - change gain and integration time\n");
			}

			if (TSL2591_OK == (rc = tsl2591_getInfraredRaw(&_tmp)))
			{
				//printf("Infrared Raw:%04X\n", _tmp);
			}
			else if (TSL2591_OVERFLOW == rc)
			{
				//printf("Infrared overflow - change gain and integration time\n");
			}

			if (TSL2591_OK == (rc = tsl2591_getLux(&_lux)))
			{
				//printf("Lux: %5.4f\n", _lux);
				//here we can read.
				
				aFunctionToSetBits(getEventGroup(), 2);
				//printf("1\n");
			}
			else if (TSL2591_OVERFLOW == rc)
			{
				//printf("Lux overflow - change gain and integration time\n");
			}
			break;

			case TSL2591_OK:
			// Last command performed successful
			break;

			case TSL2591_DEV_ID_READY:
			// Dev ID now fetched
			break;

			default:
			break;
		}
	}
	
}


/**
* Function calling Lux sensor to retrieve data
*/
void read_lux(){	
	tsl2591_fetchData();
}

/**
* Function calling Lux sensor to retrieve data
* @param[out] _lux
*/
float get_lux_value(){
		return _lux;
	}	
