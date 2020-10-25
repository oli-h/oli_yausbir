//-----------------------------------------------------------------------------
// USB_Standard_Requests.c
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Implementation details and suggestions
//-----------------------------------------------------------------------------
//
// [HALT and STALL handling on interrupt and bulk EPs]
//  HALT is set to one by
//  - Hardware (firmware) condition
//  - SetFeature( ENDPOINT_HALT )
//
//  HALT is reset to zero by
//  - SetConfiguration(), SetInterface()
//  - ClearFeature(ENDPOINT_HALT), if the hardware (firmware) condition meets
//
//  GetStatus( ENDPOINT ) returns the HALT state of the EP.
//  STALL is always returned by the EP while the EP is in HALT.
//  Isochronous EP doesn't support HALT/STALL
//
//  This implementaion fully supports this HALT scheme, exept for Hardware HALT
//
// [Data toggle handling on interrupt and bulk EPs]
//  - Set to DATA0 on Set_Configuration and Set_Interface
//  - Set to DATA0 on ClearFeature( ENDPOINT_HALT )
//
//  This implementaion fully supports this data toggle scheme
//
// [Bus-powered/Self-powered]
//  USB device can switch the actual power source on the fly.
//  bmAttributes of the config descriptor shows the default power source.
//      D6: 0 = pure bus-powered device
//          1 = pure self-powered or switchable device.
//  bMaxPower of the config descriptor indicates power requirement from the bus in mA / 2
//      pure bus-powered and switchable device has to set this value to other than 0
//      when this value is less than or equal to 50 (=100mA), enumeration will always success
//      but when more than 50, enumeration may fail
//  GetStatus( DEVICE ) returns the current power source
//      D0: 0 = bus-powered / 1 = self-powered.
//
//  This implementaion gives a pure bus-powered device
//
// [Suspend/Resume and Remote wakeup]
//  This sequence differs according to OS
//  Windows
//      Non remote wakeup device, System hibernates
//          Suspend bus (stop SOF)  - System standby (PC) - Suspend interrupt (device)
//          ...
//          bus reset               - System wake up (PC) - (no Suspend interrupt)
//          start enumeration again
//
//      Remote wakeup device
//          SetFeature( DEVICE_REMOTE_WAKEUP )
//          Suspend bus (stop SOF)  - System standby (PC) - Suspend interrupt (device)
//          ...
//          Resume bus (start SOF)  - System wake up (PC) - Resume interrupt (device)
//          ClearFeature( DEVICE_REMOTE_WAKEUP )
//
//  MacOSX
//      Non remote wakeup device
//          Suspend bus (stop SOF)  - System sleep   (Mac) - Suspend interrupt (device)
//          ...
//          Resume bus (start SOF)  - System wake up (Mac) - Resume interrupt (device)
//          GetStatus( ENDPOINT ) (MSC) - depends on the class
//
//      Remote wakeup device
//          SetFeature( DEVICE_REMOTE_WAKEUP ) - at the end of enumeration
//          ...
//          Suspend bus (stop SOF)  - System sleep   (Mac) - Suspend interrupt (device)
//          ...
//          Resume bus (start SOF)  - System wake up (Mac) - Resume interrupt (device)
//
//  Linux
//      Linux seems not yet to establish suspend well
//
// This implementation gives non-remote-wakeup device
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "common.h"
#include "USB_Configuration.h"
#include "USB_Register.h"
#include "USB_Descriptor.h"
#include "USB_ISR.h"
#include "USB_Main.h"
#include "USB_Standard_Requests.h"

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Static Variables in this file
//-----------------------------------------------------------------------------

// These are response packets used for communication with host
static BYTE code ONES_PACKET[2] = {0x01, 0x00};     
static BYTE code ZERO_PACKET[2] = {0x00, 0x00};     

//-----------------------------------------------------------------------------
// Local function prototypes
//-----------------------------------------------------------------------------

static void Get_Status(void);
static void Clear_Feature(void);
static void Set_Feature(void);
static void Set_Address(void);
static void Get_Descriptor(void);
static void Get_Configuration(void);
static void Set_Configuration(void);
static void Get_Interface(void);
static void Set_Interface(void);
static bit  Set_Halt( BYTE endpoint, BYTE status );

