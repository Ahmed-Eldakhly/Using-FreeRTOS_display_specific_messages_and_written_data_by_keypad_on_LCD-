/*****************************************************************************

 * Module 	  : FreeRTOS User Story2 Tasks implementation
 *
 * File name  : UserStory1.c
 *
 * Created on : Oct 7, 2019
 *
 * Author     : Ahmed Eldakhly & Hesham Hafez
 ******************************************************************************/

/*******************************************************************************
 *                       	Included Libraries                                 *
 *******************************************************************************/
#include "UserTasks.h"
#include "queue.h"

/*******************************************************************************
 *                           static Global Variables                           *
 *******************************************************************************/
static TaskHandle_t  AllHardwareInit_Flag = NULL;
/*Queue between PushButtonTask & LCD Task*/
static xQueueHandle MessageQueue_PushButton;

/*Queue between Send Hallo & LCD Task*/
static xQueueHandle MessageQueue_Hello;

/*******************************************************************************
 *                           Global Variables                    		       *
 *******************************************************************************/
TaskHandle_t  InitTask_Flag = NULL;

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/

/*******************************************************************************
 * Function Name:	init_Task
 *
 * Description: 	Create all tasks in the system
 *
 * Inputs:			pointer to void
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
void init_Task(void * a_Task_ptr)
{
	/*Create Queues*/
	MessageQueue_PushButton = xQueueCreate( 3 , sizeof(uint8));
	MessageQueue_Hello= xQueueCreate( 3 , sizeof(uint8));

	/*Create 4 Tasks*/
	xTaskCreate(All_Hardware_Init_Task  , "InitTasks" , configMINIMAL_STACK_SIZE ,
			NULL , (5 | portPRIVILEGE_BIT) , &AllHardwareInit_Flag);
	xTaskCreate(PushButton_Task  , "PushButton_Task" , configMINIMAL_STACK_SIZE ,
			NULL , (3 | portPRIVILEGE_BIT) , NULL);
	xTaskCreate(LCD_Task  , "LCD_Task" , configMINIMAL_STACK_SIZE ,
			NULL , (1 | portPRIVILEGE_BIT) , NULL);
	xTaskCreate(SendHallo_Task  , "SendHallo_Task" , configMINIMAL_STACK_SIZE ,
			NULL , (2 | portPRIVILEGE_BIT) , NULL);
	vTaskSuspend( InitTask_Flag);
}

/*******************************************************************************
 * Function Name:	All_Hardware_Init_Task
 *
 * Description: 	Initialize LCD & KeyPad
 *
 * Inputs:			pointer to void
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
void All_Hardware_Init_Task(void * a_Task_ptr)
{

	while(1)
	{
		PushButton_Init();
		LCD_init();
		vTaskSuspend( AllHardwareInit_Flag);
	}
}

/*******************************************************************************
 * Function Name:	PushButton_Task
 *
 * Description: 	Push Button Task
 *
 * Inputs:			pointer to void
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
void PushButton_Task(void * a_Task_ptr)
{
	/* Data is sent to LCD by Queue*/
	uint8 SendData = 1;
	/*de_bouncing Flag to make sure key is pressed*/
	uint8 Debouncing = 0;

	/*comparing value to check if key is pressed*/
	uint8 OldPressedValue = 0;

	while(1)
	{
		/*Check if Button is pressed and enable de_bouncing*/
		if(Debouncing == 0 && Buttons_getPressedButton()==1)
		{
			OldPressedValue =  Buttons_getPressedButton();
			Debouncing++;
			/*de_bouncing Delay*/
			vTaskDelay(10);
		}
		else if(Debouncing == 1)
		{
			SendData = 1;
			/*Send push Button was pressed to Lcd by queue*/
			xQueueSend(MessageQueue_PushButton , &SendData , 5);
			/*check if button is still pressed after de_bouncing time*/
			while(OldPressedValue == Buttons_getPressedButton())
			{

				vTaskDelay(100);
			}
			Debouncing = 2;
		}
		else if(Debouncing == 2)
		{
			SendData = 0;
			/*Send push Button was released to Lcd by queue*/
			xQueueSend(MessageQueue_PushButton , &SendData , 5);
			Debouncing = 0;
			vTaskDelay(100);
		}
		else
		{
			/*Send push Button is in Idle state to Lcd by queue*/
			SendData = 3;
			xQueueSend(MessageQueue_PushButton , &SendData , 5);
			vTaskDelay(100);
		}
	}
}

