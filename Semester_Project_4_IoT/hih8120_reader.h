/*
 * hih8120_reader.h
 *
 * Created: 13/05/2021 16.53.39
 *  Author: Marton Pentek
 */ 

#include <stdio.h>
//#include <avr/io.h>

#ifndef HIH8120_READER_H_
#define HIH8120_READER_H_

typedef struct{
	float temp;
	float hum;
}hih8120results;

hih8120results readValueAll();




#endif /* HIH8120_READER_H_ */

