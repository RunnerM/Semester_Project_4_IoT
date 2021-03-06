
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
#include <event_groups.h>


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

// EventBits-------
#define BIT_0	( 1 << 0 )
#define BIT_1   ( 1 << 1 )
#define BIT_2	( 1 << 2 )

static char _out_buf[100];
// define Tasks
void HIH8120_reader( void *pvParameters );
void MHZ19_reader( void *pvParameters );
void TSL2591_reader( void *pvParameters );
void lora_uplink_task( void *pvParameters );
void lora_downlink_task( void *pvParameters );
void lora_init_task( void *pvParameters );


// define semaphore handle
SemaphoreHandle_t xIOSemaphore;// For serial connection.

// Prototype for LoRaWAN handler
void lora_handler_initialise(UBaseType_t lora_handler_task_priority);
void aFunctionToWaitBits( EventGroupHandle_t xEventGroup );
void aFunctionToClearBits( EventGroupHandle_t xEventGroup );
void aFunctionToSetBits( EventGroupHandle_t xEventGroup , int bit_No);
EventGroupHandle_t getEventGroup();


//globals--------
uint16_t Temperature;
uint16_t Humidity;
uint16_t CO2ppm;
uint16_t Lux;
size_t xReceivedBytes;

unsigned char message[2];

//---Global variables----------
MessageBufferHandle_t downLinkMessageBufferHandle;
static lora_driver_payload_t _uplink_payload;
lora_driver_payload_t downlinkPayload;

QueueHandle_t xQueueForReadings;
MessageBufferHandle_t downLinkMessageBufferHandle;
EventGroupHandle_t xCreatedEventGroup;

EventGroupHandle_t getEventGroup(){
	return xCreatedEventGroup;
}

