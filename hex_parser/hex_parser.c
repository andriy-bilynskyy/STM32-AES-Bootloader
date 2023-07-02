/*
*****************************************************************************
**
**  (C) 2023 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
**
**  This code is licensed under the MIT.
**
*****************************************************************************
*/

#include "hex_parser.h"
#include <stdlib.h>
#include <string.h>


#define IHEX_START_POS      (0)
#define IHEX_LENGTH_POS     (1)
#define IHEX_ADDR_POS       (3)
#define IHEX_RECT_POS       (7)
#define IHEX_DATA_POS       (9)

#define IHEX_START_BYTE     (':')
#define IHEX_UTIL_DATA      (11)

#define IHEX_RECT_DATA      (00)
#define IHEX_RECT_EOF       (01)
#define IHEX_RECT_ESA       (02)
#define IHEX_RECT_SSA       (03)
#define IHEX_RECT_ELA       (04)
#define IHEX_RECT_SLA       (05)


static hex_parser_on_data_t hex_parser_on_data = NULL;


static bool str2byte(const char *str, uint8_t *bt);
static void byte2str(char *str, uint8_t bt);


bool hex_parser_feed(char *str, bool data_upd)
{
    static uint32_t addr_offs = 0;
    bool result = false;
    do {
        if (str[IHEX_START_POS] != IHEX_START_BYTE) {
            break;
        }
        uint8_t len;
        if (!str2byte(&str[IHEX_LENGTH_POS], &len)) {
            break;
        }
        if (strlen(str) != ((uint16_t)len << 1) + IHEX_UTIL_DATA) {
            break;
        }
        uint8_t crc = 0, crc_beg = 0;
        for (uint16_t i = IHEX_LENGTH_POS; i < ((uint16_t)len << 1) + IHEX_UTIL_DATA; i += 2) {
            uint8_t bt;
            if (!str2byte(&str[i], &bt)) {
                crc = 1;
                break;
            } else {
                crc += bt;
                if (i < IHEX_DATA_POS) {
                    crc_beg += bt;
                }
            }
        }
        if (crc) {
            break;
        }
        uint16_t addr;
        if (!str2byte(&str[IHEX_ADDR_POS], (uint8_t *)&addr + 1)) {
            break;
        }
        if (!str2byte(&str[IHEX_ADDR_POS + 2], (uint8_t *)&addr)) {
            break;
        }
        uint8_t rect;
        if (!str2byte(&str[IHEX_RECT_POS], &rect)) {
            break;
        }
        switch (rect) {
        case IHEX_RECT_DATA:
            result = true;
            uint8_t *data = malloc(len);
            if (data) {
                for (uint16_t i = 0; i < len && result; i++) {
                    if (!str2byte(&str[IHEX_DATA_POS + (i << 1)], &data[i])) {
                        result = false;
                    }
                }
                if (result && hex_parser_on_data) {
                    result = hex_parser_on_data(addr_offs + addr, data, len);
                    if (result && data_upd) {
                        for (uint16_t i = 0; i < len; i++) {
                            crc_beg += data[i];
                            byte2str(&str[IHEX_DATA_POS + (i << 1)], data[i]);
                        }
                        crc_beg = (uint8_t)0x1 + (uint8_t)(~crc_beg);
                        byte2str(&str[IHEX_DATA_POS + (len << 1)], crc_beg);
                    }
                }
                free(data);
            } else {
                result = false;
            }
            break;
        case IHEX_RECT_EOF:
            result = hex_parser_on_data(0, NULL, 0);
            addr_offs = 0;
            break;
        case IHEX_RECT_ESA: {
            uint16_t tmp;
            if (!str2byte(&str[IHEX_DATA_POS], (uint8_t *)&tmp + 1)) {
                break;
            }
            if (!str2byte(&str[IHEX_DATA_POS + 2], (uint8_t *)&tmp)) {
                break;
            }
            addr_offs = (uint32_t)tmp << 4;
            result = true;
        }
        break;
        case IHEX_RECT_SSA:
            result = true;
            break;
        case IHEX_RECT_ELA: {
            uint16_t tmp;
            if (!str2byte(&str[IHEX_DATA_POS], (uint8_t *)&tmp + 1)) {
                break;
            }
            if (!str2byte(&str[IHEX_DATA_POS + 2], (uint8_t *)&tmp)) {
                break;
            }
            addr_offs = (uint32_t)tmp << 16;
            result = true;
        }
        break;
        case IHEX_RECT_SLA:
            result = true;
            break;
        default:
            break;
        }
    } while (0);
    return result;
}

void hex_parser_set_callback(hex_parser_on_data_t on_data)
{
    hex_parser_on_data = on_data;
}


static bool str2byte(const char *str, uint8_t *bt)
{
    bool result = true;
    *bt = 0;
    for (uint8_t i = 0; i < 2; i++) {
        *bt <<= 4;
        if (str[i] >= '0' && str[i] <= '9') {
            *bt |= str[i] - '0';
        } else if (str[i] >= 'A' && str[i] <= 'F') {
            *bt |= str[i] - 'A' + 0xA;
        } else if (str[i] >= 'a' && str[i] <= 'f') {
            *bt |= str[i] - 'a' + 0xA;
        } else {
            result = false;
            break;
        }
    }
    return result;
}

static void byte2str(char *str, uint8_t bt)
{
    static const char chr[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                               '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
                              };
    str[0] = chr[bt >> 4];
    str[1] = chr[bt & 0xF];
}
