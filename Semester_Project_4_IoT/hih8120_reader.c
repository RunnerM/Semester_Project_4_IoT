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
const TickType_t xDelayWakeUp = 60;

uint16_t readValueHum(){
	uint16_t result;
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
				result= hih8120_getHumidity();
			}
		}
	}
	return result;
}

uint16_t readValueTemp(){
	uint16_t result;

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
				result= hih8120_getTemperature();
			}
		}
	}
	return result;
}

