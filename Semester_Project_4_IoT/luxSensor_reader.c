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
				printf("\nFull Raw:%04X\n", _tmp);
			}
			else if (TSL2591_OVERFLOW == rc)
			{
				printf("\nFull spectrum overflow - change gain and integration time\n");
			}

			if (TSL2591_OK == (rc = tsl259_getVisibleRaw(&_tmp)))
			{
				printf("Visible Raw:%04X\n", _tmp);
			}
			else if (TSL2591_OVERFLOW == rc)
			{
				printf("Visible overflow - change gain and integration time\n");
			}

			if (TSL2591_OK == (rc = tsl2591_getInfraredRaw(&_tmp)))
			{
				printf("Infrared Raw:%04X\n", _tmp);
			}
			else if (TSL2591_OVERFLOW == rc)
			{
				printf("Infrared overflow - change gain and integration time\n");
			}

			if (TSL2591_OK == (rc = tsl2591_getLux(&_lux)))
			{
				printf("Lux: %5.4f\n", _lux);
			}
			else if (TSL2591_OVERFLOW == rc)
			{
				printf("Lux overflow - change gain and integration time\n");
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

float read_lux(void* pvParameters){
	(void)pvParameters;
	tsl2591_returnCode_t rc;
	puts("1");
	float lux;
	puts("2");
	for (;;) {
		//puts("loop");
			if (TSL2591_OK == tsl2591_enable()) {
				//puts("3");
				for(;;){
					
				}
				if (TSL2591_OK == tsl2591_fetchData()) {
					puts("4");
					if (TSL2591_DATA_READY == tsl2591_getLux(&_lux)) {
						lux = *_lux;
						puts("5");
						return lux;
						
					}
					if (TSL2591_OK == tsl2591_disable()) {
						// were powering down
						puts("6");
					}
					else {
						// here be errors
						puts("7");
					}
				}
				else {
					// here be errors
					//puts("8");
					rc = tsl2591_fetchData();
					printf("%s", rc);
					
				}
			}
		}
	}	
