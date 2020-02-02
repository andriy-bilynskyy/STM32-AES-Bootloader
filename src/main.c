/*
********************************************************************************
**
**  (C) 2020 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
**
**  This code is licensed under the MIT.
**
********************************************************************************
*/

#include "stm32f10x_conf.h"
#include "bl_conf.h"
#include "usb_lib.h"
#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "cli.h"


__attribute__((naked)) void jump_2_app(uint32_t addr) {
    __disable_irq();
    SCB->VTOR = addr;
    __asm volatile (
        "ldr sp, [r0]           \n"
        "ldr pc, [r0, #4]       \n"
    );
}


int main(void) {
    DBG_INIT();
    DBG_OUT("main() start");

    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    FLASH_SetLatency(FLASH_Latency_2);
#ifndef DEBUG
    if(FLASH_GetReadOutProtectionStatus() == RESET) {
        FLASH_Unlock();
        FLASH_ReadOutProtection(ENABLE);
        FLASH_Lock();
    }
#endif

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO  |   \
                            RCC_APB2Periph_GPIOA |   \
                            RCC_APB2Periph_GPIOB |   \
                            RCC_APB2Periph_GPIOC |   \
                            RCC_APB2Periph_GPIOD |   \
                            RCC_APB2Periph_GPIOE |   \
                            RCC_APB2Periph_GPIOF |   \
                            RCC_APB2Periph_GPIOG, ENABLE);
    GPIO_InitTypeDef GPIO_InitStruct_JMP;
    GPIO_InitStruct_JMP.GPIO_Pin = BL_PIN;
    GPIO_InitStruct_JMP.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStruct_JMP.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(BL_PORT, &GPIO_InitStruct_JMP);
    if(GPIO_ReadInputDataBit(BL_PORT, BL_PIN) == Bit_RESET) {
        GPIO_InitStruct_JMP.GPIO_Mode = GPIO_Mode_AIN;
        GPIO_Init(BL_PORT, &GPIO_InitStruct_JMP);
        RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO  |   \
                                RCC_APB2Periph_GPIOA |   \
                                RCC_APB2Periph_GPIOB |   \
                                RCC_APB2Periph_GPIOC |   \
                                RCC_APB2Periph_GPIOD |   \
                                RCC_APB2Periph_GPIOE |   \
                                RCC_APB2Periph_GPIOF |   \
                                RCC_APB2Periph_GPIOG, DISABLE);
        jump_2_app(app_adderess());
    }

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    usb_cdc_start();
    LED_INIT();
    cli_init();

    char     command_buf[523];
    uint16_t command_buf_pos = 0;

    for(;;) {
        if(usb_cdc_is_ready()) {
            LED_STATE(1);

            char ch = usb_cdc_get_char();
            if(ch) {
                switch(ch) {
                case '\r':
                    command_buf[command_buf_pos ++] = '\0';
                    usb_cdc_put_string("\r\n");
                    uint8_t cmd = 1;
                    do {
                        cmd = cli_process(command_buf,
                                          FreeRTOS_CLIGetOutputBuffer(),
                                          configCOMMAND_INT_MAX_OUTPUT_SIZE);
                        usb_cdc_put_string(FreeRTOS_CLIGetOutputBuffer());
                    } while(cmd);
                    command_buf_pos = 0;
                    break;
                case '\n':
                    break;
                case '\b':
                    if(command_buf_pos) {
                        command_buf_pos --;
                        usb_cdc_put_char(ch);
                    }
                    break;
                default:
                    usb_cdc_put_char(ch);
                    command_buf[command_buf_pos ++] = ch;
                    break;
                }
            }
        } else {
            LED_STATE(0);
        }
    }

    LED_DEINIT();
    usb_cdc_stop();
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO  |   \
                            RCC_APB2Periph_GPIOA |   \
                            RCC_APB2Periph_GPIOB |   \
                            RCC_APB2Periph_GPIOC |   \
                            RCC_APB2Periph_GPIOD |   \
                            RCC_APB2Periph_GPIOE |   \
                            RCC_APB2Periph_GPIOF |   \
                            RCC_APB2Periph_GPIOG, DISABLE);
}


#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line) {
    DBG_OUT("assertion in %s:%04u", file, (unsigned int)line);
    for(;;);
}
#endif


__attribute__((naked)) void HardFault_Handler(void) {
    __asm volatile
    (
        " tst lr, #4                    \n"
        " ite eq                        \n"
        " mrseq r0, msp                 \n"
        " mrsne r0, psp                 \n"
        " bl prvGetRegistersFromStack   \n"
    );
    for(;;);
}


void prvGetRegistersFromStack(unsigned int * pStack) {
    DBG_OUT("hard fault\n"
            "R0  = %08x\n"
            "R1  = %08x\n"
            "R2  = %08x\n"
            "R3  = %08x\n"
            "R12 = %08x\n"
            "LR  = %08x\n"
            "PC  = %08x\n"
            "PSR = %08x\n",
            pStack[0], pStack[1], pStack[2], pStack[3],
            pStack[4], pStack[5], pStack[6], pStack[7]);
}


void SystemHseFailed(void) {
    DBG_OUT("HSE failed");
    for(;;);
}
