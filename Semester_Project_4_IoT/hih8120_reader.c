/*
 * hih_reader.c
 *
 * Created: 13/05/2021 17.06.06
 *  Author: Marton Pentek
 */ 

#include "hih8120_reader.h"
#include <hih8120.h>
#include <stdio.h>

#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <semphr.h>
TickType_t xDelayWakeUp;

hih8120results readValueAll(){
	hih8120results result;
	xDelayWakeUp = pdMS_TO_TICKS(55);
	
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
		vTaskDelay(pdMS_TO_TICKS(10));
		for (;;)
		{
			if (hih8120_isReady())
			{
				result.hum= hih8120_getHumidity();
				result.temp= hih8120_getTemperature();
				return result;
			}
		}
	}
	
}