/*
* loraWANHandler.c
*
* Created: 12/04/2019 10:09:05
*  Author: IHA
*/
/*#include <stddef.h>
#include <stdio.h>

#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include <lora_driver.h>
#include <status_leds.h>

//#include "main.h"
#include "LoRaWANHandler.h"

// Parameters for OTAA join - You have got these in a mail from IHA
#define LORA_appEUI "65FDAA6D0706AF4E"
#define LORA_appKEY "43EE2CF43D3D5797D3F7564263534C82"

static char _out_buf[100];

void lora_uplink_task( void *pvParameters );
void lora_downlink_task( void *pvParameters );
void lora_init_task( void *pvParameters );

static lora_driver_payload_t _uplink_payload;
lora_driver_payload_t downlinkPayload;

QueueHandle_t xQueueForReadings;
MeasurementValues valuesFromQueue;
MessageBufferHandle_t downLinkMessageBufferHandle;




void lora_handler_initialise(UBaseType_t lora_handler_task_priority)
{
	xTaskCreate(
	lora_init_task
	,  "LRHand"  // A name just for humans
	,  configMINIMAL_STACK_SIZE+200  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  NULL
	,  lora_handler_task_priority  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
	
	/*xTaskCreate(
		lora_uplink_task
		,  "LRHand"  // A name just for humans
		,  configMINIMAL_STACK_SIZE+200  // This stack size can be checked & adjusted by reading the Stack Highwater
		,  NULL
		,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		,  NULL );
		
		xTaskCreate(
		lora_downlink_task
		,  "LRHand"  // A name just for humans
		,  configMINIMAL_STACK_SIZE+200  // This stack size can be checked & adjusted by reading the Stack Highwater
		,  NULL
		,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		,  NULL );
}

static void _lora_setup(void)
{
	lora_driver_returnCode_t rc;
	status_leds_slowBlink(led_ST2); // OPTIONAL: Led the green led blink slowly while we are setting up LoRa

	// Factory reset the transceiver
	printf("FactoryReset >%s<\n", lora_driver_mapReturnCodeToText(lora_driver_rn2483FactoryReset()));
	
	// Configure to EU868 LoRaWAN standards
	printf("Configure to EU868 >%s<\n", lora_driver_mapReturnCodeToText(lora_driver_configureToEu868()));

	// Get the transceivers HW EUI
	rc = lora_driver_getRn2483Hweui(_out_buf);
	printf("Get HWEUI >%s<: %s\n",lora_driver_mapReturnCodeToText(rc), _out_buf);

	// Set the HWEUI as DevEUI in the LoRaWAN software stack in the transceiver
	printf("Set DevEUI: %s >%s<\n", _out_buf, lora_driver_mapReturnCodeToText(lora_driver_setDeviceIdentifier(_out_buf)));

	// Set Over The Air Activation parameters to be ready to join the LoRaWAN
	printf("Set OTAA Identity appEUI:%s appKEY:%s devEUI:%s >%s<\n", LORA_appEUI, LORA_appKEY, _out_buf, lora_driver_mapReturnCodeToText(lora_driver_setOtaaIdentity(LORA_appEUI,LORA_appKEY,_out_buf)));

	// Save all the MAC settings in the transceiver
	printf("Save mac >%s<\n",lora_driver_mapReturnCodeToText(lora_driver_saveMac()));

	// Enable Adaptive Data Rate
	printf("Set Adaptive Data Rate: ON >%s<\n", lora_driver_mapReturnCodeToText(lora_driver_setAdaptiveDataRate(LORA_ON)));

	// Set receiver window1 delay to 500 ms - this is needed if down-link messages will be used
	printf("Set Receiver Delay: %d ms >%s<\n", 500, lora_driver_mapReturnCodeToText(lora_driver_setReceiveDelay(500)));

	// Join the LoRaWAN
	uint8_t maxJoinTriesLeft = 10;
	
	do {
		rc = lora_driver_join(LORA_OTAA);
		printf("Join Network TriesLeft:%d >%s<\n", maxJoinTriesLeft, lora_driver_mapReturnCodeToText(rc));

		if ( rc != LORA_ACCEPTED)
		{
			// Make the red led pulse to tell something went wrong
			status_leds_longPuls(led_ST1); // OPTIONAL
			// Wait 5 sec and lets try again
			vTaskDelay(pdMS_TO_TICKS(5000UL));
		}
		else
		{
			break;
		}
	} while (--maxJoinTriesLeft);

	if (rc == LORA_ACCEPTED)
	{
		// Connected to LoRaWAN :-)
		// Make the green led steady
		status_leds_ledOn(led_ST2); // OPTIONAL
		// creating the up link and down link task when connected successfully.
		xTaskCreate(
		lora_uplink_task
		,  "LRHand"  // A name just for humans
		,  configMINIMAL_STACK_SIZE+200  // This stack size can be checked & adjusted by reading the Stack Highwater
		,  NULL
		,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		,  NULL );
		
		xTaskCreate(
		lora_downlink_task
		,  "LRHand"  // A name just for humans
		,  configMINIMAL_STACK_SIZE+200  // This stack size can be checked & adjusted by reading the Stack Highwater
		,  NULL
		,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		,  NULL );
	}
	else
	{
		// Something went wrong
		// Turn off the green led
		status_leds_ledOff(led_ST2); // OPTIONAL
		// Make the red led blink fast to tell something went wrong
		status_leds_fastBlink(led_ST1); // OPTIONAL

		// Lets stay here
		while (1)
		{
			taskYIELD();
		}
	}
}

/*-----------------------------------------------------------
void lora_uplink_task( void *pvParameters)
{	
	//get queue from parameter and send the last mesurements;
	_uplink_payload.len = 6;
	_uplink_payload.portNo = 2;
	//local variable for received measurements
	
	
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(300000UL); // Upload message every 5 minutes (300000 ms)
	xLastWakeTime = xTaskGetTickCount();
	
	for(;;)
	{
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
		//if(xSemaphoreTake(xIOSemaphore,pdMS_TO_TICKS(100))==pdTRUE){
			if (xQueueReceive(xQueueForReadings,&valuesFromQueue,pdMS_TO_TICKS(100)))
			{
				
				_uplink_payload.bytes[0] = valuesFromQueue.Humidity >> 8;
				_uplink_payload.bytes[1] = valuesFromQueue.Humidity & 0xFF;
				_uplink_payload.bytes[2] = valuesFromQueue.Temperature >> 8;
				_uplink_payload.bytes[3] = valuesFromQueue.Temperature & 0xFF;
				
				lora_driver_returnCode_t rc;
				
				if ((rc = lora_driver_sendUploadMessage(false, &_uplink_payload)) == LORA_MAC_TX_OK )
				{
					// The uplink message is sent and there is no downlink message received
				}
				else if (rc == LORA_MAC_RX)
				{
					// The uplink message is sent and a downlink message is received
				}
				break;
				printf("Upload Message >%s<\n", lora_driver_mapReturnCodeToText(lora_driver_sendUploadMessage(false, &_uplink_payload)));
			}
			//Giving back io semaphore.
			//xSemaphoreGive(xIOSemaphore);
		//}
		
		
		status_leds_shortPuls(led_ST4);  // OPTIONAL
		printf("Upload Message >%s<\n", lora_driver_mapReturnCodeToText(lora_driver_sendUploadMessage(false, &_uplink_payload)));
	}
}

/*-----------------------------------------------------------


void lora_downlink_task( void *pvParameters)
{
	//get message buffer from parameter;

	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(3000UL); // Upload message every 5 minutes (300000 ms)
	xLastWakeTime = xTaskGetTickCount();
	
	printf("reading down link");
	for(;;)
	{
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
		xMessageBufferReceive(downLinkMessageBufferHandle, &downlinkPayload, sizeof(lora_driver_payload_t), portMAX_DELAY);
		printf("DOWN LINK: from port: %d with %d bytes received!", downlinkPayload.portNo, downlinkPayload.len); // Just for Debug
		if (4 == downlinkPayload.len) // Check that we have got the expected 4 bytes
		{
			// decode the payload into our variales
			uint16_t payload_begin;
			uint16_t payload_end;
			payload_begin = (downlinkPayload.bytes[0] << 8) + downlinkPayload.bytes[1];
			payload_end = (downlinkPayload.bytes[2] << 8) + downlinkPayload.bytes[3];
			
			printf("payload received: %i , %i",payload_begin,payload_end);
		}
	}
	
	
}

void lora_init_task(void *pvParameters){
	
	for(;;){
		
		if (xSemaphoreTake(xIOSemaphore,pdMS_TO_TICKS(100))==pdTRUE)
		{
			printf("xIOsemaphore taken by lora init/n");
			// Hardware reset of LoRaWAN transceiver
			lora_driver_resetRn2483(1);
			vTaskDelay(2);
			lora_driver_resetRn2483(0);
			// Give it a chance to wakeup
			vTaskDelay(150);

			lora_driver_flushBuffers(); // get rid of first version string from module after reset!

			_lora_setup();
	
			xQueueForReadings = xQueueCreate(3,sizeof(struct MeasurementValues*));

	
			// deletes task after setup;
			vTaskDelete(NULL);
			printf("xIOsemaphore given by lora init/n");
			xSemaphoreGive(xIOSemaphore);
		}
	}
	
	
}*/
