

#include <stdio.h>
#include <avr/io.h>

//---FreeRTOS-----------------------

#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <semphr.h>

//---I/O----------------------------

#include <stdio_driver.h>
#include <serial.h>

//---Sensors------------------------

#include "hih8120_reader.h"
#include "mhz19_reader.h"
#include "luxSensor_reader.h"

//---Actuators----------------------

#include "rcServo_handler.h"
#include "statusLED_handler.h"


 // Needed for LoRaWAN
#include <lora_driver.h>
#include <status_leds.h>
#include "LoRaWANHandler.h"

// define Tasks
void Sensors_reader( void *pvParameters );
void Task_executor( void *pvParameters );
//void Lora_sender( void *pvParameters );
//void Lora_reciever( void *pvParameters );


// define semaphore handle
SemaphoreHandle_t xIOSemaphore;// For serial connection.

// Prototype for LoRaWAN handler
void lora_handler_initialise(UBaseType_t lora_handler_task_priority);

//---Global variables----------

//---struct for storing measured data----




struct MeasurementValues
{
	uint16_t Temperature;
	uint16_t Humidity;
	uint16_t CO2ppm;
	float Lux;
};


/*-----------------------------------------------------------*/
void create_tasks_and_semaphores(void)
{
	// Semaphores are useful to stop a Task proceeding, where it should be paused to wait,
	// because it is sharing a resource, such as the Serial port.
	// Semaphores should only be used whilst the scheduler is running, but we can set it up here.
	if ( xIOSemaphore == NULL )  // Check to confirm that the Semaphore has not already been created.
	{
		xIOSemaphore = xSemaphoreCreateMutex();  // Create a mutex semaphore.
		if ( ( xIOSemaphore ) != NULL )
		{
			xSemaphoreGive( ( xIOSemaphore ) );  // Make the mutex available for use, by initially "Giving" the Semaphore.
		}
	}

	xTaskCreate(
	Sensors_reader
	,  "sensor_reader"  // A name just for humans
	,  configMINIMAL_STACK_SIZE  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  NULL
	,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );

 	xTaskCreate(
 	Task_executor
 	,  "Task_executor"  // A name just for humans
 	,  configMINIMAL_STACK_SIZE  // This stack size can be checked & adjusted by reading the Stack Highwater
 	,  NULL
 	,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
 	,  NULL );
	
}

/*-----------------------------------------------------------*/
void Sensors_reader( void *pvParameters )
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 6000/portTICK_PERIOD_MS; // 5 min
	//measurement container.
	struct MeasurementValues newvalues;

	// Initialize the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();
	
	for(;;)
	{
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
		if (xSemaphoreTake(xIOSemaphore,pdMS_TO_TICKS(100))==pdTRUE)
		{
			//reading values -----
			newvalues.Temperature= readValueTemp();
			newvalues.Humidity = readValueHum();
			newvalues.CO2ppm= read_CO2_ppm();
			newvalues.Lux= read_lux();
			//putting new values into value Queue.
			xQueueSend(xQueueForReadings,&newvalues,portMAX_DELAY);
			//Giving back io semaphore.
			xSemaphoreGive(xIOSemaphore);
		}else{
			//timed out
		}
		
	}

/*	for(;;)
	{
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
		//puts("Task1"); // stdio functions are not reentrant - Should normally be protected by MUTEX

			const TickType_t xDelayWakeUp = 60;
			//const TickType_t xDelayFetch = 10;
			lora_driver_payload_t uplinkPayload;
			puts("Reading HIH8120\n");
			if (HIH8120_OK!=hih8120_wakeup())
			{
				puts("error with sensor hih8120 wakeup\n");
			}else{
				vTaskDelay(xDelayWakeUp);
			} 
			if (HIH8120_OK!=hih8120_measure())
			{
				puts("error with sensor hih8120 maesure\n");
			}else
			{
				for (;;)
				{
					if (hih8120_isReady())
					{
						humidity= hih8120_getHumidity();
						temperature= hih8120_getTemperature();
						printf("Humidity: %d\n",humidity);
						printf("Temp: %d\n\n",temperature);
						
						uplinkPayload.len = 4; // Length of the actual payload
						uplinkPayload.portNo = 1; // The LoRaWAN port no to sent the message to
						
						uplinkPayload.bytes[0] = humidity >> 8;
						uplinkPayload.bytes[1] = humidity & 0xFF;
						uplinkPayload.bytes[2] = temperature >> 8;
						uplinkPayload.bytes[3] = temperature & 0xFF; 
						
						lora_driver_returnCode_t rc;
						
						if ((rc = lora_driver_sendUploadMessage(false, &uplinkPayload)) == LORA_MAC_TX_OK )
						{
							// The uplink message is sent and there is no downlink message received
						}
						else if (rc == LORA_MAC_RX)
						{
							// The uplink message is sent and a downlink message is received
						}
						break;
					}
				}
			}
		
	}*/

}

/*-----------------------------------------------------------*/
void Task_executor( void *pvParameters )
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 1000/portTICK_PERIOD_MS; // 1000 ms

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();
	
	//take semaphore for IO here;

	for(;;)
	{
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
		
	}
}

/*-----------------------------------------------------------*/

void Lora_sender( void *pvParameters )
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 1000/portTICK_PERIOD_MS; // 1000 ms

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();
	
	//take semaphore for IO here;

	for(;;)
	{
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
		
	}
}

/*-----------------------------------------------------------*/

void Lora_reciever( void *pvParameters )
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 1000/portTICK_PERIOD_MS; // 1000 ms

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();
	
	//take semaphore for IO here;

	for(;;)
	{
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
		
	}
}

/*-----------------------------------------------------------*/
void initialiseSystem()
{
	
	
	// Set output ports for leds used in the example
	DDRA |= _BV(DDA0) | _BV(DDA7);

	// Make it possible to use stdio on COM port 0 (USB) on Arduino board - Setting 57600,8,N,1
	stdio_initialise(ser_USART0);
	
	// Let's create some tasks
	create_tasks_and_semaphores();

	// vvvvvvvvvvvvvvvvv BELOW IS LoRaWAN initialisation vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// Status Leds driver
	initialize_status_leds(); // 
	// Initialise the LoRaWAN driver without down-link buffer
	
	
	
	lora_driver_resetRn2483(1); // Activate reset line
	vTaskDelay(2);
	lora_driver_resetRn2483(0); // Release reset line
	vTaskDelay(150); // Wait for tranceiver module to wake up after reset
	lora_driver_flushBuffers(); // get rid of first version string from module after reset!
	
	// Create LoRaWAN task and start it up with priority 3
	lora_handler_initialise(3);
}

/*-----------------------------------------------------------*/
int main(void)
{
	initialiseSystem(); // Must be done as the very first thing!!
	printf("Program Started!!\n");
	vTaskStartScheduler(); // Initialise and run the freeRTOS scheduler. Execution should never return from here.

	/* Replace with your application code */
	while (1)
	{
	}
}

