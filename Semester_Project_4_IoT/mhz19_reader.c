/*
 * mhz19_reader.c
 *
 * Created: 23/05/2021 14.15.42
 *  Author: pente
 */ 

#include "mhz19_reader.h"
#include <mh_z19.h>

read_CO2_ppm(){
	mh_z19_returnCode_t rc;
	uint16_t CO2ppm;
	for (;;)
	{
		puts("Reading MHZ19\n");
		rc = mh_z19_takeMeassuring;
		if (rc != MHZ19_OK) {
			puts("Error reading CO2 sensor");
		}
		else {
			CO2ppm = mh_z19_getCo2Ppm();
			printf("CO2 PPM: %d\n", CO2ppm);
			return CO2ppm;
		}
	}
}
