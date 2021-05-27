
#include <stddef.h>
#include <stdio.h>
#include <avr/io.h>

#include <stdio_driver.h>
#include <serial.h>
//---FreeRTOS-----------------------
#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>


//---I/O----------------------------

#include <stdio_driver.h>
#include <serial.h>

//---Sensors------------------------

#include "hih8120_reader.h"
#include "mhz19_reader.h"
#include "luxSensor_reader.h"
#include <hih8120.h>
#include <mh_z19.h>
#include <tsl2591.h>

//---Actuators----------------------

#include "rcServo_handler.h"
#include "statusLED_handler.h"
#include <rc_servo.h>


 // Needed for LoRaWAN
#include <lora_driver.h>
#include <status_leds.h>
//#include "LoRaWANHandler.h"

// Parameters for OTAA join - You have got these in a mail from IHA
#define LORA_appEUI "65FDAA6D0706AF4E"
#define LORA_appKEY "43EE2CF43D3D5797D3F7564263534C82"

static char _out_buf[100];
// define Tasks
void Sensors_reader( void *pvParameters );
void Task_executor( void *pvParameters );
void lora_uplink_task( void *pvParameters );
void lora_downlink_task( void *pvParameters );
void lora_init_task( void *pvParameters );


// define semaphore handle
SemaphoreHandle_t xIOSemaphore;// For serial connection.

// Prototype for LoRaWAN handler
void lora_handler_initialise(UBaseType_t lora_handler_task_priority);

typedef struct
{
	float Temperature;
	float Humidity;
	uint16_t CO2ppm;
	float Lux;
}MeasurementValues ;

//---Global variables----------
MessageBufferHandle_t downLinkMessageBufferHandle;
static lora_driver_payload_t _uplink_payload;
lora_driver_payload_t downlinkPayload;

QueueHandle_t xQueueForReadings;
MeasurementValues valuesFromQueue;
MessageBufferHandle_t downLinkMessageBufferHandle;

//---struct for storing measured data----


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
	,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
	
	xTaskCreate(
		lora_uplink_task
		,  "LR_up_link"  // A name just for humans
		,  configMINIMAL_STACK_SIZE+200  // This stack size can be checked & adjusted by reading the Stack Highwater
		,  NULL
		,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		,  NULL );

 	/*xTaskCreate(
 	Task_executor
 	,  "Task_executor"  // A name just for humans
 	,  configMINIMAL_STACK_SIZE  // This stack size can be checked & adjusted by reading the Stack Highwater
 	,  NULL
 	,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
 	,  NULL );*/
	 
	 /*xTaskCreate(
		lora_downlink_task
		,  "LR_down_link"  // A name just for humans
		,  configMINIMAL_STACK_SIZE+200  // This stack size can be checked & adjusted by reading the Stack Highwater
		,  NULL
		,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		,  NULL );*/
	
}

/*-----------------------------------------------------------*/
void Sensors_reader( void *pvParameters )
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(30000UL); // Upload message every 5 minutes (300000 ms)
	xLastWakeTime = xTaskGetTickCount();
	//measurement container.
	 MeasurementValues newvalues;
	 MeasurementValues *poniterToValues;
	hih8120results hihresults;

	for(;;)
	{
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
		if (xSemaphoreTake(xIOSemaphore,pdMS_TO_TICKS(100))==pdTRUE)
		{
			printf("xIOsemaphore taken by sensor reader\n");
			//reading values -----
			hihresults = readValueAll();
			//printf("hih reading have been performed. Hum: %i , temp: %i  ", hihresults.hum,hihresults.temp);
			newvalues.Temperature= hihresults.temp;
			printf("temp done: %7.4f \n", hihresults.temp);
			newvalues.Humidity = hihresults.hum;
			printf("hum done : %7.4f \n", hihresults.hum);
			uint16_t co2ppm= read_CO2_ppm();
			printf("co2ppm : %d ",co2ppm);
			newvalues.CO2ppm= co2ppm;
			printf("co2 done %d \n", newvalues.CO2ppm);
			//newvalues.Lux= read_lux();
			//printf("lux done\n");
			//putting new values into value Queue.
			poniterToValues= &newvalues;
			
			if (pdTRUE==xQueueSend(xQueueForReadings,(void *) &poniterToValues,portMAX_DELAY)){
				puts("message sent.");
			}else{
				puts("message was not sent");
			}
			//Giving back io semaphore.
			printf("xIOsemaphore given sensor reader\n");
			xSemaphoreGive( ( xIOSemaphore ) );
		}else{
			//timed out
		}
		
	}
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
		if (xSemaphoreTake(xIOSemaphore,pdMS_TO_TICKS(100))==pdTRUE)
		{
			printf("xIOsemaphore taken/n");
			
			
			printf("xIOsemaphore given/n");
			xSemaphoreGive( ( xIOSemaphore ) );
	}
	}
}

/*-----------------------------------------------------------*/
void initialiseSystem()
{
	//creating queue for communicating between reader and LoRa sender task.
	xQueueForReadings = xQueueCreate(10,sizeof(struct MeasurementValues*));
	// Set output ports for leds used in the example
	DDRA |= _BV(DDA0) | _BV(DDA7);

	// Make it possible to use stdio on COM port 0 (USB) on Arduino board - Setting 57600,8,N,1
	stdio_initialise(ser_USART0);
	
	// Let's create some tasks
	create_tasks_and_semaphores();

	// vvvvvvvvvvvvvvvvv BELOW IS LoRaWAN initialisation vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// Status Leds driver
	initialize_status_leds(); // 
	mh_z19_initialise(ser_USART3);
	if ( HIH8120_OK == hih8120_initialise() )
	{
	}
	if ( TSL2591_OK == tsl2591_initialise(&tsl2591Callback) )
	{
	}
	rc_servo_initialise();	
	
	
	// Initialise the LoRaWAN driver without down-link buffer
	
	downLinkMessageBufferHandle = xMessageBufferCreate(sizeof(lora_driver_payload_t)*2); // Here I make room for two downlink messages in the message buffer
	lora_driver_initialise(ser_USART1, downLinkMessageBufferHandle); // The parameter is the USART port the RN2483 module is connected to - in this case USART1 - here no message buffer for down-link messages are defined
	
	// Create LoRaWAN task and start it up with priority 3
	//lora_handler_initialise(3);
}
	
