//**********************************************************************+*********************************************
// CREATION : 17.01.2011 Uwe Guenther
//**********************************************************************+*********************************************

#ifndef USB_CONFIGURATION_H
#define USB_CONFIGURATION_H

// Definition of configuration
#define VER_USB                 0x0110  // USB specification revision

// Meine registierte VID/PID
#define VID                     0x10C4  // Vendor ID
#define PID                     0x876C  // Product ID

#define DEV_REV                 0x0100  // Device Release number

enum {                                  // interface number
  DSC_INTERFACE_HID,
  DSC_NUM_INTERFACE                     // Number of Interfaces (for config desc)
};

                                        // Define Endpoint Packet Sizes
#define EP0_PACKET_SIZE         0x40    // Endpoint 0

                                        // Endpoint 1-3
                                        //  For full-speed,
                                        //      bulk:       8, 16, 32, 64
                                        //      interrupt:  0 - 64
                                        //      isoc:       0 - 1023 (512 for 'F32x/34x)
#define EP1_PACKET_SIZE         0x0040

// Endpoint usage - uncomment the def for the EPs to use
#define USE_OUT_EP1
#define USE_IN_EP1

// Endpoint double buffer - uncomment the def for the EPs to apply
//#define ENABLE_OUT_EP1_DOUBLE_BUF
//#define ENABLE_IN_EP1_DOUBLE_BUF

// Isochronous Endpoint - uncomment the def for the EPs to apply
//  When this option is not applied, the EP is set to interrupt or bulk
//#define ENABLE_OUT_EP1_ISO
//#define ENABLE_IN_EP1_ISO

// Endpoint interrupt - uncomment the def for the EPs to apply
#define ENABLE_OUT_EP1_INTERRUPT
#define ENABLE_IN_EP1_INTERRUPT

// Suspend/Resume handling - uncomment this def to apply
#define ENABLE_SUSPEND_RESUME

// SOF interrupt - uncomment this def to apply
//#define ENABLE_SOF_INTERRUPT

// Inline POLL_READ_BYTE / POLL_WRITE_BYTE - uncomment this def  to apply
//
// When this def is applied, POLL_READ_BYTE / POLL_WRITE_BYTE is defined as inline macros.
// Otherwise, these are defined as functions.
// Inline macro gives fast execution speed, but greater code size
#define ENABLE_INLINE_POLL_READ_WRITE

#endif  // USB_CONFIGURATION_H
