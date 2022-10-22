/*Main Include*/
#include "include.h"
/*Drivers*/
#include "main.h"
#include "usart.h"
#include "tim.h"
// #include "cmsis_os.h" /*for FreeRTOS*/
/*Library*/
#include "libs/support/uart/serial_cpp.h"
#include "libs/gui/interface/include/alientek_lcd.h"
/*App*/
#include "app/include/button.h"

/*-------NOTE-------
1.FreeRTOS Task configuration
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
	.name 		= "defaultTask",
	.stack_size = 128 * 4,
	.priority 	= (osPriority_t) osPriorityNormal,
};
2.FreeRTOS load Task
defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
3.FreeRTOS Task implement
void StartDefaultTask(void *argument) {
  for(;;) {
    osDelay(1);
  }
}
  -------END--------*/

Serial *sp = new Serial(&huart1);

void Main(){
	
	/* FreeRTOS Init */
	
	
	
	
	// HAL_TIM_Base_Start_IT(&htim7);
	
	/*If FreeRTOS is used, the code block below should be removed*/
	for(;;){
		/* do something ... */
	}
}


/* stm32XXxx_it.c		extern void TimxCallback1kHz(); 	*/
/* TIMx_IRQHandler()	TimxCallback1kHz();return;			*/
/* main.cpp				HAL_TIM_Base_Start_IT(&htimx);		*/
extern "C" void Tim7Callback1kHz(){
	
	static unsigned int freq_25Hz = 0;
	
	__HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);

	if(freq_25Hz++ >= 40){
		freq_25Hz = 0;
		LCD_Fill(10,10,100,100,0x001f);
	}
}









