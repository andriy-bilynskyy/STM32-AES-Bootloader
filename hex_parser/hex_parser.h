/*
*****************************************************************************
**
**  (C) 2023 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
**
**  This code is licensed under the MIT.
**
*****************************************************************************
*/

#ifndef __HEX_PARSER_H
#define __HEX_PARSER_H

#include <stdint.h>
#include <stdbool.h>


typedef bool (*hex_parser_on_data_t)(uint32_t addr, uint8_t data[], uint16_t len);


bool hex_parser_feed(char *str, bool data_upd);
void hex_parser_set_callback(hex_parser_on_data_t on_data);


#endif
