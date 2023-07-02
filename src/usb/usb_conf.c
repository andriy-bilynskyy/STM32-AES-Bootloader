/*
********************************************************************************
**
**  (C) 2023 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
**
**  This code is licensed under the MIT.
**
********************************************************************************
*/

#include <usb_lib.h>


/*******************************************************************************
 * USB descriptors
 ******************************************************************************/
static const uint8_t usb_cdc_device_descriptor[] = {
    0x12,   /* bLength 0x12*/
    0x01,   /* bDescriptorType */
    0x00,
    0x02,   /* bcdUSB = 2.00 */
    0x00,   /* bDeviceClass: Described in interface */
    0x00,   /* bDeviceSubClass */
    0x00,   /* bDeviceProtocol */
    USB_CDC_EP0_PACK,   /* bMaxPacketSize0 */
    0xCD,
    0xAB,   /* idVendor = 0xABCD */
    0xCD,
    0xAB,   /* idProduct = 0xABCD */
    0x00,
    0x02,   /* bcdDevice = 2.00 */
    1,      /* Index of string descriptor describing manufacturer */
    2,      /* Index of string descriptor describing product */
    0,      /* Index of string descriptor describing the device's SN */
    0x01    /* bNumConfigurations */
};

static const uint8_t usb_cdc_config_descriptor[] = {
    /* Configuration Descriptor */
    0x09,   /* bLength: Configuration Descriptor size */
    0x02,   /* bDescriptorType: Configuration */
    0x2C,   /* wTotalLength: Number of returned bytes */
    0x00,
    0x01,   /* bNumInterfaces: One interface */
    0x01,   /* bConfigurationValue: One Configuration */
    0x00,   /* iConfiguration: Index of string descriptor describing config   */
    0xC0,   /* bmAttributes: self powered */
    0x00,   /* MaxPower 0 mA */

    /* Interface Descriptor */
    0x09,   /* bLength: Interface Descriptor size */
    0x04,   /* bDescriptorType: Interface */
    0x00,   /* bInterfaceNumber: First Interface */
    0x00,   /* bAlternateSetting: None */
    0x03,   /* bNumEndpoints: Three endpoints used */
    0x02,   /* bInterfaceClass: Communication Interface Class */
    0x02,   /* bInterfaceSubClass: Abstract Control Model */
    0x00,   /* bInterfaceProtocol: None */
    0x00,   /* iInterface: */

    /* Union Functional Descriptor */
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x06,   /* bDescriptorSubtype: Union func desc */
    0x00,   /* bMasterInterface: Communication class interface */
    0x00,   /* bSlaveInterface0: Data Class Interface */

    /* Endpoint 1 Descriptor MCU TX */
    0x07,   /* bLength: Endpoint Descriptor size */
    0x05,   /* bDescriptorType: Endpoint */
    0x81,   /* bEndpointAddress: (IN1) */
    0x02,   /* bmAttributes: Bulk */
    USB_CDC_EP1_PACK,   /* wMaxPacketSize: */
    0x00,
    0x00,    /* bInterval */

    /* Endpoint 2 Descriptor MCU serial config */
    0x07,   /* bLength: Endpoint Descriptor size */
    0x05,   /* bDescriptorType: Endpoint */
    0x82,   /* bEndpointAddress: (IN2) */
    0x03,   /* bmAttributes: Interrupt */
    USB_CDC_EP2_PACK,      /* wMaxPacketSize: */
    0x00,
    0xFF,   /* bInterval: */

    /* Endpoint 3 Descriptor MCU RX */
    0x07,   /* bLength: Endpoint Descriptor size */
    0x05,   /* bDescriptorType: Endpoint */
    0x03,   /* bEndpointAddress: (OUT3) */
    0x02,   /* bmAttributes: Bulk */
    USB_CDC_EP3_PACK,   /* wMaxPacketSize: */
    0x00,
    0x00    /* bInterval: ignore for Bulk transfer */
};

static const uint8_t usb_cdc_lang_id_descriptor[] = {
    0x04,   /* bLength: LangID Descriptor size */
    0x03,   /* bDescriptorType: String */
    0x09,
    0x04    /* LangID: 0x0409 - U.S. English */
};

static const uint8_t usb_cdc_vendor_str_descriptor[] = {
    0x16,     /* bLength: Vendor string Descriptor size */
    0x03,     /* bDescriptorType: String */
    /* Manufacturer: "EmSoftware" */
    'E', 0, 'm', 0, 'S', 0, 'o', 0, 'f', 0, 't', 0, 'w', 0, 'a', 0, 'r', 0, 'e', 0
};

static const uint8_t usb_cdc_product_str_descriptor[] = {
    0x22,     /* bLength: Product string Descriptor size */
    0x03,     /* bDescriptorType: String */
    /* Product name: "Virtual COM Port" */
    'V', 0, 'i', 0, 'r', 0, 't', 0, 'u', 0, 'a', 0, 'l', 0, ' ', 0, 'C', 0, 'O', 0, 'M', 0, ' ', 0,
    'P', 0, 'o', 0, 'r', 0, 't', 0
};


/*******************************************************************************
 * Driver USB structures
 ******************************************************************************/
DEVICE Device_Table = {
    USB_CDC_EP_CNT,  /* Four endpoints */
    1   /* One configuration */
};

ONE_DESCRIPTOR Device_Descriptor = {
    (uint8_t *)usb_cdc_device_descriptor,
    sizeof(usb_cdc_device_descriptor)
};

ONE_DESCRIPTOR Config_Descriptor = {
    (uint8_t *)usb_cdc_config_descriptor,
    sizeof(usb_cdc_config_descriptor)
};