//-----------------------------------------------------------------------------
// configuration conditions
//-----------------------------------------------------------------------------

#if (defined USE_OUT_EP1) && !(defined ENABLE_OUT_EP1_ISO)
    #define USE_OUT_EP1_STATUS
#endif
#if (defined USE_IN_EP1) && !(defined ENABLE_IN_EP1_ISO)
    #define USE_IN_EP1_STATUS
#endif

//-----------------------------------------------------------------------------
// Standard device request dispatcher
//-----------------------------------------------------------------------------

void Standard_Device_Request( void )
{
    switch(Setup.bRequest)
    {
        case GET_STATUS:        Get_Status();           break;
        case CLEAR_FEATURE:     Clear_Feature();        break;
        case SET_FEATURE:       Set_Feature();          break;
        case SET_ADDRESS:       Set_Address();          break;
        case GET_DESCRIPTOR:    Get_Descriptor();       break;
        case GET_CONFIGURATION: Get_Configuration();    break;
        case SET_CONFIGURATION: Set_Configuration();    break;
        case GET_INTERFACE:     Get_Interface();        break;
//      case SET_INTERFACE:     Set_Interface();        break;

        case STD_REQ_RESERVE1:
        case STD_REQ_RESERVE2:
        case SET_DESCRIPTOR:
        case SET_INTERFACE:
        case SYNCH_FRAME:
        default:                                        break;
    }
}

//-----------------------------------------------------------------------------
// Support Subroutines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Get_Status
//-----------------------------------------------------------------------------
//
// Get_Status( DEVICE )     : returns the current state of
//                              a) bus-powered/self-powered
//                              b) remote wakeup enabled/disabled
// Get_Status( INTERFACE )  : returns always 0x00
// Get_Status( ENDPOINT )   : returns current state of EP HALT
//
//-----------------------------------------------------------------------------

static void Get_Status(void)
{
    bit aStatus;

    // Valid when wValue equals to zero, and wLength (data length) equals to 2 
    if ( (Setup.wValue.i == 0) && (Setup.wLength.i == 2) )
    {
        // Determine if recipient was device, interface, or EP
        switch( Setup.bmRequestType )                   
        {
            case IN_DEVICE:                         // If recipient was device
                if ( Setup.wIndex.i == 0 )          // Valid when wIndex equals to zero
                {
                    // send 0x00, indicating bus power and no remote wake-up supported
                    DataPtr = (BYTE*)&ZERO_PACKET;
                    setup_handled = TRUE;
                }
                break;

            case IN_INTERFACE:                      // See if recipient was interface                       
                // Only valid if device is configured and existing interface index
                if ( (USB_State == DEV_CONFIGURED) && (Setup.wIndex.i < DSC_NUM_INTERFACE) )                                                
                {
                    // Status packet always returns 0x00
                    DataPtr = (BYTE*)&ZERO_PACKET;      
                    setup_handled = TRUE;
                }
                break;

            case IN_ENDPOINT:                       // See if recipient was an endpoint                         
                // Make sure device is configured and index msb = 0x00
                if ((USB_State == DEV_CONFIGURED) && (Setup.wIndex.c[MSB] == 0) )
                {
                    aStatus = EP_IDLE;
                    switch ( Setup.wIndex.c[LSB] )
                    {
#ifdef USE_OUT_EP1_STATUS
                        case OUT_EP1:   aStatus = Ep_StatusOUT1;    setup_handled = TRUE;   break;      
#endif
#ifdef USE_IN_EP1_STATUS
                        case IN_EP1:    aStatus = Ep_StatusIN1;     setup_handled = TRUE;   break;
#endif
                        default:                                                            break;
                    }
                    if (aStatus == EP_HALT)
                    {
                        // If endpoint is halted, return 0x01,0x00
                        DataPtr = (BYTE*)&ONES_PACKET;
                    } else {
                        // Otherwise return 0x00,0x00 to indicate endpoint active
                        DataPtr = (BYTE*)&ZERO_PACKET;
                    }
                }
                break;

            default:
                break;
        }
    }

    if ( setup_handled )
    {
        // Set serviced Setup Packet, Endpoint 0 intransmit mode, 
        Ep_Status0 = EP_TX;                     
        DataSize = 2;                       
    }
}

