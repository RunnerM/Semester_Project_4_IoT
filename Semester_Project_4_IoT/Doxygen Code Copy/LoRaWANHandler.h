/*
 * LoRaWANHandler.h
 *
 * Created: 24/05/2021 15.12.21
 *  Author: pente
 */ 


#ifndef LORAWANHANDLER_H_
#define LORAWANHANDLER_H_

/*typedef struct
{
	uint16_t Temperature;
	uint16_t Humidity;
	uint16_t CO2ppm;
	float Lux;
}MeasurementValues ;

extern QueueHandle_t xQueueForReadings;
void lora_handler_initialise(UBaseType_t lora_handler_task_priority);
static void _lora_setup(void);



#endif /* LORAWANHANDLER_H_ */