/*--------------------------LORA---------------------------------*/

void lora_handler_initialise(UBaseType_t lora_handler_task_priority)
{
	xTaskCreate(
	lora_init_task
	,  "LRHand"  // A name just for humans
	,  configMINIMAL_STACK_SIZE+200  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  NULL
	,  lora_handler_task_priority  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
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
		/*xTaskCreate(
		lora_uplink_task
		,  "LR_up_link"  // A name just for humans
		,  configMINIMAL_STACK_SIZE+200  // This stack size can be checked & adjusted by reading the Stack Highwater
		,  NULL
		,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		,  NULL );*/
		
		/*xTaskCreate(
		lora_downlink_task
		,  "LR_down_link"  // A name just for humans
		,  configMINIMAL_STACK_SIZE+200  // This stack size can be checked & adjusted by reading the Stack Highwater
		,  NULL
		,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		,  NULL );*/
		
		printf("xIOsemaphore given by lora init\n");
		xSemaphoreGive( ( xIOSemaphore ) );
		while (1)
		{
			taskYIELD();// this gives thread to other task.
		}
		
	}
	else
	{
		// Something went wrong
		// Turn off the green led
		status_leds_ledOff(led_ST2); // OPTIONAL
		// Make the red led blink fast to tell something went wrong
		status_leds_fastBlink(led_ST1); // OPTIONAL
		printf("xIOsemaphore given by lora init\n");
		xSemaphoreGive( ( xIOSemaphore ) );
		// Lets stay here
		while (1)
		{
			taskYIELD();// this gives thread to other task.
		}
	}
}

void lora_uplink_task( void *pvParameters)
{
	//get queue from parameter and send the last mesurements;
	_uplink_payload.len = 6;
	_uplink_payload.portNo = 2;
	//local variable for received measurements
	
	
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(15000UL); // Upload message every 5 minutes (300000 ms)
	xLastWakeTime = xTaskGetTickCount();
	
	for(;;)
	{
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
		if (pdTRUE == xQueueForReadings,&valuesFromQueue,pdMS_TO_TICKS(100))
		{
			if(xSemaphoreTake(xIOSemaphore,pdMS_TO_TICKS(100))==pdTRUE){
				puts("semaphore taken by uplink");
				//printf("semaphore taken by up link task\n");
				//printf("trying to read from queue.\n");
			/*_uplink_payload.bytes[0] = valuesFromQueue.Humidity >> 8;
			_uplink_payload.bytes[1] = valuesFromQueue.Humidity & 0xFF;
			_uplink_payload.bytes[2] = valuesFromQueue.Temperature >> 8;
			_uplink_payload.bytes[3] = valuesFromQueue.Temperature & 0xFF;*/
			
			printf("Temp: %f \n", valuesFromQueue.Temperature);
			printf("Hum: %f \n", valuesFromQueue.Humidity);
			printf("CO2ppm: %d \n", valuesFromQueue.CO2ppm);
			puts("3");
			/*lora_driver_returnCode_t rc;
			
			if ((rc = lora_driver_sendUploadMessage(false, &_uplink_payload)) == LORA_MAC_TX_OK )
			{
				printf("Lora message has been sent!");
			}
			else if (rc == LORA_MAC_RX)
			{
				printf("Lora message has been sent down link message recieved!");
			}
			break;
			printf("Upload Message >%s<\n", lora_driver_mapReturnCodeToText(lora_driver_sendUploadMessage(false, &_uplink_payload)));*/
			
			//Giving back io semaphore.
			printf("semaphore given by up link task\n");
			xSemaphoreGive( ( xIOSemaphore ) );
		}
		else{
			printf("nothing received from queue\n");
		}
		
		}
	}
	}


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
		if(xSemaphoreTake(xIOSemaphore,pdMS_TO_TICKS(100))==pdTRUE){
			printf("semaphore taken by up downlink task\n");
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
		printf("semaphore given by down link task\n");
		xSemaphoreGive( ( xIOSemaphore ) );
		}
	}
}

void lora_init_task(void *pvParameters)
{
	
	for(;;){
		
		if (xSemaphoreTake(xIOSemaphore,pdMS_TO_TICKS(100))==pdTRUE)
		{
			printf("xIOsemaphore taken by lora init\n");
			// Hardware reset of LoRaWAN transceiver
			lora_driver_resetRn2483(1);
			vTaskDelay(2);
			lora_driver_resetRn2483(0);
			// Give it a chance to wakeup
			vTaskDelay(150);

			lora_driver_flushBuffers(); // get rid of first version string from module after reset!

			_lora_setup();
			printf("xIOsemaphore given by lora init\n");
		xSemaphoreGive( ( xIOSemaphore ) );
		}
	}
}

/*void vApplicationIdleHook(void)
{
	if (xSemaphoreTake(xIOSemaphore,pdMS_TO_TICKS(100))==pdTRUE)
	{
		printf(".");
		xSemaphoreGive( ( xIOSemaphore ) );
	}
}*/
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



