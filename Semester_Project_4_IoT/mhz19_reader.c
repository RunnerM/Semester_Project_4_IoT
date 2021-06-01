/*
 * mhz19_reader.c
 *
 * Created: 23/05/2021 14.15.42
 *  Author: Gustaw
 */ 

#include "mhz19_reader.h"
#include <mh_z19.h>
#include <ATMEGA_FreeRTOS.h>
#include <stdint.h>

uint16_t _co2ppm;
uint16_t *_co2ppm_pointer;
mh_z19_returnCode_t rc;
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
