
#if 0

#ifndef _BUTTON_IMPLEMENT_H_
#define _BUTTON_IMPLEMENT_H_


#include "multi_button.h"


enum Button_IDs {
	KEY0_id,
	KEY1_id,
	WK_UP_id,
};

uint8_t read_button_GPIO(uint8_t button_id);

// Callback Function
void Event_KEY1_PRESS_DOWN_Callback(void* btn);
void Event_KEY1_DOUBLE_CLICK_Callback(void* btn);
void Event_KEY0_DOUBLE_CLICK_Callback(void* btn);

#endif

#endif



