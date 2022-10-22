

#include "serial.h"
#include <cstdio>
#include <cstdarg>
#include <cstdlib>

/**
 * @brief
 * @param _serial
 * @param fmt
 * @param ...
 */
static void SerialPrint(Serial *_serial,const char *fmt,...){
    if(_serial){
        if(_serial->initialized){
            int size = 0;
            char *p = _serial->_buf;
            va_list arg_list;
            va_start(arg_list,fmt);
            vsprintf(_serial->_buf,fmt,arg_list);
            while (*p++) size++;/*计算字符串大小*/
            HAL_UART_Transmit(
                    _serial->huart,
                    (uint8_t*)_serial->_buf,
                    size,
                    10
            );
            va_end(arg_list);
        }
    }
}

/**
 *
 */
Serial* SerialInit(UART_HandleTypeDef* _huart){
    auto *_serial = (Serial*)malloc(sizeof(Serial));
    _serial->initialized = true;
    _serial->huart = _huart;
    _serial->print = SerialPrint;
    return _serial;
}



int print(const char *fmt,...){
    int size = 0;
    char buf[SerialBufferSize];
    char *p = buf;

    va_list arg_list;
    va_start(arg_list,fmt);
    vsprintf(buf,fmt,arg_list);
    while (*p++) size++;/*计算字符串大小*/

    int ret = HAL_UART_Transmit(
            &SerialUartHandle,
            (uint8_t*)buf,
            size,
            10
    );

    va_end(arg_list);
    return ret;
}







