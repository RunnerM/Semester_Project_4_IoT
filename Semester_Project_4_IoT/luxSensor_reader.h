/*
 * luxSensor_reader.h
 *
 * Created: 23/05/2021 14.56.29
 *  Author: pente
 */ 

#include <stdio.h>
//#include <avr/io.h>
#include <tsl2591.h>

#ifndef LUXSENSOR_READER_H_
#define LUXSENSOR_READER_H_
/**
\brief Callback function required by TSL2591 driver.
\return void
*/
void tsl2591Callback(tsl2591_returnCode_t rc);
/**
\brief This function is initiating the reading process.
\return void
*/
void read_lux();
/**
\brief Getter for the measured value.
\return float
*/
float get_lux_value();


#endif /* LUXSENSOR_READER_H_ */