ONE_DESCRIPTOR String_Descriptor[] = {
    {
        (uint8_t *)usb_cdc_lang_id_descriptor,
        sizeof(usb_cdc_lang_id_descriptor)
    },
    {
        (uint8_t *)usb_cdc_vendor_str_descriptor,
        sizeof(usb_cdc_vendor_str_descriptor)
    },
    {
        (uint8_t *)usb_cdc_product_str_descriptor,
        sizeof(usb_cdc_product_str_descriptor)
    }
};


static void usb_cdc_init(void);
static void usb_cdc_reset(void);
static void usb_cdc_in(void);
static void usb_cdc_out(void);
static RESULT usb_cdc_data_setup(uint8_t RequestNo);
static RESULT usb_cdc_no_data_setp(uint8_t RequestNo);
static RESULT usb_cdc_get_interface_settings(uint8_t Interface,
        uint8_t AlternateSetting);
static uint8_t *usb_cdc_get_device_descriptor(uint16_t Length);
static uint8_t *usb_cdc_get_config_descriptor(uint16_t Length);
static uint8_t *usb_cdc_get_str_descriptor(uint16_t Length);

static void usb_cdc_set_configuration(void);


DEVICE_PROP Device_Property = {
    usb_cdc_init,
    usb_cdc_reset,
    usb_cdc_in,
    usb_cdc_out,
    usb_cdc_data_setup,
    usb_cdc_no_data_setp,
    usb_cdc_get_interface_settings,
    usb_cdc_get_device_descriptor,
    usb_cdc_get_config_descriptor,
    usb_cdc_get_str_descriptor,
    0,
    USB_CDC_MAX_PACK
};

USER_STANDARD_REQUESTS User_Standard_Requests = {
    NOP_Process,
    usb_cdc_set_configuration,
    NOP_Process,
    NOP_Process,
    NOP_Process,
    NOP_Process,
    NOP_Process,
    NOP_Process,
    NOP_Process
};


extern bool usb_cdc_ready;


static void usb_cdc_init(void)
{
    usb_cdc_ready = FALSE;
    /* Set Virtual_Com_Port DEVICE as not configured */
    pInformation->Current_Configuration = 0;
    /* Connect the device */
    _SetCNTR(CNTR_FRES);
    wInterrupt_Mask = 0;
    _SetCNTR(wInterrupt_Mask);
    SetISTR(0);
    /* Perform basic device initialization operations */
    USB_SIL_Init();
}

static void usb_cdc_reset(void)
{
    usb_cdc_ready = FALSE;
    /* Set Virtual_Com_Port DEVICE as not configured */
    pInformation->Current_Configuration = 0;
    /* Current Feature initialization */
    pInformation->Current_Feature = usb_cdc_config_descriptor[7];
    /* Set Virtual_Com_Port DEVICE with the default Interface*/
    pInformation->Current_Interface = 0;
    SetBTABLE(BTABLE_ADDRESS);
    /* Initialize Endpoint 0 */
    SetEPType(ENDP0, EP_CONTROL);
    SetEPTxStatus(ENDP0, EP_TX_STALL);
    SetEPRxAddr(ENDP0, ENDP0_RXADDR);
    SetEPTxAddr(ENDP0, ENDP0_TXADDR);
    Clear_Status_Out(ENDP0);
    SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
    SetEPRxValid(ENDP0);
    /* Initialize Endpoint 1 */
    SetEPType(ENDP1, EP_BULK);
    SetEPTxAddr(ENDP1, ENDP1_TXADDR);
    SetEPTxStatus(ENDP1, EP_TX_NAK);
    SetEPRxStatus(ENDP1, EP_RX_DIS);
    /* Initialize Endpoint 2 */
    SetEPType(ENDP2, EP_INTERRUPT);
    SetEPTxAddr(ENDP2, ENDP2_TXADDR);
    SetEPRxStatus(ENDP2, EP_RX_DIS);
    SetEPTxStatus(ENDP2, EP_TX_NAK);
    /* Initialize Endpoint 3 */
    SetEPType(ENDP3, EP_BULK);
    SetEPRxAddr(ENDP3, ENDP3_RXADDR);
    SetEPRxCount(ENDP3, USB_CDC_EP3_PACK);
    SetEPRxStatus(ENDP3, EP_RX_VALID);
    SetEPTxStatus(ENDP3, EP_TX_DIS);
    /* Set this device to response on default address */
    SetDeviceAddress(0);
}

static void usb_cdc_in(void) {}

static void usb_cdc_out(void) {}

static RESULT usb_cdc_data_setup(uint8_t RequestNo)
{
    return USB_UNSUPPORT;
}

static RESULT usb_cdc_no_data_setp(uint8_t RequestNo)
{
    return USB_UNSUPPORT;
}

static RESULT usb_cdc_get_interface_settings(uint8_t Interface,
        uint8_t AlternateSetting)
{
    RESULT result = USB_UNSUPPORT;
    if (!Interface && !AlternateSetting) {
        result = USB_SUCCESS;
    }
    return result;
}

static uint8_t *usb_cdc_get_device_descriptor(uint16_t Length)
{
    return Standard_GetDescriptorData(Length, &Device_Descriptor);
}

static uint8_t *usb_cdc_get_config_descriptor(uint16_t Length)
{
    return Standard_GetDescriptorData(Length, &Config_Descriptor);
}

static uint8_t *usb_cdc_get_str_descriptor(uint16_t Length)
{
    uint8_t *result = NULL;
    uint8_t w_value = pInformation->USBwValue0;
    if (w_value <= 2) {
        result = Standard_GetDescriptorData(Length,
                                            &String_Descriptor[w_value]);
    }
    return result;
}

static void usb_cdc_set_configuration(void)
{
    DEVICE_INFO *pInfo = &Device_Info;
    if (pInfo->Current_Configuration) {
        usb_cdc_ready = TRUE;
    }
}
