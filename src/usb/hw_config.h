/*
********************************************************************************
**
**  (C) 2023 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
**
**  This code is licensed under the MIT.
**
********************************************************************************
*/

#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H


#include <stm32f10x_conf.h>

void usb_cdc_start(void);
void usb_cdc_stop(void);
uint8_t usb_cdc_is_ready(void);
uint8_t usb_cdc_rx_avail(void);
int usb_cdc_read(uint8_t data[], uint16_t len);
int usb_cdc_write(const uint8_t data[], uint16_t len);
char usb_cdc_get_char(void);
void usb_cdc_put_char(char ch);
void usb_cdc_put_string(const char *str);


#endif