//-----------------------------------------------------------------------------
// Set_Halt
//-----------------------------------------------------------------------------
//
// Helper routine to set/reset HALT on the EP
//  endpoint:   Endopoint address, (OUT_EP1, IN_EP1, etc) defined in USB_Descriptor.h
//  status:     EP_IDLE / EP_HALT
//
//  Set/Reset HALT flag to the specified EP
//  Enable/disable STALL on the EP
//  Clear the data toggle
//
//-----------------------------------------------------------------------------

static bit Set_Halt( BYTE endpoint, BYTE status )
{
    BYTE controlReg;

    switch ( endpoint ) {

#ifdef USE_OUT_EP1_STATUS
        case OUT_EP1:   Ep_StatusOUT1 = status;     break;
#endif
#ifdef USE_IN_EP1_STATUS
        case IN_EP1:    Ep_StatusIN1  = status;     break;
#endif
        default:        return FALSE;
    }

    POLL_WRITE_BYTE (INDEX, endpoint & 0x7F);       // Set endpoint index masking IN/OUT bit
    if ( endpoint & 0x80 ) {                        // IN endpoints
        if ( status == EP_IDLE )
            controlReg = (rbInCLRDT | rbInFLUSH);   // clear data toggle, SDSTL, STSTL
        else
            controlReg = (rbInCLRDT | rbInSDSTL | rbInFLUSH);   // send STALL
        POLL_WRITE_BYTE( EINCSRL, controlReg );

#if defined ENABLE_IN_EP1_DOUBLE_BUF
        {                                           // clear double buffer
            BYTE eincsrl;
            do {                                    // wait until FLUSH complete
                POLL_READ_BYTE( EINCSRL, eincsrl );
            } while ( eincsrl & rbInFLUSH );
            if ( eincsrl & rbInFIFONE ) {           // clear double buffer
                POLL_WRITE_BYTE( EINCSRL, controlReg );
            }
        }
#endif

    } else {                                        // OUT endpoints
        if ( status == EP_IDLE )
            controlReg = (rbOutCLRDT | rbOutFLUSH); // clear data toggle, SDSTL, STSTL
        else
            controlReg = (rbOutCLRDT | rbOutSDSTL | rbOutFLUSH);    // send STALL
        POLL_WRITE_BYTE( EOUTCSRL, controlReg );

#if defined ENABLE_OUT_EP1_DOUBLE_BUF
        {                                           // clear double buffer
            BYTE eoutcsrl;
            do {                                    // wait until FLUSH complete
                POLL_READ_BYTE( EOUTCSRL, eoutcsrl );
            } while ( eoutcsrl & rbOutFLUSH );
            POLL_WRITE_BYTE( EOUTCSRL, controlReg );// clear double buffer
        }
#endif

    }
                                                    // initialize USB-related variables
                                                    // because the FIFO is flushed
    if ( endpoint == IN_EP1 )
        IN_FIFO_empty   = TRUE;
    if ( endpoint == OUT_EP1 )
        OUT_FIFO_loaded = FALSE;

    return TRUE;
}

//-----------------------------------------------------------------------------
// Clear_Feature
//-----------------------------------------------------------------------------
//
// Clear_Feature( DEVICE_REMOTE_WAKEUP ): disable remote wakeup
// Clear_Feature( ENDPOINT_HALT )       : clear HALT satate of the EP
//
//-----------------------------------------------------------------------------

static void Clear_Feature()
{
    if (   (USB_State == DEV_CONFIGURED)            // Make sure device is configured
        && (Setup.wLength.i == 0) )                 // and data length set to zero.
    {
        switch( Setup.bmRequestType )                   
        {
            case OUT_DEVICE:                        // for device, only remote wakeup is valid
                if (   (Setup.wValue.i == DEVICE_REMOTE_WAKEUP)
                    && (Setup.wIndex.i == 0) )
                {
                    // disable remote wakeup here
                    setup_handled = TRUE;
                }
                break;

            case OUT_ENDPOINT:                      // for endpoint, only endpoint halt is valid
                if (   (Setup.wValue.i      == ENDPOINT_HALT)
                    && (Setup.wIndex.c[MSB] == 0) )
                {
                    if ( Set_Halt( Setup.wIndex.c[LSB], EP_IDLE ) )
                        setup_handled = TRUE;
                }
                break;

            default:
                break;
        }
    }
}

