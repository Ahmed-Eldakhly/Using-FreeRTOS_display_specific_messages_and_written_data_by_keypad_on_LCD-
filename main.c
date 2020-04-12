



#include "UserTasks.h"


void main(void)
{
	xTaskCreate(init_Task  , "Init_Task" , configMINIMAL_STACK_SIZE ,
			NULL , (4 | portPRIVILEGE_BIT) , &InitTask_Flag);

	/* Start Scheduler */
	vTaskStartScheduler();

	while(1)
	{

	}
}

