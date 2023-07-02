/*
********************************************************************************
**
**  (C) 2023 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
**
**  This code is licensed under the MIT.
**
********************************************************************************
*/

#include "cli.h"
#include <stm32f10x_conf.h>
#include <FreeRTOS.h>
#include <FreeRTOS_CLI.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "bl_data.h"
#include "stm_hex.h"


static const char cmd_done[] =          "Command done.\r\n\r\n";
#ifdef LED_CLI_CTL
static const char cmd_wrong_param[] =   "Wrong parameter. See help.\r\n\r\n";
#endif
static char program_status[256] =       "NA.\r\n\r\n";
static bool command_mode = true;


static bool on_flash_word(uint32_t addr, uint16_t data)
{
    bool result = false;
    if (addr >= bl_data_get_app_first_addr() && addr < bl_data_get_app_last_addr()) {
        if (FLASH_ProgramHalfWord(addr, data) == FLASH_COMPLETE) {
            result = true;
        } else {
            uint32_t len = strlen(program_status);
            strncpy(&program_status[len], "Flash error at: 0x", sizeof(program_status) - len);
            len = strlen(program_status);
            __utoa(addr, &program_status[len], 16);
            len = strlen(program_status);
            strncpy(&program_status[len], "\r\n", sizeof(program_status) - len);
        }
    } else {
        uint32_t len = strlen(program_status);
        strncpy(&program_status[len], "Access out of app: 0x", sizeof(program_status) - len);
        len = strlen(program_status);
        __utoa(addr, &program_status[len], 16);
        len = strlen(program_status);
        strncpy(&program_status[len], "\r\n", sizeof(program_status) - len);
    }
    return result;
}


#if defined(LED_PORT) && defined(LED_PIN)

static bool led_locked = false;

void led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct_LED;
    GPIO_InitStruct_LED.GPIO_Pin = LED_PIN;
    GPIO_InitStruct_LED.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct_LED.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(LED_PORT, &GPIO_InitStruct_LED);
    led_state(0);
}

void led_deinit(void)
{
    led_state(0);
    GPIO_InitTypeDef GPIO_InitStruct_LED;
    GPIO_InitStruct_LED.GPIO_Pin = LED_PIN;
    GPIO_InitStruct_LED.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStruct_LED.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(LED_PORT, &GPIO_InitStruct_LED);
}

void led_state(uint8_t on)
{
    if (!led_locked) {
        if (on) {
            GPIO_ResetBits(LED_PORT, LED_PIN);
        } else {
            GPIO_SetBits(LED_PORT, LED_PIN);
        }
    }
}


#ifdef LED_CLI_CTL
static BaseType_t cmd_led_ctl(char *pcWriteBuffer, size_t xWriteBufferLen,
                              const char *pcCommandString)
{
    BaseType_t len = 0;
    const char *param = FreeRTOS_CLIGetParameter(pcCommandString, 1, &len);
    if (!strncmp(param, "on", len)) {
        led_locked = true;
        GPIO_ResetBits(LED_PORT, LED_PIN);
        strncpy(pcWriteBuffer, cmd_done, xWriteBufferLen);
    } else if (!strncmp(param, "off", len)) {
        led_locked = true;
        GPIO_SetBits(LED_PORT, LED_PIN);
        strncpy(pcWriteBuffer, cmd_done, xWriteBufferLen);
    } else if (!strncmp(param, "unlock", len)) {
        led_locked = false;
        strncpy(pcWriteBuffer, cmd_done, xWriteBufferLen);
    } else {
        strncpy(pcWriteBuffer, cmd_wrong_param, xWriteBufferLen);
    }
    return pdFALSE;
}

static const CLI_Command_Definition_t led_ctl = {
    "led",
    "led <state>:\r\n Set LED state on/off/unlock\r\n\r\n",
    cmd_led_ctl,
    1
};
#endif

#endif


static BaseType_t cmd_info(char *pcWriteBuffer, size_t xWriteBufferLen,
                           const char *pcCommandString)
{
    static uint8_t iteration = 0;
    BaseType_t result = pdTRUE;
    switch (iteration) {
    case 0:
        strncpy(pcWriteBuffer, "Boot Loader: 0x", xWriteBufferLen);
        iteration = 1;
        break;
    case 1:
        __utoa(bl_data_get_first_addr(), pcWriteBuffer, 16);
        iteration = 2;
        break;
    case 2:
        strncpy(pcWriteBuffer, " - 0x", xWriteBufferLen);
        iteration = 3;
        break;
    case 3:
        __utoa(bl_data_get_last_addr(), pcWriteBuffer, 16);
        iteration = 4;
        break;
    case 4:
        strncpy(pcWriteBuffer, " (", xWriteBufferLen);
        iteration = 5;
        break;
    case 5:
        __utoa(bl_data_get_last_addr() - bl_data_get_first_addr() + 1, pcWriteBuffer, 10);
        iteration = 6;
        break;
    case 6:
        strncpy(pcWriteBuffer, " bytes)\r\n", xWriteBufferLen);
        iteration = 7;
        break;
    case 7:
        strncpy(pcWriteBuffer, "Application: 0x", xWriteBufferLen);
        iteration = 8;
        break;
    case 8:
        __utoa(bl_data_get_app_first_addr(), pcWriteBuffer, 16);
        iteration = 9;
        break;
    case 9:
        strncpy(pcWriteBuffer, " - 0x", xWriteBufferLen);
        iteration = 10;
        break;
    case 10:
        __utoa(bl_data_get_app_last_addr(), pcWriteBuffer, 16);
        iteration = 11;
        break;
    case 11:
        strncpy(pcWriteBuffer, " (", xWriteBufferLen);
        iteration = 12;
        break;
    case 12:
        __utoa(bl_data_get_app_last_addr() - bl_data_get_app_first_addr() + 1, pcWriteBuffer, 10);
        iteration = 13;
        break;
    case 13:
        strncpy(pcWriteBuffer, " bytes)\r\n", xWriteBufferLen);
        iteration = 14;
        break;
    case 14:
        strncpy(pcWriteBuffer, "Page size: ", xWriteBufferLen);
        iteration = 15;
        break;
    case 15:
        __utoa(FLASH_PAGE_SIZE, pcWriteBuffer, 10);
        iteration = 16;
        break;
    case 16:
        strncpy(pcWriteBuffer, " bytes\r\n", xWriteBufferLen);
        iteration = 17;
        break;
    default:
        strncpy(pcWriteBuffer, "\r\n", xWriteBufferLen);
        iteration = 0;
        result = pdFALSE;
        break;
    }
    return result;
}

