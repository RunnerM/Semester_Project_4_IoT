/*
 * hih8120_reader.h
 *
 * Created: 13/05/2021 16.53.39
 *  Author: Marton Pentek
 */ 

#include <stdio.h>
#include <avr/io.h>

#ifndef HIH8120_READER_H_
#define HIH8120_READER_H_

uint16_t readValueTemp();
uint16_t readValueHum();


#endif /* HIH8120_READER_H_ */

