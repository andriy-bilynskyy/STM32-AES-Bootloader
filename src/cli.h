/*
********************************************************************************
**
**  (C) 2020 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
**
**  This code is licensed under the MIT.
**
********************************************************************************
*/

#ifndef __CLI_H
#define __CLI_H


#include "bl_conf.h"
#include <stdint.h>


void cli_init(void);
uint8_t cli_process(const char * input, char * output, uint32_t output_size);
uint32_t app_adderess(void);

#if defined(LED_PORT) && defined(LED_PIN)
void led_init(void);
void led_deinit(void);
void led_state(uint8_t on);

#define LED_INIT()          led_init()
#define LED_DEINIT()        led_deinit()
#define LED_STATE(on)       led_state(on)
#else
#define LED_INIT()
#define LED_DEINIT()
#define LED_STATE(on)
#endif


#endif
