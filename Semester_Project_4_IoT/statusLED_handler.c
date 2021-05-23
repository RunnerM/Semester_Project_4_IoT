/*
 * statusLED_handler.c
 *
 * Created: 23/05/2021 14.21.39
 *  Author: pente
 */ 

#include "statusLED_handler.h"
#include <status_leds.h>

void setLED(int num, bool toOn){
	if (toOn)
	{
		switch (num)
		{
			case 1: status_leds_ledOn(led_ST1);
			break;
			case 2: status_leds_ledOn(led_ST2);
			break;
			case 3: status_leds_ledOn(led_ST3);
			break;
			case 4: status_leds_ledOn(led_ST4);
			break;
		}
	}else{
		switch (num)
		{
			case 1: status_leds_ledOff(led_ST1);
			break;
			case 2: status_leds_ledOff(led_ST2);
			break;
			case 3: status_leds_ledOff(led_ST3);
			break;
			case 4: status_leds_ledOff(led_ST4);
			break;
		}
	}
		
}
void initialize(){
	status_leds_initialise(5);
}
