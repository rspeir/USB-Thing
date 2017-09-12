#include "usbdefs.h"

const USB_DEVICE_DESCRIPTOR dd =
{
    0x12,       // length = 18 bytes
    0x01,       // descriptor is a device descriptor
    0x0200,     //0x0200,     // supports USB 2.0
    0x00,       // don't define device class at this stage
    0x00,       // don't define device subclass at this stage
    0x00,       // don't define protocol at this stage
    0x40,       // max packet size 64 bytes
    0x1d50,     // vendor id
    0x6120,     // product id
    0x0001,     // device version 0.01
    0x01,       // manufacturer string descriptor at index 1
    0x02,       // product string descriptor at index 2
    0x03,       // serial number string at index 3
    0x01        // 1 configuration
};

const USB_QUALIFIER_DESCRIPTOR qd =
{
    0x0A,
    0x06,
    0x0200,
    0x00,
    0x00,
    0x00,
    0x40,
    0x00,
    0x00
};

const USB_CONFIGURATION_DESCRIPTOR cd =
{
    0x09,       // length of this descriptor = 9 bytes
    0x02,       // descriptor type = config descriptor
    0x0029,       // length of this descriptor and its subordinate descriptors = 29h (41d)
    0x01,       // num interfaces = 1
    0x01,       // this configuration is configuration 1
    0x00,       // we're not using a string descriptor for this configuration
    0x00,       // configuration is bus powered, does not support remote wakeup
    0x32        // max power = 100 mA
};

const USB_INTERFACE_DESCRIPTOR id =
{
    0x09,       // descriptor is 9 bytes long
    0x04,       // descriptor type is interface descriptor
    0x00,       // interface ID number is 0
    0x00,       // no alternate setting
    0x01,       // 1 endpoint in addition to endpoint 0
    0x03,       // interface class is HID
    0x00,       // interface subclass
    0x00,       // interface protocol
    0x00,       // no string descriptor for this interface
};

const USB_HID_DESCRIPTOR hd =
{
    0x09,       // length of this descriptor
    0x21,       // descriptor type = HID
    0x0110,     // supports HID revision 1.1
    0x00,       // country code
    0x01,       // number of HID class descriptors to follow
    0x22,       // HID will have a subordinate report descriptor
    0x002f      // and report descriptor size will be 47 bytes
};

const USB_ENDPOINT_DESCRIPTOR ed_in =
{
    0x07,       // descriptor is 7 bytes long
    0x05,       // descriptor type is endpoint descriptor
    0x81,       // endpoint address EP1 in
    0x03,       // transfer scheme = interrupt
    0x0040,     // max packet size = 64 bytes
    0xff        // max poll interval = 255 ms?
};

const USB_ENDPOINT_DESCRIPTOR ed_out =
{
    0x07,       // descriptor is 7 bytes long
    0x05,       // descriptor type is endpoint descriptor
    0x01,       // endpoint address is EP1 out
    0x03,       // transfer type = interrupt
    0x0040,       // max packet size = 64 bytes
    0xff        // max poll interval = 255 ms
};

const uint8_t HID_REPORT_DESCRIPTOR[47] =
{
    0x06,   0xA0,   0xFF,       // Usage page = vendor defined (0xFFA0)
    0x09,   0x01,               // Usage = vendor defined
    0xA1,   0x01,               // Collection (application)

    0x09,   0x03,               // Usage = vendor defined
    0x15,   0x00,               // Logical min = 0
    0x26,   0xFF,   0x00,       // Logical max = 255
    0x95,   0x02,               // Report count = 2
    0x75,   0x08,               // Report size = 8 bits
    0x81,   0x02,               // Input (Data, Variable, Absolute)

    0x09,   0x04,               // Usage = vendor defined
    0x15,   0x00,               // Logical min = 0
    0x26,   0xFF,   0x00,       // Logical max = 255
    0x75,   0x08,               // Report size = 8 bits
    0x95,   0x02,               // Report count = 2
    0x91,   0x02,               // Output (Data, Variable, Absolute)

    0x09,   0x05,               // Usage = vendor defined
    0x15,   0x00,               // Logical minimum = 0
    0x26,   0xff,   0x00,       // Logical max = 255
    0x75,   0x08,               // Report size = 8 bits
    0x95,   0x02,               // Report count = 2
    0xB1,   0x02,               // Feature (Data, Variable, Absolute)

    0xC0                        // End collection
};


const USB_STRING_DESCRIPTOR _LangID_StringDescriptor =
{
    0x04,       //bSize = 4 bytes
    0x03,       //Endpoint type = 3 = string descriptor
    {0x0409}    //Lang ID = US English
};

const USB_STRING_DESCRIPTOR *LangID_StringDescriptor = &_LangID_StringDescriptor;


const USB_STRING_DESCRIPTOR _Manufacturer_StringDescriptor =
{
    0x1C,
    0x03,
    {0x0052,0x0069,0x0063,0x0068,0x0061,0x0072,0x0064,0x0020,0x0053,0x0070,0x0065,0x0069,0x0072}
   //R      i      c      h      a      r      d      _      S      p      e      i      r
   //2      4      6      8      10     12     14     16     18     20     22     24     26
};

const USB_STRING_DESCRIPTOR *Manufacturer_StringDescriptor = &_Manufacturer_StringDescriptor;


const USB_STRING_DESCRIPTOR _Product_StringDescriptor =
{
    0x14,
    0x03,
    {0x0055,0x0053,0x0042,0x0020,0x0054,0x0068,0x0069,0x006e,0x0067}
   //U      S      B      _      T      h      i      n      g
   //2      4      6      8      10     12     14     16     18
};

const USB_STRING_DESCRIPTOR *Product_StringDescriptor = &_Product_StringDescriptor;


const USB_STRING_DESCRIPTOR _Serial_StringDescriptor =
{
    0x0C,
    0x03,
    {0x0030,0x002e,0x0030,0x002e,0x0031}
   //0      .      0      .      1
   //2      4      6      8      10
};

const USB_STRING_DESCRIPTOR *Serial_StringDescriptor = &_Serial_StringDescriptor;
