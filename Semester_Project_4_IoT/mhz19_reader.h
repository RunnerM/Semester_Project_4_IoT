/*
 * mhz19_reader.h
 *
 * Created: 23/05/2021 14.13.52
 *  Author: pente
 */ 
#include <stdio.h>
//#include <avr/io.h>

#ifndef MHZ19_READER_H_
#define MHZ19_READER_H_

/**
\brief Function for reading the CO2ppm value.
\return uint16_t
*/
uint16_t read_CO2_ppm();

#endif /* MHZ19_READER_H_ */