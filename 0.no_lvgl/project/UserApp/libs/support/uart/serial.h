/**********************************************
 * 如何使用？
 * 1.初始化↓↓↓
 * Serial *serial = SerialInit(&huart1);
 * 2.这样使用↓↓↓
 * serial->print(serial,"Ok,%d,%.2f",123,1.230);
 * 或者
 * 更改宏`SerialUartHandle`后
 * 可以直接使用print()打印串口
 **********************************************/

#ifndef _SERIAL_H
#define _SERIAL_H
#ifdef __cplusplus
extern "C" {
#endif
#if 1

/*********************
* include
*********************/
#include "main.h"
#include "usart.h"
/*********************
 * define
 *********************/
#define SerialUartHandle huart1
#define SerialBufferSize 128 /*byte*/
/*********************
 * typedef
 *********************/
typedef struct _serial{
    bool initialized;
    UART_HandleTypeDef* huart;
    void (*print)(struct _serial*,const char *,...);
    char _buf[SerialBufferSize];
} Serial;
/*********************
 * function
 *********************/
Serial* SerialInit(UART_HandleTypeDef* _huart);
int print(const char *,...);

#endif
#ifdef __cplusplus
}
#endif
#endif //_SERIAL_H