//-----------------------------------------------------------------------------
// Set_Feature
//-----------------------------------------------------------------------------
//
// Set_Feature( DEVICE_REMOTE_WAKEUP )  : enable remote wakeup
// Set_Feature( ENDPOINT_HALT )         : set HALT satate to the EP
//
//-----------------------------------------------------------------------------

static void Set_Feature(void)
{
    if (   (USB_State       == DEV_CONFIGURED)      // Make sure device is configured
        && (Setup.wLength.i == 0) )                 // and data length set to zero.
    {
        switch( Setup.bmRequestType )                   
        {
            case OUT_DEVICE:                        // for device, only remote wakeup is valid
                if (   (Setup.wValue.i == DEVICE_REMOTE_WAKEUP)
                    && (Setup.wIndex.i == 0) )
                {
                    // enable remote wakeup here
                    setup_handled = TRUE;
                }
                break;

            case OUT_ENDPOINT:                      // for endpoint, only endpoint halt is valid
                if (   (Setup.wValue.i      == ENDPOINT_HALT)
                    && (Setup.wIndex.c[MSB] == 0) )
                {
                    if ( Set_Halt( Setup.wIndex.c[LSB], EP_HALT ) )
                        setup_handled = TRUE;
                }
                break;

            default:
                break;
        }
    }
}

//-----------------------------------------------------------------------------
// Set_Address
//-----------------------------------------------------------------------------
//
// Set device address to the USB engine
//
//-----------------------------------------------------------------------------

static void Set_Address(void)
{
    if (   (Setup.bmRequestType == OUT_DEVICE)      // Request must be directed to device
        && (Setup.wIndex.i == 0)                    // with index and length set to zero.
        && (Setup.wLength.i == 0)                   // wValue holds the address, up to 0x7F
        && (Setup.wValue.c[MSB] == 0) && ((Setup.wValue.c[LSB] & 0x80) == 0) )
    {
        if (Setup.wValue.c[LSB] != 0)
        {
            POLL_WRITE_BYTE(FADDR, Setup.wValue.c[LSB]);    // write new address to FADDR
                                                            // SIE applies this address after status stage
            USB_State = DEV_ADDRESS;                // Indicate that device state is now address
        }
        else
        {
            USB_State = DEV_DEFAULT;                // If new address was 0x00, return device to default state
        }
        setup_handled = TRUE;
    }
}

//-----------------------------------------------------------------------------
// Get_Descriptor
//-----------------------------------------------------------------------------
//
// Return specified descriptor
//
// bmRequestType == DEVICE
//  wValue[MSB] : DEVICE                device descriptor
//              : CONFIG                config descriptor
//              : STRING                string descriptor
//              : DEVICE_QUALIFIER      for HS support
//              : OTHER_SPEED_CONFIG    for HS support
//
// bmRequestType == INTERFACE
//  class specific descriptors
//
//-----------------------------------------------------------------------------

static void Get_Descriptor(void)
{
    if ( Setup.bmRequestType == IN_DEVICE )         // Request to device
    {
        switch(Setup.wValue.c[MSB])                 // Determine which type of descriptor
        {                                           // was requested, and set data ptr and
            case DST_DEVICE:                        // size accordingly
                if ( (Setup.wValue.c[LSB] == 0) && (Setup.wIndex.i == 0) )
                {
                    DataPtr = (BYTE*)&DeviceDesc;
                    DataSize = sizeof( Tdevice_descriptor );
                    setup_handled = TRUE;
                }
                break;

            case DST_CONFIG:                        // wValue.LSB holds config index
                if ( (Setup.wValue.c[LSB] == 0) && (Setup.wIndex.i == 0) )
                {
                    DataPtr = (BYTE*)&ConfigDescSet;
                    DataSize = sizeof( Tconfiguration_desc_set );
                    setup_handled = TRUE;
                }
                break;

            case DST_STRING:                        // wValue.LSB holds string index
                                                    // wIndex holds language ID
                if ( Setup.wValue.c[LSB] < StringDescNum )
                {
                    DataPtr = StringDescTable[Setup.wValue.c[LSB]];
                    DataSize = *DataPtr;
                    setup_handled = TRUE;
                }
                break;

            default:
                break;
        }
    }
    else if ( Setup.bmRequestType == IN_INTERFACE ) // Request to Interface
    {

        if ( Setup.wIndex.i == DSC_INTERFACE_HID ) {    // HID interface
            switch(Setup.wValue.c[MSB])             // Determine which type of descriptor
            {
                case DSC_SUBTYPE_CS_HID_CLASS:
                    if ( Setup.wValue.c[LSB] == 0 )
                    {
                        DataPtr = (BYTE*)&(ConfigDescSet.m_HID_class_descriptor);
                        DataSize = sizeof( THID_class_descriptor );
                        setup_handled = TRUE;
                    }
                    break;

                case DSC_SUBTYPE_CS_HID_REPORT:
                    if ( Setup.wValue.c[LSB] == 0 )
                    {
                        DataPtr = (BYTE*)HID_report_desc;
                        DataSize = HID_report_desc_size;
                        setup_handled = TRUE;
                    }
                    break;

                default:
                    break;
            }

        }
    }

    if ( setup_handled )
    {
        Ep_Status0 = EP_TX;                         // Put endpoint in transmit mode
        if ( DataSize > Setup.wLength.i )
            DataSize = Setup.wLength.i;             // Send only requested amount of data
    }
}

