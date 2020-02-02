/*
********************************************************************************
**
**  (C) 2020 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
**
**  This code is licensed under the MIT.
**
********************************************************************************
*/

#ifndef __BL_CONF_H
#define __BL_CONF_H

/* bootloader jumper BL/app# */
#define BL_PIN     GPIO_Pin_2
#define BL_PORT    GPIOB

/* bootloader LED */
#define LED_PIN     GPIO_Pin_13
#define LED_PORT    GPIOC
/* control LED from CLI */
//#define LED_CLI_CTL


#ifdef DEBUG
#include <stdio.h>

extern void initialise_monitor_handles(void);

#define DBG_INIT() initialise_monitor_handles()
#define DBG_OUT(fmt, ...)   \
                printf("%s:%04u " fmt "\n", __FILE__, (unsigned)__LINE__, ##__VA_ARGS__)
#else
#define DBG_INIT()
#define DBG_OUT(fmt, ...)
#endif


#endif
