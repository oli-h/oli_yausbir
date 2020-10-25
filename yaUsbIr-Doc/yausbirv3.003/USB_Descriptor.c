//**********************************************************************+*********************************************
// CREATION : 17.01.2011 Uwe Guenther
//**********************************************************************+*********************************************

#include "common.h"
#include "USB_Configuration.h"
#include "USB_Descriptor.h"

// Macro for two byte field
#define LE(x)   ((((x)&0x00FF)<<8)|(((x)&0xFF00)>>8))   // convert to little endian

// Descriptor Declarations

// Device descriptor
Tdevice_descriptor code DeviceDesc =
{
    sizeof(Tdevice_descriptor),             // bLength
    DSC_TYPE_DEVICE,                        // bDescriptorType
    LE( VER_USB ),                          // bcdUSB
    0x00,                                   // bDeviceClass
    0x00,                                   // bDeviceSubClass
    0x00,                                   // bDeviceProtocol
    EP0_PACKET_SIZE,                        // bMaxPacketSize0
    LE( VID ),                              // idVendor
    LE( PID ),                              // idProduct
    LE( DEV_REV ),                          // bcdDevice
    0x01,                                   // iManufacturer
    0x02,                                   // iProduct
    0x03,                                   // iSerialNumber
    0x01                                    // bNumConfigurations
};

// HID report descriptor placed here because the size of this array is referred in config:HID class desc
BYTE code HID_report_desc[] =
{
    0x06, 0x00, 0xff,                       // USAGE_PAGE (Vendor Defined Page 1)
    0x09, 0x01,                             // USAGE (Vendor Usage 1)
    0xa1, 0x01,                             // COLLECTION (Application)
    0x15, 0x00,                             //  LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,                       //  LOGICAL_MAXIMUM (255)
    0x75, 0x08,                             //  REPORT_SIZE  (8)    - bits
    0x95, 0x40,                             //  REPORT_COUNT (64)   - Bytes
    0x09, 0x01,                             //  USAGE (Vendor Usage 1)
    0x81, 0x02,                             //  INPUT (Data,Var,Abs)
    0x09, 0x01,                             //  USAGE (Vendor Usage 1)
    0x91, 0x02,                             //  OUTPUT (Data,Var,Abs)
    0xc0                                    // END_COLLECTION
};

BYTE code HID_report_desc_size = sizeof( HID_report_desc );     // export report desc size
                                                                // Original idea by Patryk
// Configuration descriptor
Tconfiguration_desc_set code ConfigDescSet =
{
  {                                     // Configuration descriptor
    sizeof(Tconfiguration_descriptor),      // bLength
    DSC_TYPE_CONFIG,                        // bDescriptorType
    LE( sizeof(Tconfiguration_desc_set) ),  // bTotalLength
    DSC_NUM_INTERFACE,                      // bNumInterfaces
    0x01,                                   // bConfigurationValue
    0x04,                                   // iConfiguration
    0xC0,                                   // bmAttributes
    (100/2),                                // bMaxPower (100mA)
  },
  {                                     // Interface descriptor
    sizeof(Tinterface_descriptor),          // bLength
    DSC_TYPE_INTERFACE,                     // bDescriptorType
    DSC_INTERFACE_HID,                      // bInterfaceNumber
    0x00,                                   // bAlternateSetting
    0x02,                                   // bNumEndpoints
    0x03,                                   // bInterfaceClass (HID)
    0x00,                                   // bInterfaceSubClass
    0x00,                                   // bInterfaceProcotol
    0x00,                                   // iInterface
  },
  {                                     // HID class descriptor
    sizeof(THID_class_descriptor),          // bLength
    DSC_SUBTYPE_CS_HID_CLASS,               // bDescriptorType
    LE( 0x0111 ),                           // bcdHID (ver1.11)
    0x00,                                   // bCountryCode
    0x01,                                   // bNumDescriptors
    DSC_SUBTYPE_CS_HID_REPORT,              // bDescriptorType
    LE( sizeof( HID_report_desc ) )         // wDescriptorLength
  },
  {                                     // Endpoint1 IN descriptor
    sizeof(Tendpoint_descriptor),           // bLength
    DSC_TYPE_ENDPOINT,                      // bDescriptorType
    IN_EP1,                                 // bEndpointAddress
    DSC_EP_INTERRUPT,                       // bmAttributes
    LE( EP1_PACKET_SIZE ),                  // MaxPacketSize
    10,                                     // bInterval
  },
  {                                     // Endpoint1 OUT descriptor
    sizeof(Tendpoint_descriptor),           // bLength
    DSC_TYPE_ENDPOINT,                      // bDescriptorType
    OUT_EP1,                                // bEndpointAddress
    DSC_EP_INTERRUPT,                       // bmAttributes
    LE( EP1_PACKET_SIZE ),                  // MaxPacketSize
    10                                      // bInterval
  }
};

