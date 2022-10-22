/**********************************************
 * How to use this？
 * 1.init↓↓↓
 * auto sp = new Serial(&huart1);
 * 2.using like this↓↓↓
 * sp->print("sd,%4d,%.2f",56,1.223);
 **********************************************/

#ifndef _SERIAL_CPP_H
#define _SERIAL_CPP_H
#if 1
#include "usart.h"
#define SerialCppBufferSize 128
class Serial{
public:
    typedef enum {
        B8 = 8,
        B9
    } _wordLength;
    typedef enum {
        STOPBITS_1 = 1,
        STOPBITS_2
    } _stopBits;
    typedef enum {
        NONE,
        EVEN,
        ODD
    } _parity;
private:
    UART_HandleTypeDef *huart;
    char buf[SerialCppBufferSize];
    int buf_size;
public:
    explicit Serial(UART_HandleTypeDef* _huart);
    // void setProperties(uint32_t BaudRate,_wordLength WordLength,_stopBits StopBits,_parity Parity);
    int print(const char *fmt,...);
};

#endif
#endif //_SERIAL_CPP_H
