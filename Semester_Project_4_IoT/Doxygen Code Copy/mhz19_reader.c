/**
* @file mhz19_reader.c
* CO2 Sensor reader class
*/

#include "mhz19_reader.h"
#include <mh_z19.h>
#include <ATMEGA_FreeRTOS.h>


///  Variable returned by read_CO2_ppm
uint16_t _co2ppm;

/// Pointer for read_CO2_ppm to write to
uint16_t *_co2ppm_pointer;

/// CO2 Sensor Return Code
mh_z19_returnCode_t rc;


/**
* Reads CO2 value from the sensor, and returns it
* @param[out] _co2ppm
*/
uint16_t read_CO2_ppm(){
	_co2ppm_pointer = &_co2ppm;
	
	for (;;)
	{
		rc = mh_z19_takeMeassuring();
		if (rc != MHZ19_OK) {
			puts("Error reading CO2 sensor");
		}
		else {
			for (;;)
			{
				if (MHZ19_NO_MEASSURING_AVAILABLE!=mh_z19_getCo2Ppm(_co2ppm_pointer))
				{
					return _co2ppm;
				}
			}
		}
	}
}
