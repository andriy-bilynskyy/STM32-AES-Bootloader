/*
********************************************************************************
**
**  (C) 2020 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
**
**  This code is licensed under the MIT.
**
********************************************************************************
*/

#ifndef __BL_DATA_H
#define __BL_DATA_H


#include <stdint.h>
#include <stdbool.h>


/* ST store MCU flash size in KB at this address */
#define FLASH_SIZE_ADDR     (0x1FFFF7E0)
#define FLASH_TOTAL_SIZE    ((uint32_t)(*(const uint16_t *)FLASH_SIZE_ADDR) << 10)
#if !defined (STM32F10X_LD) || defined (STM32F10X_MD)
#define FLASH_PAGE_SIZE     (1024)
#elif  defined (STM32F10X_HD) || defined (STM32F10X_XL) || defined (STM32F10X_CL)
#define FLASH_PAGE_SIZE     (2048)
#else
#error Unknown STM device line.
#endif

uint32_t bl_data_get_first_addr(void);
uint32_t bl_data_get_last_addr(void);
uint32_t bl_data_get_app_first_addr(void);
uint32_t bl_data_get_app_last_addr(void);
bool bl_data_is_page_clean(uint32_t addr);


#endif
