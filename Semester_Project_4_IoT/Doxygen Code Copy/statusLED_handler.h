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

void setLED(int num, int toOn);
void initialize_status_leds();


#endif /* STATUSLED_HANDLER_H_ */