/*-----------------------------------------------------------*/
/** 
\note This function is for creating tasks and IOSeamphore.
*/
void create_tasks_and_semaphores(void)
{
	if ( xIOSemaphore == NULL )  
	{
		xIOSemaphore = xSemaphoreCreateMutex();  // Create a mutex semaphore.
		if ( ( xIOSemaphore ) != NULL )
		{
			xSemaphoreGive( ( xIOSemaphore ) );  // Make the mutex available for use, by initially "Giving" the Semaphore.
		}
	}

	xTaskCreate(
	TSL2591_reader
	,  "sensor_reader_TSL"  // A name just for humans
	,  configMINIMAL_STACK_SIZE  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  NULL
	,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
	xTaskCreate(
		HIH8120_reader
		,  "sensor_reader_HIH"  // A name just for humans
		,  configMINIMAL_STACK_SIZE  // This stack size can be checked & adjusted by reading the Stack Highwater
		,  NULL
		,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		,  NULL );
	
	xTaskCreate(
		MHZ19_reader
		,  "sensor_reader_MZ"  // A name just for humans
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
	 
	 xTaskCreate(
		lora_downlink_task
		,  "LR_down_link"  // A name just for humans
		,  configMINIMAL_STACK_SIZE+200  // This stack size can be checked & adjusted by reading the Stack Highwater
		,  NULL
		,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		,  NULL );
	
}

/** 
\note Task to read the HIH8120 sensor.
*/
void HIH8120_reader( void *pvParameters )
{
	hih8120results result;
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(60000UL); // Upload message every 5 minutes (300000 ms)
	xLastWakeTime = xTaskGetTickCount();
	for(;;)
	{
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
		if (xSemaphoreTake(xIOSemaphore,pdMS_TO_TICKS(100))==pdTRUE)
		{
			printf("\nHIH\n");
			result=readValueAll();
			Temperature= result.temp;
			Humidity= result.hum;
			aFunctionToSetBits(xCreatedEventGroup,1);
			//Giving back io semaphore.
			//printf("xIOsemaphore given sensor reader\n");
			xSemaphoreGive( ( xIOSemaphore ) );
		}else{
			//timed out
		}
		
	}
	}
	
/** 
\note Task for reading the MHZ19 sensor.
*/
void MHZ19_reader( void *pvParameters )
	{
		TickType_t xLastWakeTime;
		const TickType_t xFrequency = pdMS_TO_TICKS(60000UL); // Upload message every 5 minutes (300000 ms)
		xLastWakeTime = xTaskGetTickCount();
		for(;;)
		{
			xTaskDelayUntil( &xLastWakeTime, xFrequency );
			if (xSemaphoreTake(xIOSemaphore,pdMS_TO_TICKS(100))==pdTRUE)
			{
				printf("\nMHZ\n");
				CO2ppm= read_CO2_ppm();
				aFunctionToSetBits(xCreatedEventGroup,0);
				//Giving back io semaphore.
				//printf("xIOsemaphore given sensor reader\n");
				xSemaphoreGive( ( xIOSemaphore ) );
				}else{
				//timed out
			}
			
		}
	}

/** 
\note Task to read TSL2591 sensor.
*/
void TSL2591_reader( void *pvParameters )
	{
		TickType_t xLastWakeTime;
		const TickType_t xFrequency = pdMS_TO_TICKS(60000UL); // Upload message every 5 minutes (300000 ms)
		xLastWakeTime = xTaskGetTickCount();
		for(;;)
		{
			xTaskDelayUntil( &xLastWakeTime, xFrequency );
			if (xSemaphoreTake(xIOSemaphore,pdMS_TO_TICKS(100))==pdTRUE)
			{
				printf("\nTSL\n");
				read_lux();
				//the callback sets the bit.
				
				//Giving back io semaphore.
				//printf("xIOsemaphore given sensor reader\n");
				xSemaphoreGive( ( xIOSemaphore ) );
				}else{
				//timed out
			}
			
		}
	}
/** 
\note Function for sending LoRa uplink payload.
*/
void sendLoraPayload(){
	_uplink_payload.len = 8;
	_uplink_payload.portNo = 2;
	
	_uplink_payload.bytes[0] = Humidity >> 8;
	_uplink_payload.bytes[1] = Humidity & 0xFF;
	_uplink_payload.bytes[2] = Temperature >> 8;
	_uplink_payload.bytes[3] = Temperature & 0xFF;
	_uplink_payload.bytes[4] = CO2ppm >> 8;
	_uplink_payload.bytes[5] = CO2ppm & 0xFF;
	_uplink_payload.bytes[6] = Lux >> 8;
	_uplink_payload.bytes[7] = Lux & 0xFF;
	
	lora_driver_returnCode_t rc;
	
	if ((rc = lora_driver_sendUploadMessage(false, &_uplink_payload)) == LORA_MAC_TX_OK )
	{
		printf("Lora message has been sent!\n");
		
	}
	else if (rc == LORA_MAC_RX)
	{
		printf("Lora message has been sent down link message received!\n");
	}
}
/** 
\note Function responsible for waiting the event bits.
*/
void aFunctionToWaitBits( EventGroupHandle_t xEventGroup )
{
EventBits_t uxBits;
const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS;

  /* Wait a maximum of 100ms for either bit 0 or bit 4 to be set within
  the event group.  Clear the bits before exiting. */
  uxBits = xEventGroupWaitBits(
            xEventGroup,   /* The event group being tested. */
            BIT_0 | BIT_1 | BIT_2 , /* The bits within the event group to wait for. */
            pdFALSE,        /* BIT_0 & BIT_4 should not be cleared before returning. */
            pdTRUE,       /* Wait for both bits, either bit will do. */
            xTicksToWait );/* Wait a maximum of 100ms for either bit to be set. */

  if( ( uxBits & ( BIT_0 | BIT_1 | BIT_2) ) == ( BIT_0 | BIT_1 | BIT_2) )
  {
      /* xEventGroupWaitBits() returned because both bits were set. */
	  printf("Temp: %i \n", Temperature );
	  printf("Hum: %i \n", Humidity );
	  printf("CO2ppm: %i \n", CO2ppm);
	  Lux= get_lux_value();
	  printf("Lux: %i \n", Lux);

	  //lora up link send
	  sendLoraPayload();
	  aFunctionToClearBits( xEventGroup );
  }
  else if( ( uxBits & BIT_0 ) != 0 )
  {
      /* xEventGroupWaitBits() returned because just BIT_0 was set. */
  }
  if( ( uxBits & BIT_1 ) != 0 )
  {
      /* xEventGroupWaitBits() returned because just BIT_1 was set. */
  }
  if( ( uxBits & BIT_2 ) != 0 )
  {
	  // xEventGroupWaitBits() returned because just BIT_2 was set. 
	   //puts("2");
  }
  else
  {
      /* xEventGroupWaitBits() returned because xTicksToWait ticks passed
      without either BIT_0 or BIT_4 becoming set. */
	  
  }
}
/** 
\note Function responsible for clearing the event bits.
*/
void aFunctionToClearBits( EventGroupHandle_t xEventGroup )
{
EventBits_t uxBits;

  /* Clear bit 0 and bit 4 in xEventGroup. */
  uxBits = xEventGroupClearBits(
                                xEventGroup,  /* The event group being updated. */
                                BIT_0 | BIT_1 | BIT_2 );/* The bits being cleared. */

  if( ( uxBits & ( BIT_0 | BIT_1 | BIT_2 ) ) == ( BIT_0 | BIT_1| BIT_2 ) )
  {
      /* Both bit 0 and bit 4 were set before xEventGroupClearBits()
      was called.  Both will now be clear (not set). */
  }
  else if( ( uxBits & BIT_0 ) != 0 )
  {
      /* Bit 0 was set before xEventGroupClearBits() was called.  It will
      now be clear. */
  }
  else if( ( uxBits & BIT_1 ) != 0 )
  {
      /* Bit 1 was set before xEventGroupClearBits() was called.  It will
      now be clear. */
  }
  else if( ( uxBits & BIT_2 ) != 0 )
  {
      /* Bit 2 was set before xEventGroupClearBits() was called.  It will
      now be clear. */
  }
  else
  {
      /* Neither bit 0 nor bit 4 were set in the first place. */
  }
}
/** 
\note Function responsible for setting event bits. Takes two parameter an EventGroup and the number of the bit in the event group. 
*/
void aFunctionToSetBits( EventGroupHandle_t xEventGroup , int bit_No)
{
	EventBits_t uxBits;
	switch(bit_No){
		case 0:
			/* Set bit 0 and bit 4 in xEventGroup. */
			uxBits = xEventGroupSetBits(
			xEventGroup,    /* The event group being updated. */
			BIT_0 );/* The bits being set. */
			break;
		case 1:
			/* Set bit 0 and bit 4 in xEventGroup. */
			uxBits = xEventGroupSetBits(
			xEventGroup,    /* The event group being updated. */
			BIT_1 );/* The bits being set. */
			break;
		case 2:
			/* Set bit 0 and bit 4 in xEventGroup. */
			uxBits = xEventGroupSetBits(
			xEventGroup,    /* The event group being updated. */
			BIT_2 );/* The bits being set. */
			break;
	}
	

	if( ( uxBits & (  BIT_0 ) ) == (  BIT_0 ) )
	{
		/* bit 4 remained set when the function returned. */
	}
	if( ( uxBits & (  BIT_1 ) ) == (  BIT_1 ) )
	{
		/* bit 4 remained set when the function returned. */
	}
	if( ( uxBits & (  BIT_2 ) ) == (  BIT_2 ) )
	{
		/* bit 4 remained set when the function returned. */
	}
}
/** 
\note Function responsible for initializing all necessary drivers. 
*/
void initialiseSystem()
{
	xCreatedEventGroup = xEventGroupCreate();
	if( xCreatedEventGroup == NULL )
	{
		// The event group was not created because there was insufficient
		// FreeRTOS heap available.
	}
	else
	{
		
	}
	// Set output ports for leds used in the example
	DDRA |= _BV(DDA0) | _BV(DDA7);
	// Make it possible to use stdio on COM port 0 (USB) on Arduino board - Setting 57600,8,N,1
	stdio_initialise(ser_USART0);
	// Let's create some tasks
	create_tasks_and_semaphores();
	// Status Leds driver
	initialize_status_leds();
	mh_z19_initialise(ser_USART3);
	if ( HIH8120_OK == hih8120_initialise() )
	{
	}
	
	if ( TSL2591_OK == tsl2591_initialise(&tsl2591Callback) )
	{
		if (TSL2591_OK == tsl2591_enable()) {
		}
	}
	rc_servo_initialise();	
	// Initialize the LoRaWAN driver without down-link buffer
	downLinkMessageBufferHandle = xMessageBufferCreate(sizeof(lora_driver_payload_t)*2); // Here I make room for two downlink messages in the message buffer
	lora_driver_initialise(ser_USART1, downLinkMessageBufferHandle); // The parameter is the USART port the RN2483 module is connected to - in this case USART1 - here no message buffer for down-link messages are defined
	// Create LoRaWAN task and start it up with priority 3
	lora_handler_initialise(3);
}
	
/*--------------------------LORA---------------------------------*/
/** 
\note Function for creating LoRA set up task.
*/
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

/** 
\note Function for initializing LoRA connection.
*/
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
		status_leds_ledOn(led_ST2); 
		xSemaphoreGive( ( xIOSemaphore ) );
		//Lets stay here.
		while (1)
		{
			taskYIELD();// this gives thread to other task.
		}
		
	}
	else
	{
		status_leds_ledOff(led_ST2); 
		//This led is not working on our device.
		status_leds_fastBlink(led_ST1);
		xSemaphoreGive( ( xIOSemaphore ) );
		// Lets stay here.
		while (1)
		{
			taskYIELD();// this gives thread to other task.
		}
	}
}

