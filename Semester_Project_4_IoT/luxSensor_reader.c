/*
 * luxSensor_reader.c
 *
 * Created: 23/05/2021 14.58.27
 *  Author: pente
 */ 
#include "luxSensor_reader.h"
#include <tsl2591.h>
#include <stdio.h>

#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <event_groups.h>

void aFunctionToSetBits( EventGroupHandle_t xEventGroup , int bit_No);
EventGroupHandle_t getEventGroup();

float *_lux;

void tsl2591Callback(tsl2591_returnCode_t rc){
	{
		uint16_t _tmp;
		float _lux;
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
				printf("Lux: %5.4f\n", _lux);
				//here we can read.
				
				aFunctionToSetBits(getEventGroup(), 2);
				printf("1\n");
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

void read_lux(void* pvParameters){
	(void)pvParameters;	
	tsl2591_fetchData();
	
	}
	
float get_lux_value(){
		return *_lux;
	}	
