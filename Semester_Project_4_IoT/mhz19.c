/*
 * mhz19.c
 *
 * Created: 14/05/2021 10.34.24
 *  Author: Gustaw 
 */ 

uint16_t read_MHZ19(void* pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 500;
	xLastWakeTime = xTaskGetTickCount();
	mh_z19_returnCode_t rc;
	uint16_t CO2ppm;

	for (;;)
	{
		xTaskDelayUntil(&xLastWakeTime, xFrequency);
		lora_driver_payload_t uplinkPayload;
		puts("Reading MHZ19\n");
		rc = mh_z19_takeMeassuring;
		if (rc != MHZ19_OK) {
			puts("Error reading CO2 sensor");
		}
		else {
			CO2ppm = mh_z19_getCo2Ppm();
			printf("CO2 PPM: %d\n", CO2ppm);
			return CO2ppm;
		}
	}
}