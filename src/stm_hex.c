/*
*****************************************************************************
**
**  (C) 2020 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
**
**  This code is licensed under the MIT.
**
*****************************************************************************
*/

#include "stm_hex.h"
#include <stddef.h>
#include "hex_parser.h"
#include "aes.h"


extern const uint8_t _binary_keys_iv_bin_start[];
extern const uint8_t _binary_keys_iv_bin_end[];
extern const uint8_t _binary_keys_key_bin_start[];
extern const uint8_t _binary_keys_key_bin_end[];


static stm_hex_on_word_t stm_hex_on_word = NULL;
static struct AES_ctx ctx;


static bool new_data(uint32_t addr, uint8_t data[], uint16_t len);
static bool hex_parser_on_data(bool finish, uint32_t addr, uint8_t data);


bool stm_hex_feed(const char * str) {
    return hex_parser_feed((char *)str, false);
}

void stm_hex_set_callback(stm_hex_on_word_t on_word) {
    stm_hex_on_word = on_word;
    hex_parser_set_callback(new_data);
    AES_init_ctx_iv(&ctx, _binary_keys_key_bin_start, _binary_keys_iv_bin_start);
}


static bool new_data(uint32_t addr, uint8_t data[], uint16_t len) {
    bool result = true;

    if(data) {
        AES_CTR_xcrypt_buffer(&ctx, data, len);
        for(uint16_t i = 0; i < len && result; i++) {
            result = hex_parser_on_data(false, addr + i, data[i]);
        }
    } else {
        result = hex_parser_on_data(true, 0, 0);
    }
    return result;
}

static bool hex_parser_on_data(bool finish, uint32_t addr, uint8_t data) {
    static bool     even_valid = false;
    static uint32_t even_addr  = 0;
    static uint8_t  even_data  = 0;

    bool result = true;

    if(finish) {
        if(even_valid && stm_hex_on_word) {
            result = stm_hex_on_word(even_addr, (uint16_t)0xFF00 | even_data);
        }
        even_valid = false;
    } else if(addr & 0x01) {
        if(stm_hex_on_word) {
            if(even_valid) {
                if(addr - even_addr == 1) {
                    result = stm_hex_on_word(even_addr, ((uint16_t)data << 8) | even_data);
                } else {
                    result = stm_hex_on_word(even_addr, (uint16_t)0xFF00 | even_data);
                    if(result) {
                        result = stm_hex_on_word(addr - 1, ((uint16_t)data << 8) | 0xFF);
                    }
                }
            } else {
                result = stm_hex_on_word(addr - 1, ((uint16_t)data << 8) | 0xFF);
            }
        }
        even_valid = false;
    } else {
        if(even_valid && stm_hex_on_word) {
            result = stm_hex_on_word(even_addr, (uint16_t)0xFF00 | even_data);
        }
        even_valid = true;
        even_addr = addr;
        even_data = data;
    }

    return result;
}
