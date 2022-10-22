
#if 0

#include "button.h"
#include "main.h"

uint8_t read_button_GPIO(uint8_t button_id)
{
	// you can share the GPIO read function with multiple Buttons
	switch(button_id)
	{
		case KEY0_id:
			return HAL_GPIO_ReadPin(KEY0_GPIO_Port,KEY0_Pin);

		case KEY1_id:
			return HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin);
		
		case WK_UP_id:
			return HAL_GPIO_ReadPin(WK_UP_GPIO_Port,WK_UP_Pin);
		
		default:
			return 0;
	}
}

void Event_KEY1_PRESS_DOWN_Callback(void* btn){

}

void Event_KEY1_DOUBLE_CLICK_Callback(void* btn){

}


#endif
