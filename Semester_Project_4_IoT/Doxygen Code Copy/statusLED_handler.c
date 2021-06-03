/**
* @file statusLED_handler.c
* LED handler class
*/
#include "statusLED_handler.h"
#include <status_leds.h>


/**
*	Switches LED's on and off
*	0 = Off, 1 = On
*	@param[in] num
*	@param[in] toOn
*/
void setLED(int num, int toOn){
	if (toOn==1)
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

///  Function to init the leds, called at runtime start
void initialize_status_leds(){
	
	status_leds_initialise(5);
}
