//**********************************************************************+*********************************************
// CREATION : 17.01.2011 Uwe Guenther
//**********************************************************************+*********************************************

#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

// Definition of descriptors

// Standard Device Descriptor Type Defintion
typedef struct {
  BYTE bLength;                         // Size of this Descriptor in Bytes
  BYTE bDescriptorType;                 // Descriptor Type (=1)
  UINT bcdUSB;                          // USB Spec Release Number in BCD
  BYTE bDeviceClass;                    // Device Class Code
  BYTE bDeviceSubClass;                 // Device Subclass Code
  BYTE bDeviceProtocol;                 // Device Protocol Code
  BYTE bMaxPacketSize0;                 // Maximum Packet Size for EP0
  UINT idVendor;                        // Vendor ID
  UINT idProduct;                       // Product ID
  UINT bcdDevice;                       // Device Release Number in BCD
  BYTE iManufacturer;                   // Index of String Desc for Manufacturer
  BYTE iProduct;                        // Index of String Desc for Product
  BYTE iSerialNumber;                   // Index of String Desc for SerNo
  BYTE bNumConfigurations;              // Number of possible Configurations
} Tdevice_descriptor;

// Standard Configuration Descriptor Type Definition
typedef struct {
  BYTE bLength;                         // Size of this Descriptor in Bytes
  BYTE bDescriptorType;                 // Descriptor Type (=2)
  UINT wTotalLength;                    // Total Length of Data for this Conf
  BYTE bNumInterfaces;                  // # of Interfaces supported by Conf
  BYTE bConfigurationValue;             // Designator Value for *this* Conf
  BYTE iConfiguration;                  // Index of String Desc for this Conf
  BYTE bmAttributes;                    // Configuration Characteristics
  BYTE bMaxPower;                       // Max. Power Consumption in Conf (*2mA)
} Tconfiguration_descriptor;

// Standard Interface Descriptor Type Definition
typedef struct {
  BYTE bLength;                         // Size of this Descriptor in Bytes
  BYTE bDescriptorType;                 // Descriptor Type (=4)
  BYTE bInterfaceNumber;                // Number of *this* Interface (0..)
  BYTE bAlternateSetting;               // Alternative for this Interface
  BYTE bNumEndpoints;                   // No of EPs used by this IF (excl. EP0)
  BYTE bInterfaceClass;                 // Interface Class Code
  BYTE bInterfaceSubClass;              // Interface Subclass Code
  BYTE bInterfaceProtocol;              // Interface Protocol Code
  BYTE iInterface;                      // Index of String Desc for Interface
} Tinterface_descriptor;

// Standard Endpoint Descriptor Type Definition
typedef struct {
   BYTE bLength;                        // Size of this Descriptor in Bytes
   BYTE bDescriptorType;                // Descriptor Type (=5)
   BYTE bEndpointAddress;               // Endpoint Address (Number + Direction)
   BYTE bmAttributes;                   // Endpoint Attributes (Transfer Type)
   UINT wMaxPacketSize;                 // Max. Endpoint Packet Size
   BYTE bInterval;                      // Polling Interval (Interrupt) ms
} Tendpoint_descriptor;

// Class specific descriptors for HID

// HID class Descriptor
typedef struct {
  BYTE bLength;
  BYTE bDescriptorType;
  UINT bcdHID;
  BYTE bCountryCode;
  BYTE bNumDescriptors;
  BYTE bReportDescriptorType;
  UINT wReportDescriptorLength;
} THID_class_descriptor;

// Configuration Descriptor Set
typedef struct {
  Tconfiguration_descriptor           m_config_desc;
    Tinterface_descriptor             m_interface_desc_0;
      THID_class_descriptor           m_HID_class_descriptor; // HID IF
      Tendpoint_descriptor            m_endpoint_desc_IN1;
      Tendpoint_descriptor            m_endpoint_desc_OUT2;
} Tconfiguration_desc_set;

// Descriptor type
#define DSC_TYPE_DEVICE         0x01
#define DSC_TYPE_CONFIG         0x02
#define DSC_TYPE_STRING         0x03
#define DSC_TYPE_INTERFACE      0x04
#define DSC_TYPE_ENDPOINT       0x05

// Endpoint transfer type
#define DSC_EP_CONTROL          0x00
#define DSC_EP_ISOC             0x01
#define DSC_EP_BULK             0x02
#define DSC_EP_INTERRUPT        0x03

// Endopoint address
#define OUT_EP1                 0x01
#define IN_EP1                  0x81

// Descriptor type (Class specific)
#define DSC_TYPE_CS_INTERFACE       0x24

// Descriptor type (Class specific) - HID
#define DSC_SUBTYPE_CS_HID_CLASS    0x21
#define DSC_SUBTYPE_CS_HID_REPORT   0x22
#define DSC_SUBTYPE_CS_HID_PHYSICAL 0x23

// Global variables
extern Tdevice_descriptor code DeviceDesc;
extern Tconfiguration_desc_set code ConfigDescSet;
extern BYTE code * code StringDescTable[];
extern BYTE code StringDescNum;
extern BYTE code HID_report_desc[];
extern BYTE code HID_report_desc_size;

#endif  // USB_DESCRIPTORS_H
