
#if 1

#include "./button.h"
#include "main.h"

uint8_t read_button_GPIO(uint8_t button_id)
{
	// you can share the GPIO read function with multiple Buttons
	switch(button_id)
	{
		case KEY0_id:
			return HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_10);
		case KEY1_id:
			return HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_10);
		case WK_UP_id:
			return HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_10);
		default:
			return 0;
	}
}

/**
 * @brief    
 * @param 
 */
void Event_KEY1_PRESS_DOWN_Callback(void* btn){

}

/**
 * @brief    
 * @param 
 */
void Event_KEY1_DOUBLE_CLICK_Callback(void* btn){

}


#endif