// String descriptors
#define STR0LEN 4
static BYTE code String0Desc[STR0LEN] =
{
    STR0LEN, DSC_TYPE_STRING, 0x09, 0x04
};

/*
#define STR1LEN sizeof("G-LABORATORIES")*2
static BYTE code String1Desc[STR1LEN] =
{
    STR1LEN, DSC_TYPE_STRING,
    'G', 0,
    '-', 0,
    'L', 0,
    'A', 0,
    'B', 0,
    'O', 0,
    'R', 0,
    'A', 0,
    'T', 0,
    'O', 0,
    'R', 0,
    'I', 0,
    'E', 0,
    'S', 0,
};
*/

#define STR1LEN sizeof("UG Development Lab")*2
static BYTE code String1Desc[STR1LEN] =
{
    STR1LEN, DSC_TYPE_STRING,
    'U', 0,
    'G', 0,
    ' ', 0,
    'D', 0,
    'e', 0,
    'v', 0,
    'e', 0,
    'l', 0,
    'o', 0,
    'p', 0,
    'm', 0,
    'e', 0,
    'n', 0,
    't', 0,
    ' ', 0,
    'L', 0,
    'a', 0,
    'b', 0,
};

#define STR2LEN sizeof("yaUsbIR V2:IR transceiver with power switch")*2
static BYTE code String2Desc[STR2LEN] =
{
    STR2LEN, DSC_TYPE_STRING,
    'y', 0,
    'a', 0,
    'U', 0,
    's', 0,
    'b', 0,
    'I', 0,
    'R', 0,
    ' ', 0,
    'V', 0,
    '3', 0,
    ':', 0,
    'I', 0,
    'R', 0,
    ' ', 0,
    't', 0,
    'r', 0,
    'a', 0,
    'n', 0,
    's', 0,
    'c', 0,
    'e', 0,
    'i', 0,
    'v', 0,
    'e', 0,
    'r', 0,
    ' ', 0,
    'w', 0,
    'i', 0,
    't', 0,
    'h', 0,
    ' ', 0,
    'p', 0,
    'o', 0,
    'w', 0,
    'e', 0,
    'r', 0,
    ' ', 0,
    's', 0,
    'w', 0,
    'i', 0,
    't', 0,
    'c', 0,
    'h', 0,
};

// serial number string
#define STR3LEN sizeof("0001")*2

static BYTE code String3Desc[STR3LEN] =
{
    STR3LEN, DSC_TYPE_STRING,
    '0', 0,
    '0', 0,
    '1', 0,
    '2', 0
};

#define STR4LEN sizeof("IR transceiver with power switch")*2
static BYTE code String4Desc[STR4LEN] =
{
    STR4LEN, DSC_TYPE_STRING,
    'I', 0,
    'R', 0,
    ' ', 0,
    't', 0,
    'r', 0,
    'a', 0,
    'n', 0,
    's', 0,
    'c', 0,
    'e', 0,
    'i', 0,
    'v', 0,
    'e', 0,
    'r', 0,
    ' ', 0,
    'w', 0,
    'i', 0,
    't', 0,
    'h', 0,
    ' ', 0,
    'p', 0,
    'o', 0,
    'w', 0,
    'e', 0,
    'r', 0,
    ' ', 0,
    's', 0,
    'w', 0,
    'i', 0,
    't', 0,
    'c', 0,
    'h', 0,
};

BYTE code * code StringDescTable[] =
{
    String0Desc,
    String1Desc,
    String2Desc,
    String3Desc,
    String4Desc
};

BYTE code StringDescNum = sizeof( StringDescTable ) / sizeof( StringDescTable[0] );
