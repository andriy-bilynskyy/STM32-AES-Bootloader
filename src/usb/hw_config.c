/*
********************************************************************************
**
**  (C) 2020 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
**
**  This code is licensed under the MIT.
**
********************************************************************************
*/

#include "usb_lib.h"
#include <string.h>


__IO bool       usb_cdc_ready                       = FALSE;
__IO uint16_t   usb_cdc_rx_cnt                      = 0;
__IO uint16_t   usb_cdc_rx_idx                      = 0;
__IO uint8_t    usb_cdc_rx_data[USB_CDC_EP3_PACK];
__IO bool       usb_cdc_tx_done                     = TRUE;


void usb_cdc_start(void) {
    usb_cdc_ready   = FALSE;
    usb_cdc_rx_cnt  = 0;
    usb_cdc_rx_idx  = 0;
    usb_cdc_tx_done = TRUE;

    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USB_Init();
}


void usb_cdc_stop(void) {
    usb_cdc_ready = FALSE;

    /* disable all interrupts and force USB reset */
    _SetCNTR(CNTR_FRES);
    /* clear interrupt status register */
    _SetISTR(0);
    /* switch-off device */
    _SetCNTR(CNTR_FRES + CNTR_PDWN);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, DISABLE);

    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}


uint8_t usb_cdc_is_ready(void) {
    return (uint8_t)usb_cdc_ready;
}


uint8_t usb_cdc_rx_avail(void) {
    uint8_t result = FALSE;

    if(usb_cdc_rx_cnt > usb_cdc_rx_idx) {
        result = TRUE;
    }

    return result;
}


int usb_cdc_read(uint8_t data[], uint16_t len) {
    int result = -1;
    if(usb_cdc_ready == TRUE && data) {
        if(len) {
            while(usb_cdc_rx_cnt <= usb_cdc_rx_idx);
            result = len < usb_cdc_rx_cnt - usb_cdc_rx_idx ?
                     len : usb_cdc_rx_cnt - usb_cdc_rx_idx;
            memcpy(data, (const uint8_t *)&usb_cdc_rx_data[usb_cdc_rx_idx],
                   (uint16_t)result);
            usb_cdc_rx_idx += (uint16_t)result;
            if(usb_cdc_rx_cnt <= usb_cdc_rx_idx) {
                SetEPRxValid(ENDP3);
            }
        } else {
            result = 0;
        }
    }
    return result;
}


int usb_cdc_write(const uint8_t data[], uint16_t len) {
    int result = -1;
    if(usb_cdc_ready == TRUE && data) {
        result = len < USB_CDC_EP1_PACK ? len : USB_CDC_EP1_PACK;
        if(result > 0) {
            while(usb_cdc_tx_done == FALSE);
            usb_cdc_tx_done = FALSE;
            UserToPMABufferCopy((uint8_t *)data, ENDP1_TXADDR, (uint16_t)result);
            SetEPTxCount(ENDP1, (uint16_t)result);
            SetEPTxValid(ENDP1);
        }
    }
    return result;
}


char usb_cdc_get_char(void) {
    char ch = 0;
    if(usb_cdc_ready == TRUE) {
        while(usb_cdc_rx_cnt <= usb_cdc_rx_idx);
        ch = usb_cdc_rx_data[usb_cdc_rx_idx];
        usb_cdc_rx_idx ++;
        if(usb_cdc_rx_cnt <= usb_cdc_rx_idx) {
            SetEPRxValid(ENDP3);
        }
    }
    return ch;
}


void usb_cdc_put_char(char ch) {
    if(usb_cdc_ready == TRUE) {
        while(usb_cdc_tx_done == FALSE);
        usb_cdc_tx_done = FALSE;
        UserToPMABufferCopy((uint8_t *)&ch, ENDP1_TXADDR, sizeof(ch));
        SetEPTxCount(ENDP1, sizeof(ch));
        SetEPTxValid(ENDP1);
    }
}


void usb_cdc_put_string(const char * str) {
    uint16_t len = strlen(str);

    while(len) {
        int res = usb_cdc_write((const uint8_t *)str, len);
        if(res > 0) {
            str += res;
            len -= res;
        } else {
            break;
        }
    }
}


/*******************************************************************************
 * USB interrupt
 ******************************************************************************/

/* ISTR register last read value */
__IO uint16_t wIstr;


/* function pointers to non-control endpoints service routines */
static void usb_cdc_on_tx_finish(void);
static void usb_cdc_on_rx_data(void);

void (*pEpInt_IN[7])(void) = {
    usb_cdc_on_tx_finish,
    NOP_Process,
    NOP_Process,
    NOP_Process,
    NOP_Process,
    NOP_Process,
    NOP_Process,
};

void (*pEpInt_OUT[7])(void) = {
    NOP_Process,
    NOP_Process,
    usb_cdc_on_rx_data,
    NOP_Process,
    NOP_Process,
    NOP_Process,
    NOP_Process,
};


void USB_LP_CAN1_RX0_IRQHandler(void) {
    wIstr = _GetISTR();

    if (wIstr & ISTR_CTR & wInterrupt_Mask) {
        /* servicing of the endpoint correct transfer interrupt */
        /* clear of the CTR flag into the sub */
        CTR_LP();
    }
    if (wIstr & ISTR_RESET & wInterrupt_Mask) {
        _SetISTR((uint16_t)CLR_RESET);
        Device_Property.Reset();
    }
}


static void usb_cdc_on_tx_finish(void) {
    usb_cdc_tx_done = TRUE;
}


static void usb_cdc_on_rx_data(void) {
    usb_cdc_rx_cnt = GetEPRxCount(ENDP3);
    usb_cdc_rx_idx = 0;
    if(usb_cdc_rx_cnt) {
        PMAToUserBufferCopy((uint8_t *)usb_cdc_rx_data, ENDP3_RXADDR,
                            usb_cdc_rx_cnt);
    } else {
        SetEPRxValid(ENDP3);
    }
}
