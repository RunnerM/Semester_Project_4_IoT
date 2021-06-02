/*
 * statusLED_handler.h
 *
 * Created: 23/05/2021 14.20.48
 *  Author: pente
 */ 
#include <ATMEGA_FreeRTOS.h>
#include <task.h>

#ifndef STATUSLED_HANDLER_H_
#define STATUSLED_HANDLER_H_
/**
\brief Function to set led on or off. Takes two parameters number of led and the value on(1) or off(0).
\return void
*/
void setLED(int num, int toOn);
/**
\brief Function for initializing status leds.
\return void
*/
void initialize_status_leds();


#endif /* STATUSLED_HANDLER_H_ */