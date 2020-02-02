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


#define LED_PIN     GPIO_Pin_13
#define LED_PORT    GPIOC


#ifdef DEBUG
#include <stdio.h>

extern void initialise_monitor_handles(void);

#define DBG_INIT() initialise_monitor_handles()
#define DBG_OUT(fmt, ...)   \
                printf("%s:%04u " fmt "\n", __FILE__, (unsigned)__LINE__, ##__VA_ARGS__)
#else
#define DBG_INIT()
#define DBG_OUT(fmt, ...)
#endif


static void sleep_ms(unsigned int ms) {
    while(ms--) {
        volatile unsigned int tmp = 5971u;
        while(tmp--) {
            __asm("nop");
        }
    }
}


int main(void) {

    DBG_INIT();
    DBG_OUT("main() start");
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO  |   \
                            RCC_APB2Periph_GPIOA |   \
                            RCC_APB2Periph_GPIOB |   \
                            RCC_APB2Periph_GPIOC |   \
                            RCC_APB2Periph_GPIOD |   \
                            RCC_APB2Periph_GPIOE |   \
                            RCC_APB2Periph_GPIOF |   \
                            RCC_APB2Periph_GPIOG, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct_LED;
    GPIO_InitStruct_LED.GPIO_Pin = LED_PIN;
    GPIO_InitStruct_LED.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct_LED.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(LED_PORT, &GPIO_InitStruct_LED);

    for(;;) {
        GPIO_ResetBits(LED_PORT, LED_PIN);
        sleep_ms(500);
        GPIO_SetBits(LED_PORT, LED_PIN);
        sleep_ms(500);
    }

    GPIO_InitStruct_LED.GPIO_Pin = LED_PIN;
    GPIO_InitStruct_LED.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStruct_LED.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(LED_PORT, &GPIO_InitStruct_LED);

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
