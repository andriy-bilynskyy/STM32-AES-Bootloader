/*
********************************************************************************
**
**  (C) 2020 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
**
**  This code is licensed under the MIT.
**
********************************************************************************
*/

#include "bl_data.h"
#include "stm32f10x_conf.h"


extern char _sidata;
extern char _edata;
extern char _sdata;


uint32_t bl_data_get_first_addr(void) {
    return FLASH_BASE;
}

uint32_t bl_data_get_last_addr(void) {
    return (uint32_t)(&_sidata + (&_edata - &_sdata)) - 1;
}

uint32_t bl_data_get_app_first_addr(void) {
    return ((bl_data_get_last_addr() + FLASH_PAGE_SIZE) / FLASH_PAGE_SIZE) * FLASH_PAGE_SIZE;
}

uint32_t bl_data_get_app_last_addr(void) {
    return FLASH_BASE + FLASH_TOTAL_SIZE - 1;
}

bool bl_data_is_page_clean(uint32_t addr) {
    bool result = true;
    uint32_t saddr = (addr / FLASH_PAGE_SIZE) * FLASH_PAGE_SIZE;
    uint32_t eaddr = saddr + FLASH_PAGE_SIZE - 1;

    for(uint32_t i = saddr; i <= eaddr; i += sizeof(uint32_t)) {
        if(*(const uint32_t *)i != 0xFFFFFFFF) {
            result = false;
            break;
        }
    }
    return result;
}
