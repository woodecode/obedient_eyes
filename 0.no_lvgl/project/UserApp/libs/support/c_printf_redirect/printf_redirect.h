
#if 1

#ifndef _RETARGET_H
#define _RETARGET_H

#include "main.h"
#include <stdio.h>

// 使用平台
#define USE_CMAKE_TOOLCHAINS 1	//Clion and Cmake
#define USE_KEIL_PLATFORM 0		//Keil
// **********************************

#if USE_CMAKE_TOOLCHAINS
#include <sys/stat.h>
#include "stm32h7xx_hal.h"

int _isatty(int fd);

int _write(int fd, char *ptr, int len);

int _close(int fd);

int _lseek(int fd, int ptr, int dir);

int _read(int fd, char *ptr, int len);

int _fstat(int fd, struct stat *st);

#elif USE_KEIL_PLATFORM

#endif // USE_CMAKE_TOOLCHAINS



#endif //EMBEDDED_SPI_SD_RETARGET_H


#endif
