#ifndef USBDEFS_H_INCLUDED
#define USBDEFS_H_INCLUDED

#include "stm32f10x_conf.h"

#define USB_MAX_PACKET0 64
#define USB_EP_COUNT    2       // Two total endpoints defined for this application
#define EP_ADDR_MASK    0xFFFE      /* Address Mask */
#define EP_COUNT_MASK   0x03FF      /* Count Mask */


typedef struct _EP_BUF_DSCR {
    uint32_t ADDR_TX;
    uint32_t COUNT_TX;
    uint32_t ADDR_RX;
    uint32_t COUNT_RX;
} EP_BUF_DSCR;

#define EP_BUF_ADDR     (sizeof(EP_BUF_DSCR)*USB_EP_COUNT)

typedef union {
    uint16_t u16;
    struct {
        uint8_t low;
        uint8_t high;
    } __attribute__((packed)) bytes;
} __attribute__((packed)) uint16_8_t;


// bmRequestType.bits.direction
#define REQUEST_HOST_TO_DEVICE     0
#define REQUEST_DEVICE_TO_HOST     1

typedef enum {
    HOST_TO_DEVICE = 0,
    DEVICE_TO_HOST = 1
} REQUEST_DIRECTION;

// bmRequestType.bits.type
#define REQUEST_STANDARD           0
#define REQUEST_CLASS              1
#define REQUEST_VENDOR             2
#define REQUEST_RESERVED           3

typedef enum {
    STANDARD = 0,
    CLASS    = 1,
    VENDOR   = 2,
    RESERVED = 3
} REQUEST_TYPE_ENUM;

// bmRequestType.bits.recipient
#define REQUEST_TO_DEVICE          0
#define REQUEST_TO_INTERFACE       1
#define REQUEST_TO_ENDPOINT        2
#define REQUEST_TO_OTHER           3

typedef enum {
    TO_DEVICE    = 0,
    TO_INTERFACE = 1,
    TO_ENDPOINT  = 2,
    TO_OTHER     = 3
} REQUEST_RECIPIENT;

/* bmRequestType Definition */
typedef union _REQUEST_TYPE {
    struct _bitmap {
        uint8_t recipient : 5;
        uint8_t type      : 2;
        uint8_t direction : 1;
    } __attribute__((packed)) bits;
    uint8_t byte;
} __attribute__((packed)) REQUEST_TYPE;

/* USB Standard Request Codes */
#define USB_REQUEST_GET_STATUS                 0
#define USB_REQUEST_CLEAR_FEATURE              1
#define USB_REQUEST_SET_FEATURE                3
#define USB_REQUEST_SET_ADDRESS                5
#define USB_REQUEST_GET_DESCRIPTOR             6
#define USB_REQUEST_SET_DESCRIPTOR             7
#define USB_REQUEST_GET_CONFIGURATION          8
#define USB_REQUEST_SET_CONFIGURATION          9
#define USB_REQUEST_GET_INTERFACE              10
#define USB_REQUEST_SET_INTERFACE              11
#define USB_REQUEST_SYNC_FRAME                 12

typedef enum {
    GET_STATUS        = 0,
    CLEAR_FEATURE     = 1,
    SET_FEATURE       = 3,
    SET_ADDRESS       = 5,
    GET_DESCRIPTOR    = 6,
    SET_DESCRIPTOR    = 7,
    GET_CONFIGURATION = 8,
    SET_CONFIGURATION = 9,
    GET_INTERFACE     = 10,
    SET_INTERFACE     = 11,
    SYNC_FRAME        = 12
} USB_REQUEST;

/* USB GET_STATUS Bit Values */
#define USB_GETSTATUS_SELF_POWERED             0x01
#define USB_GETSTATUS_REMOTE_WAKEUP            0x02
#define USB_GETSTATUS_ENDPOINT_STALL           0x01

/* USB Standard Feature selectors */
#define USB_FEATURE_ENDPOINT_STALL             0
#define USB_FEATURE_REMOTE_WAKEUP              1

/* USB Default Control Pipe Setup Packet */
typedef struct _USB_SETUP_PACKET {
    REQUEST_TYPE bmRequestType;
    uint8_t      bRequest;
    uint16_8_t   wValue;
    uint16_8_t   wIndex;
    uint16_t     wLength;
} __attribute__((packed)) USB_SETUP_PACKET;


