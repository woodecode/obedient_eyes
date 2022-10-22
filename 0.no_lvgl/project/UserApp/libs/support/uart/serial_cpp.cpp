#if 1

#include "serial_cpp.h"
#include <stdio.h>
#include <stdarg.h>

Serial::Serial(UART_HandleTypeDef* _huart) {
    this->huart = _huart;
}

int Serial::print(const char *fmt, ...) {
    this->buf_size = 0;
    char *p = this->buf;
    va_list arg_list;
    va_start(arg_list,fmt);
    vsprintf(buf,fmt,arg_list);
    while (*p++){
        this->buf_size++;
    }
    int ret = HAL_UART_Transmit(this->huart,(uint8_t*)this->buf,this->buf_size,10);
    va_end(arg_list);
    return ret;
}

#endif

