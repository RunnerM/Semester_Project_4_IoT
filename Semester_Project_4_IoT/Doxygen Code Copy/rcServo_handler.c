/**
* @file rcServo_handler.c
* Servo handler class
*/
#include "rcServo_handler.h"
#include <rc_servo.h>

 ///  Function to init the servo, called at runtime start
void initialize(){
	rc_servo_initialise();
}

/**
* Sets the position of the servo
* @param[in] pos
*/
void setPosition(int pos){
	rc_servo_setPosition(0,pos);
}