/**
* @file hih8120_reader.c
* Temperature and Humidity Sensor reader class
*/

#include "hih8120_reader.h"
#include <hih8120.h>
#include <stdio.h>

#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <semphr.h>

/// Wakeup delay
TickType_t xDelayWakeUp;


/**
* Function that reads both humidity % and temperature
* @param[out] result
*/
hih8120results readValueAll(){
	hih8120results result;
	xDelayWakeUp = pdMS_TO_TICKS(55);
	
	if (HIH8120_OK!=hih8120_wakeup())
	{
		puts("error with sensor hih8120 wakeup");
		}else{
		vTaskDelay(xDelayWakeUp);
	}
	if (HIH8120_OK!=hih8120_measure())
	{
		puts("error with sensor hih8120 measure\n");
		return result;
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