/*******************************************************************************
 * Function Name:	LCD_Task
 *
 * Description: 	LCD Task
 *
 * Inputs:			pointer to void
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
void LCD_Task(void * a_Task_ptr)
{
	vTaskDelay(100);
	/*received data from Push Button Task to display button state on LCD*/
	uint8 PushButtonTask_Data;

	/*received message from Hello Task to display button state on LCD*/
	uint8 HelloTask_Data;

	/*Push Button "Pressed" message*/
	uint8 ButtonPressedString[] = "Over Written!!!!";

	/*Hello message*/
	uint8 MessageString[] = "Hello LCD !!";
	/*Clear one line*/
	uint8 Spaces[] =  "                 ";

	/*counter to hundle displaying of hello message*/
	uint8 Counter = 0;

	/*Synchronize between displaying of Push Button task and hello task*/
	uint8 Flag = 0;
	uint8 Flag2 = 0;

	while(1)
	{
		/*receive messages fro other tasks by queues*/
		xQueueReceive(MessageQueue_PushButton , &PushButtonTask_Data , 100);
		xQueueReceive(MessageQueue_Hello , &HelloTask_Data , 100);

		/*Increment counter to Display Hello message for specific time*/
		if(Counter != 0)
		{
			Counter++;
		}
		else
		{
			/*Do Nothing*/
		}

		/*check on Push Button Message is received*/
		if(PushButtonTask_Data == 1)
		{
			LCD_displayStringRowColumn(1 , 0 , ButtonPressedString);
			PushButtonTask_Data = 5;
			Flag = 1;
		}
		else if(PushButtonTask_Data == 0)
		{
			Flag = 0;
			if(Flag2 == 1)
			{
				LCD_displayStringRowColumn(1 , 0 ,Spaces);
				LCD_displayStringRowColumn(0 , 0 , MessageString);
			}
			else
			{
				LCD_clearScreen();
			}
		}

		/*check on Hello Message is received*/
		if(HelloTask_Data == 2)
		{
			LCD_displayStringRowColumn(0 , 0 , MessageString);
			if(Counter == 0)
			{
				Counter++;
			}

			Flag2 = 1;
			HelloTask_Data = 0;
		}
		else
		{
			/*Do Nothing*/
		}

		/*Reset counter to Display off Hello message for specific time*/
		if(Counter == 10)
		{
			Counter = 0;
			Flag2 = 0;
			if(Flag == 1)
			{
				LCD_displayStringRowColumn(0 , 0 ,Spaces);
				LCD_displayStringRowColumn(1 , 0 , ButtonPressedString);
			}
			else
			{
				LCD_clearScreen();
			}
		}
		else
		{
			/*Do Nothing*/
		}
		vTaskDelay(20);

	}
}

/*******************************************************************************
 * Function Name:	SendHallo_Task
 *
 * Description: 	Send Message to LCD
 *
 * Inputs:			pointer to void
 *
 * Outputs:			NULL
 *
 * Return:			NULL
 *******************************************************************************/
void SendHallo_Task(void * a_Task_ptr)
{
	/* Data is sent to LCD by Queue*/
	uint8 SendData = 2;
	while(1)
	{
		xQueueSend(MessageQueue_Hello , &SendData , 10);
		vTaskDelay(400);
	}
}
