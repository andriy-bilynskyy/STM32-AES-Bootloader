/*
********************************************************************************
**
**  (C) 2023 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
**
**  This code is licensed under the MIT.
**
********************************************************************************
*/


#ifndef __FREE_RTOS_TASK_STUB_H
#define __FREE_RTOS_TASK_STUB_H


#include <stm32f10x_conf.h>
#include <stdlib.h>


#define configASSERT(expr) assert_param(expr)
#define pvPortMalloc(size) malloc(size)
#define taskENTER_CRITICAL()
#define taskEXIT_CRITICAL()


#endif
