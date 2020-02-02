/*
********************************************************************************
**
**  (C) 2020 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
**
**  This code is licensed under the MIT.
**
********************************************************************************
*/

#ifndef __USB_CONF_H
#define __USB_CONF_H


/* defines how many endpoints are used by the device */
#define USB_CDC_RX_TX_PACKET    (64)
/******************************************************************************/


/* defines how many endpoints are used by the device */
#define USB_CDC_EP_CNT      (4)         /* Total CDC endpoints include 0. */
#define USB_CDC_EP0_PACK    (64)
/* For CDC EP1: MCU TX */
#define USB_CDC_EP1_PACK    (USB_CDC_RX_TX_PACKET)
/* For CDC EP2: MCU serial config */
#define USB_CDC_EP2_PACK    (8)
/* For CDC EP3: MCU RX */
#define USB_CDC_EP3_PACK    (USB_CDC_RX_TX_PACKET)
/* For CDC maximum packet */
#define USB_CDC_MAX_PACK   (USB_CDC_EP0_PACK > USB_CDC_EP1_PACK ?   \
                            USB_CDC_EP0_PACK > USB_CDC_EP2_PACK ?   \
                            USB_CDC_EP0_PACK > USB_CDC_EP3_PACK ?   \
                            USB_CDC_EP0_PACK : USB_CDC_EP3_PACK :   \
                            USB_CDC_EP2_PACK > USB_CDC_EP3_PACK ?   \
                            USB_CDC_EP2_PACK : USB_CDC_EP3_PACK :   \
                            USB_CDC_EP1_PACK > USB_CDC_EP2_PACK ?   \
                            USB_CDC_EP1_PACK > USB_CDC_EP3_PACK ?   \
                            USB_CDC_EP1_PACK : USB_CDC_EP3_PACK :   \
                            USB_CDC_EP2_PACK > USB_CDC_EP3_PACK ?   \
                            USB_CDC_EP2_PACK : USB_CDC_EP3_PACK)


/* mask defining which events has to be handled */
#define IMR_MSK             (CNTR_CTRM | CNTR_RESETM)

/* calculate endpoint address */
#define BTABLE_ADDRESS      (0x00)  /* align to 8 */
#define __ALIGN_8(x)        ((((x) + 7) >> 3) << 3)

#define ENDP0_RXADDR        __ALIGN_8(BTABLE_ADDRESS + (USB_CDC_EP_CNT << 3))
#define ENDP0_TXADDR        __ALIGN_8(ENDP0_RXADDR + USB_CDC_EP0_PACK)
#define ENDP1_TXADDR        __ALIGN_8(ENDP0_TXADDR + USB_CDC_EP0_PACK)
#define ENDP2_TXADDR        __ALIGN_8(ENDP1_TXADDR + USB_CDC_EP1_PACK)
#define ENDP3_RXADDR        __ALIGN_8(ENDP2_TXADDR + USB_CDC_EP2_PACK)


#endif
