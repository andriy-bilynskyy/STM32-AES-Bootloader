/*
********************************************************************************
**
**  (C) 2020 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
**
**  This code is licensed under the MIT.
**
********************************************************************************
*/


#ifndef __FREE_RTOS_STUB_H
#define __FREE_RTOS_STUB_H


#include <stddef.h>


/* CLI output buffer size */
#define configCOMMAND_INT_MAX_OUTPUT_SIZE   (523)


/* type definitions to compile freeRTOS CLI */
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

#define pdFALSE         ( ( BaseType_t ) 0 )
#define pdTRUE          ( ( BaseType_t ) 1 )
#define pdPASS          ( pdTRUE )
#define pdFAIL          ( pdFALSE )


#endif