/* USB Descriptor Types */
#define USB_DEVICE_DESCRIPTOR_TYPE             1
#define USB_CONFIGURATION_DESCRIPTOR_TYPE      2
#define USB_STRING_DESCRIPTOR_TYPE             3
#define USB_INTERFACE_DESCRIPTOR_TYPE          4
#define USB_ENDPOINT_DESCRIPTOR_TYPE           5
#define USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE   6
#define USB_OTHER_SPEED_CONFIG_DESCRIPTOR_TYPE 7
#define USB_INTERFACE_POWER_DESCRIPTOR_TYPE    8
#define USB_HID_REPORT_DESCRIPTOR_TYPE         0x22


/* USB Device Classes */
#define USB_DEVICE_CLASS_RESERVED              0x00
#define USB_DEVICE_CLASS_AUDIO                 0x01
#define USB_DEVICE_CLASS_COMMUNICATIONS        0x02
#define USB_DEVICE_CLASS_HUMAN_INTERFACE       0x03
#define USB_DEVICE_CLASS_MONITOR               0x04
#define USB_DEVICE_CLASS_PHYSICAL_INTERFACE    0x05
#define USB_DEVICE_CLASS_POWER                 0x06
#define USB_DEVICE_CLASS_PRINTER               0x07
#define USB_DEVICE_CLASS_STORAGE               0x08
#define USB_DEVICE_CLASS_HUB                   0x09
#define USB_DEVICE_CLASS_VENDOR_SPECIFIC       0xFF

/* bmAttributes in Configuration Descriptor */
#define USB_CONFIG_POWERED_MASK                0xC0
#define USB_CONFIG_BUS_POWERED                 0x80
#define USB_CONFIG_SELF_POWERED                0xC0
#define USB_CONFIG_REMOTE_WAKEUP               0x20

/* bMaxPower in Configuration Descriptor */
#define USB_CONFIG_POWER_MA(mA)                ((mA)/2)

/* bEndpointAddress in Endpoint Descriptor */
#define USB_ENDPOINT_DIRECTION_MASK            0x80
#define USB_ENDPOINT_OUT(addr)                 ((addr) | 0x00)
#define USB_ENDPOINT_IN(addr)                  ((addr) | 0x80)

/* bmAttributes in Endpoint Descriptor */
#define USB_ENDPOINT_TYPE_MASK                 0x03
#define USB_ENDPOINT_TYPE_CONTROL              0x00
#define USB_ENDPOINT_TYPE_ISOCHRONOUS          0x01
#define USB_ENDPOINT_TYPE_BULK                 0x02
#define USB_ENDPOINT_TYPE_INTERRUPT            0x03
#define USB_ENDPOINT_SYNC_MASK                 0x0C
#define USB_ENDPOINT_SYNC_NO_SYNCHRONIZATION   0x00
#define USB_ENDPOINT_SYNC_ASYNCHRONOUS         0x04
#define USB_ENDPOINT_SYNC_ADAPTIVE             0x08
#define USB_ENDPOINT_SYNC_SYNCHRONOUS          0x0C
#define USB_ENDPOINT_USAGE_MASK                0x30
#define USB_ENDPOINT_USAGE_DATA                0x00
#define USB_ENDPOINT_USAGE_FEEDBACK            0x10
#define USB_ENDPOINT_USAGE_IMPLICIT_FEEDBACK   0x20
#define USB_ENDPOINT_USAGE_RESERVED            0x30

/* USB Standard Device Descriptor */
typedef struct _USB_DEVICE_DESCRIPTOR {
    uint8_t   bLength;
    uint8_t   bDescriptorType;
    uint16_t  bcdUSB;
    uint8_t   bDeviceClass;
    uint8_t   bDeviceSubClass;
    uint8_t   bDeviceProtocol;
    uint8_t   bMaxPacketSize0;
    uint16_t  idVendor;
    uint16_t  idProduct;
    uint16_t  bcdDevice;
    uint8_t  iManufacturer;
    uint8_t  iProduct;
    uint8_t  iSerialNumber;
    uint8_t  bNumConfigurations;
} __attribute__((packed)) USB_DEVICE_DESCRIPTOR;

