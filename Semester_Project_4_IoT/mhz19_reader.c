/*
 * mhz19_reader.c
 *
 * Created: 23/05/2021 14.15.42
 *  Author: Gustaw
 */ 

#include "mhz19_reader.h"
#include <mh_z19.h>

uint16_t *_co2ppm;
uint16_t read_CO2_ppm(){
	mh_z19_returnCode_t rc;
	for (;;)
	{
		puts("Reading MHZ19\n");
		rc = mh_z19_takeMeassuring();
		if (rc != MHZ19_OK) {
			puts("Error reading CO2 sensor");
		}
		else {
			mh_z19_getCo2Ppm(*_co2ppm);
			printf("CO2 PPM: %d\n",*_co2ppm );
			return &_co2ppm;
		}
	}
}
