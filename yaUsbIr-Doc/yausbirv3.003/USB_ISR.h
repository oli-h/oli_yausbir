//**********************************************************************+*********************************************
// CREATION : 17.01.2011 Uwe Guenther
//**********************************************************************+*********************************************
#ifndef USB_ISR_H
#define USB_ISR_H

// Setup Packet Type Definition
typedef struct {
  BYTE bmRequestType;                   // Request recipient, type, direction
  BYTE bRequest;                        // Specific standard request number
  tWORD wValue;                         // varies according to request
  tWORD wIndex;                         // varies according to request
  tWORD wLength;                        // Number of bytes to transfer
} Tsetup_buffer;

// Global variables
extern BYTE          USB_State;         // Hold current usb device state
extern Tsetup_buffer Setup;             // Buffer for current device request
extern bit           setup_handled;     // flag that indicates setup stage is handled or not
extern UINT          DataSize;          // Size of data to return
extern BYTE*         DataPtr;           // Pointer to data to return

// Holds the status for each endpoint
extern volatile BYTE Ep_Status0;
extern volatile bit Ep_StatusOUT1;
extern volatile bit Ep_StatusIN1;

// FIFO status of endpoints
extern volatile bit IN_FIFO_empty;
extern volatile bit OUT_FIFO_loaded;

/*
// variable allocation
#ifdef ALLOCATE_VARS
#define EXTERN
#define EXT_TERM
#else
#define EXTERN extern
#define EXT_TERM    ;/ ## /
#endif

// Holds the status for each endpoint
EXTERN volatile BYTE Ep_Status0    EXT_TERM = EP_IDLE;
//EXTERN volatile bit Ep_StatusOUT1  EXT_TERM   = EP_HALT;
EXTERN volatile bit Ep_StatusIN1   EXT_TERM = EP_HALT;

// FIFO status of endpoints
EXTERN volatile bit IN_FIFO_empty       EXT_TERM    = TRUE;
EXTERN volatile bit OUT_FIFO_loaded     EXT_TERM    = FALSE;
*/

// Prototypes
void vUsb0Init(void);
void vUsbSuspend(void);
void vUsbResume(void);

// Used for multiple byte reads of Endpoint FIFOs
void Fifo_Read(BYTE addr, BYTE uNumBytes, BYTE * pData);
// Used for multiple byte writes of Endpoint FIFOs
void Fifo_Write(BYTE addr, BYTE uNumBytes, BYTE * pData);

/*
void Vendor_Request(void);            // Vendor request
*/

#endif // USB_ISR_H