/** 
\note Task for LoRA up link connection.
*/
void lora_uplink_task( void *pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(30000UL); // Upload message every 5 minutes (300000 ms)
	xLastWakeTime = xTaskGetTickCount();
	
	for(;;)
	{
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
		aFunctionToWaitBits(xCreatedEventGroup);
	}
}
/** 
\note Task for LoRA down link connection.
*/
void lora_downlink_task( void *pvParameters)
{

	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(3000UL); // Attempt reading every (3000 ms)
	xLastWakeTime = xTaskGetTickCount();
	
	for(;;)
	{
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
		if(xSemaphoreTake(xIOSemaphore,pdMS_TO_TICKS(100))==pdTRUE){
				xReceivedBytes=0;
				xReceivedBytes=xMessageBufferReceive(downLinkMessageBufferHandle, &downlinkPayload, sizeof(lora_driver_payload_t), pdMS_TO_TICKS(100));//let it timeout if there is no new message.
				
		
				if (xReceivedBytes>0)
				{
					message[0] = downlinkPayload.bytes[0];
					message[1] = downlinkPayload.bytes[1];
					if(message[0]==0){
						/*
						if(message[1]==0){
							printf("Vent Off\n");
							setPosition(100);
						}else
						if (message[1]==1)
						{
							printf("Vent On\n");
							setPosition(-100);
						}
						*/
						setPosition(100);
					}
					if (message[0]==1)
					{
						/*
						if(message[1]==0){
							printf("Light Off\n");
							setLED(4,0);
						}else
						if (message[1]==1)
						{
							printf("Light On\n");
							setLED(4,1);
						}
						*/
						setPosition(-100);
					}
					if (message[1] == 0)
					{
						setLED(4, 0);
					}
					if (message[1] == 1)
					{
						setLED(4, 1);
					}
				
				}else{
					printf("-");
				}
				xSemaphoreGive( ( xIOSemaphore ) );
		}
	}
}

/** 
\note Task for LoRA initialization.
*/
void lora_init_task(void *pvParameters)
{
	for(;;){
		
		if (xSemaphoreTake(xIOSemaphore,pdMS_TO_TICKS(100))==pdTRUE)
		{
			// Hardware reset of LoRaWAN transceiver
			lora_driver_resetRn2483(1);
			vTaskDelay(2);
			lora_driver_resetRn2483(0);
			// Give it a chance to wakeup
			vTaskDelay(150);

			lora_driver_flushBuffers(); // get rid of first version string from module after reset!
			_lora_setup();
		}
	}
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