//-----------------------------------------------------------------------------
// Get_Configuration
//-----------------------------------------------------------------------------
//
// This routine returns current configuration value (default: 0x01)
//
//-----------------------------------------------------------------------------

static void Get_Configuration(void)
{
    if (   (Setup.bmRequestType == IN_DEVICE)       // This request must be directed to the device
        && (Setup.wValue.i == 0)                    // with value word set to zero
        && (Setup.wIndex.i == 0)                    // and index set to zero
        && (Setup.wLength.i == 1) )                 // and setup length set to one
    {
        if (USB_State == DEV_CONFIGURED)            // If the device is configured, then return value 0x01
        {                                           // since this software only supports one configuration
            DataPtr = (BYTE*)&ONES_PACKET;
            setup_handled = TRUE;
        }
        if (USB_State == DEV_ADDRESS)               // If the device is in address state, it is not
        {                                           // configured, so return 0x00
            DataPtr = (BYTE*)&ZERO_PACKET;
            setup_handled = TRUE;
        }
    }
    
    if ( setup_handled )
    {
        // Set serviced Setup Packet, Endpoint 0 intransmit mode
        Ep_Status0 = EP_TX;                     
        DataSize = 1;                       
    }
}

//-----------------------------------------------------------------------------
// Set_Configuration
//-----------------------------------------------------------------------------
//
// Set up EPs and USB-related variables
//
//-----------------------------------------------------------------------------

// configuration conditions

// EP1

#if (defined USE_OUT_EP1) && (defined USE_IN_EP1) && ((defined C8051F320_H) || (defined C8051F340_H))
    #define EP1_CONFIG_SPLIT    rbInSPLIT               // split EP FIFO
#else
    #define EP1_CONFIG_SPLIT    0
#endif
#if !(defined USE_OUT_EP1) && (defined USE_IN_EP1) && ((defined C8051F320_H) || (defined C8051F340_H))
    #define EP1_CONFIG_IN       rbInDIRSEL              // set EP to IN
#else
    #define EP1_CONFIG_IN       0
#endif
#ifdef ENABLE_IN_EP1_DOUBLE_BUF
    #define EP1_CONFIG_IN_DBLBUF    rbInDBIEN           // enable double buffer
#else
    #define EP1_CONFIG_IN_DBLBUF    0
#endif
#ifdef ENABLE_IN_EP1_ISO
    #define EP1_CONFIG_IN_ISO   rbInISO                 // set to isoc
#else
    #define EP1_CONFIG_IN_ISO   0
#endif

#ifdef ENABLE_OUT_EP1_DOUBLE_BUF
    #define EP1_CONFIG_OUT_DBLBUF   rbOutDBOEN          // enable double buffer
#else
    #define EP1_CONFIG_OUT_DBLBUF   0
#endif
#ifdef ENABLE_OUT_EP1_ISO
    #define EP1_CONFIG_OUT_ISO  rbOutISO                // set to isoc
#else
    #define EP1_CONFIG_OUT_ISO  0
#endif

