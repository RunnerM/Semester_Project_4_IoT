/*
 * luxSensor_reader.h
 *
 * Created: 23/05/2021 14.56.29
 *  Author: pente
 */ 

#include <stdio.h>
#include <avr/io.h>
#include <tsl2591.h>

#ifndef LUXSENSOR_READER_H_
#define LUXSENSOR_READER_H_

void tsl2591Callback(tsl2591_returnCode_t rc);
float read_lux();


#endif /* LUXSENSOR_READER_H_ */