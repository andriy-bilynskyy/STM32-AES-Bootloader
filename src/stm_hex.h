/*
*****************************************************************************
**
**  (C) 2023 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
**
**  This code is licensed under the MIT.
**
*****************************************************************************
*/

#ifndef __STM_HEX_H
#define __STM_HEX_H

#include <stdint.h>
#include <stdbool.h>


typedef bool (*stm_hex_on_word_t)(uint32_t addr, uint16_t data);


bool stm_hex_feed(const char *str);
void stm_hex_set_callback(stm_hex_on_word_t on_word);


#endif
