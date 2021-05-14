/*
 * hih8120_reader.h
 *
 * Created: 13/05/2021 16.53.39
 *  Author: Marton Pentek
 */ 


#ifndef HIH8120_READER_H_
#define HIH8120_READER_H_

struct hih_data_container 
{
	uint16_t humidity;
	uint16_t temperature;
};

hih_data_container refresh_data(); 

#endif /* HIH8120_READER_H_ */

