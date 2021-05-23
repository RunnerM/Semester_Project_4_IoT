/*
 * luxSensor_reader.c
 *
 * Created: 23/05/2021 14.58.27
 *  Author: pente
 */ 
#include "luxSensor_reader.h"
#include <tsl2591.h>

float _lux;

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

int read_lux(void* pvParameters){
	(void)pvParameters;
	int lux;
	for (;;) {
		if (TSL2591_OK == tsl2591_enable()) {
			if (TSL2591_OK == tsl2591_fetchData()) {
				if (TSL2591_DATA_READY == tsl2591_getLux(&_lux) {
					lux = *_lux;
					return lux;
				}
				if (TSL2591_OK == tsl2591_disable()) {
					// were powering down
				}
				else {
					// here be errors
				}
			}
			else {
				// here be errors
			}
		}
	}
}
// for init
if ( TSL2591_OK == tsl2591_initialise(tsl2591Callback) )
{
	// Driver initilised OK
	// Always check what tsl2591_initialise() returns
}
	
}