static const CLI_Command_Definition_t info = {
    "info",
    "info:\r\n Show chip info\r\n\r\n",
    cmd_info,
    0
};


static BaseType_t cmd_erase(char *pcWriteBuffer, size_t xWriteBufferLen,
                            const char *pcCommandString)
{
    static uint32_t erase_addr = 0;
    BaseType_t result = pdTRUE;
    if (!erase_addr) {
        FLASH_Unlock();
        FLASH_ClearFlag(FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
        erase_addr = bl_data_get_app_first_addr();
    }
    if (erase_addr <= bl_data_get_app_last_addr()) {
        if (bl_data_is_page_clean(erase_addr)) {
            strncpy(pcWriteBuffer, "-", xWriteBufferLen);
        } else {
            if (FLASH_ErasePage(erase_addr) == FLASH_COMPLETE) {
                strncpy(pcWriteBuffer, "*", xWriteBufferLen);
            } else {
                strncpy(pcWriteBuffer, "!", xWriteBufferLen);
            }
        }
        erase_addr += FLASH_PAGE_SIZE;
    } else {
        FLASH_Lock();
        strncpy(pcWriteBuffer, "\r\n", xWriteBufferLen);
        strncpy(&pcWriteBuffer[2], cmd_done, xWriteBufferLen - 2);
        erase_addr = 0;
        result = pdFALSE;
    }
    return result;
}

static const CLI_Command_Definition_t erase = {
    "erase",
    "erase:\r\n Erase application\r\n\r\n",
    cmd_erase,
    0
};


static BaseType_t cmd_program(char *pcWriteBuffer, size_t xWriteBufferLen,
                              const char *pcCommandString)
{
    BaseType_t result = cmd_erase(pcWriteBuffer, xWriteBufferLen, pcCommandString);
    if (result == pdFALSE) {
        strncpy(pcWriteBuffer, "\r\nNow send your .hex file as ASCII.\r\n\r\n", xWriteBufferLen);
        stm_hex_set_callback(on_flash_word);
        program_status[0] = '\0';
        FLASH_Unlock();
        command_mode = false;
    }
    return result;
}

static const CLI_Command_Definition_t program = {
    "program",
    "program \r\n Program application\r\n\r\n",
    cmd_program,
    0
};


static BaseType_t cmd_status(char *pcWriteBuffer, size_t xWriteBufferLen,
                             const char *pcCommandString)
{
    strncpy(pcWriteBuffer, program_status, xWriteBufferLen);
    return pdFALSE;
}

static const CLI_Command_Definition_t status = {
    "status",
    "status \r\n Program status\r\n\r\n",
    cmd_status,
    0
};

void cli_init(void)
{
#if defined(LED_PORT) && defined(LED_PIN) && defined(LED_CLI_CTL)
    FreeRTOS_CLIRegisterCommand(&led_ctl);
#endif
    FreeRTOS_CLIRegisterCommand(&info);
    FreeRTOS_CLIRegisterCommand(&erase);
    FreeRTOS_CLIRegisterCommand(&program);
    FreeRTOS_CLIRegisterCommand(&status);
}

uint8_t cli_process(const char *input, char *output, uint32_t output_size)
{
    uint8_t result = 0;
    if (command_mode) {
        result = FreeRTOS_CLIProcessCommand(input, output, output_size);
    } else {
        if (stm_hex_feed(input)) {
            if (!strncmp(input, ":00000001FF", 11)) {
                uint32_t len = strlen(program_status);
                strncpy(&program_status[len], "Program OK!\r\n\r\n", sizeof(program_status) - len);
                FLASH_Lock();
                command_mode = true;
            }
        } else {
            uint32_t len = strlen(program_status);
            strncpy(&program_status[len], "Program Error at:\r\n", sizeof(program_status) - len);
            len = strlen(program_status);
            strncpy(&program_status[len], input, sizeof(program_status) - len);
            len = strlen(program_status);
            strncpy(&program_status[len], "\r\n\r\n", sizeof(program_status) - len);
            FLASH_Lock();
            command_mode = true;
        }
        output[0] = '\0';
    }
    return result;
}

uint32_t app_adderess(void)
{
    return bl_data_get_app_first_addr();
}
