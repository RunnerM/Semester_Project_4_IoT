/*
 * rcServo_handler.c
 *
 * Created: 23/05/2021 14.36.38
 *  Author: pente
 */ 
#include "rcServo_handler.h"
#include <rc_servo.h>


void setPosition(int pos){
	rc_servo_setPosition(0,pos);
}