#define EP1_EOUTCSRH    (EP1_CONFIG_OUT_DBLBUF | EP1_CONFIG_OUT_ISO)
#define EP1_EINCSRH (EP1_CONFIG_IN_DBLBUF | EP1_CONFIG_IN_ISO | EP1_CONFIG_IN | EP1_CONFIG_SPLIT)

// Body of Set_Configuration

static void Set_Configuration(void)
{
    if (   (Setup.bmRequestType == OUT_DEVICE)      // This request must be directed to the device
        && (Setup.wIndex.i == 0)                    // and index set to zero
        && (Setup.wLength.i == 0) )                 // and data length set to one
    {
        switch( Setup.wValue.c[LSB] )
        {
            case 0:
                USB_State = DEV_ADDRESS;            // Unconfigures device by setting state to
                                                    // address, and changing endpoint 1, 2 and 3
                                                    // status to halt
#ifdef USE_OUT_EP1_STATUS
                Ep_StatusOUT1 = EP_HALT;
#endif
#ifdef USE_IN_EP1_STATUS
                Ep_StatusIN1  = EP_HALT;
#endif
                setup_handled = TRUE;
                break;

            case 1:                                 // default configuration
                USB_State = DEV_CONFIGURED;

                // The endpoint regsiters, E0CSR, EINCSRL/H and EOUTCSRL/H are cleared by bus reset.
                // Set_Configuration is a good place to set these registers.
                // Also initialize USB-related variables here.
                // When the device has any alternate interface, initialize these registers in Set_Interface
                //
                // In 'F32x/34x, bus reset flushes all EP FIFOs, and set data toggle to DATA0
                // Then, data toggle setup and FIFO flush is not handled here in this implementation.

                                                        // configure EPs
#if (defined USE_OUT_EP1) || (defined USE_IN_EP1)
                POLL_WRITE_BYTE(INDEX, 1);                  // select EP1
    #if (EP1_EINCSRH != 0)
                POLL_WRITE_BYTE(EINCSRH, EP1_EINCSRH);      // set EINCSRH
    #endif
    #if (EP1_EOUTCSRH != 0)
                POLL_WRITE_BYTE(EOUTCSRH, EP1_EOUTCSRH);    // set EOUTCSRH
    #endif
#endif

                                                    // set EP status to IDLE
#ifdef USE_OUT_EP1_STATUS
                Ep_StatusOUT1 = EP_IDLE;
#endif
#ifdef USE_IN_EP1_STATUS
                Ep_StatusIN1  = EP_IDLE;
#endif
                                                    // initialize USB-related variables
                IN_FIFO_empty   = TRUE;
                OUT_FIFO_loaded = FALSE;

                setup_handled = TRUE;
                break;

            default:
                break;
        }
    }
}

//-----------------------------------------------------------------------------
// Get_Interface
//-----------------------------------------------------------------------------
//
// Return current interface (default: 0x00)
//
//-----------------------------------------------------------------------------

static void Get_Interface(void)
{
    if (   (USB_State == DEV_CONFIGURED)            // If device is configured
        && (Setup.bmRequestType == IN_INTERFACE)    // and recipient is an interface
        && (Setup.wValue.i == 0)                    // and wValue equals to 0
        && (Setup.wIndex.i < DSC_NUM_INTERFACE)     // and valid interface index
        && (Setup.wLength.i == 1) )                 // and data length equals to one
    {
        DataPtr = (BYTE*)&ZERO_PACKET;              // return 0x00 to host
        Ep_Status0 = EP_TX;
        DataSize = 1;
        setup_handled = TRUE;
    }
}

//-----------------------------------------------------------------------------
// Set_Interface
//-----------------------------------------------------------------------------
//
// Switch the interface to one of alternate interface
// When the device has no alternate IF, just reutrn STALL
//
//-----------------------------------------------------------------------------

/*
static void Set_Interface(void)
{
    // return STALL when no alternate interface
    if (   (Setup.bmRequestType == OUT_INTERFACE)
        && (Setup.wValue.i == 0 )       // wValue holds alternate interface index
        && (Setup.wIndex.i < DSC_NUM_INTERFACE)
        && (Setup.wLength.i == 0) )
    {
        // Indicate setup packet has been serviced
        setup_handled = TRUE;
    }
}
*/