/* USB 2.0 Device Qualifier Descriptor */
typedef struct _USB_DEVICE_QUALIFIER_DESCRIPTOR {
    uint8_t   bLength;
    uint8_t   bDescriptorType;
    uint16_t  bcdUSB;
    uint8_t   bDeviceClass;
    uint8_t   bDeviceSubClass;
    uint8_t   bDeviceProtocol;
    uint8_t   bMaxPacketSize0;
    uint8_t   bNumConfigurations;
    uint8_t   bReserved;
} __attribute__((packed)) USB_DEVICE_QUALIFIER_DESCRIPTOR;

/* USB Standard Configuration Descriptor */
typedef struct _USB_CONFIGURATION_DESCRIPTOR {
    uint8_t   bLength;
    uint8_t   bDescriptorType;
    uint16_t  wTotalLength;
    uint8_t   bNumInterfaces;
    uint8_t   bConfigurationValue;
    uint8_t   iConfiguration;
    uint8_t   bmAttributes;
    uint8_t   bMaxPower;
} __attribute__((packed)) USB_CONFIGURATION_DESCRIPTOR;

/* USB Standard Interface Descriptor */
typedef struct _USB_INTERFACE_DESCRIPTOR {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bInterfaceNumber;
    uint8_t  bAlternateSetting;
    uint8_t  bNumEndpoints;
    uint8_t  bInterfaceClass;
    uint8_t  bInterfaceSubClass;
    uint8_t  bInterfaceProtocol;
    uint8_t  iInterface;
} __attribute__((packed)) USB_INTERFACE_DESCRIPTOR;

/* USB Standard Endpoint Descriptor */
typedef struct _USB_ENDPOINT_DESCRIPTOR {
    uint8_t   bLength;
    uint8_t   bDescriptorType;
    uint8_t   bEndpointAddress;
    uint8_t   bmAttributes;
    uint16_t  wMaxPacketSize;
    uint8_t   bInterval;
} __attribute__((packed)) USB_ENDPOINT_DESCRIPTOR;

typedef struct _USB_HID_DESCRIPTOR {
    uint8_t   bLength;
    uint8_t   bDescriptorType;
    uint16_t  bcdHID;
    uint8_t   bCountryCode;
    uint8_t   bNumDescriptors;
    uint8_t   bClassDescriptorType;
    uint16_t  wDescriptorLength;
} __attribute__((packed)) USB_HID_DESCRIPTOR;

typedef struct _USB_QUALIFIER_DESCRIPTOR {
    uint8_t     bLength;            // 0x0A
    uint8_t     bDescriptorType;    // 0x06
    uint16_t    bcdUSB;             // 0x0200
    uint8_t     bDeviceClass;       // 0
    uint8_t     bDeviceSubClass;    // 0
    uint8_t     bDeviceProtocol;    // 0
    uint8_t     bMaxPacketSize0;    // 0x40
    uint8_t     bNumConfigurations; // 0 ??
    uint8_t     bReserved;
} __attribute((packed)) USB_QUALIFIER_DESCRIPTOR;



typedef struct _USB_CONFIG_DESCRIPTOR_FULL {
    USB_CONFIGURATION_DESCRIPTOR cd;
    USB_INTERFACE_DESCRIPTOR id;
    USB_HID_DESCRIPTOR hd;
    USB_ENDPOINT_DESCRIPTOR ed_in;
    USB_ENDPOINT_DESCRIPTOR ed_out;
} __attribute__((packed)) USB_CONFIG_DESCRIPTOR_FULL;



/* USB String Descriptor */
typedef struct _USB_STRING_DESCRIPTOR {
    uint8_t   bLength;
    uint8_t   bDescriptorType;
    uint16_t  bString[];
} __attribute__((packed)) USB_STRING_DESCRIPTOR;




/* USB Common Descriptor */
typedef struct _USB_COMMON_DESCRIPTOR {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
} __attribute__((packed)) USB_COMMON_DESCRIPTOR;



#endif /* USBDEFS_H_INCLUDED */
