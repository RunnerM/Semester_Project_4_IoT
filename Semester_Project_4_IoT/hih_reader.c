/*
 * hih_reader.c
 *
 * Created: 13/05/2021 17.06.06
 *  Author: Marton Pentek
 */ 

#include "hih8120_reader.h"
#include <stdio.h>


hih_data_container refresh_data(){
	hih_data_container result;
	
	if (HIH8120_OK!=hih8120_wakeup())
	{
		puts("error with sensor hih8120 wakeup\n");
		}else{
		vTaskDelay(xDelayWakeUp);
	}
	if (HIH8120_OK!=hih8120_measure())
	{
		puts("error with sensor hih8120 maesure\n");
	}else
	{
		for (;;)
		{
			if (hih8120_isReady())
			{
				result.humidity= hih8120_getHumidity();
				result.temperature= hih8120_getTemperature();
				printf("Humidity: %d\n",humidity);
				printf("Temp: %d\n\n",temperature);
			}
		}
	}
	return result;
	
}

