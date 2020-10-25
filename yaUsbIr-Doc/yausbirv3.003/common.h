//**********************************************************************+*********************************************
//
// CREATION :      14.03.98 Uwe Guenther
// CHANGE   : 
//                                                                        
//**********************************************************************+*********************************************
                                           
#ifndef COMMON_H                            
#define COMMON_H                            

//*** Konstanten *******************************************************+*********************************************

#include <compiler_defs.h>
#include <C8051F380_defs.h>
#include <intrins.h>                                                    // for _testbit_(), _nop_()
#define C8051F340_H
#define TRUE       1                                                      
#define FALSE      0                                                      
                                            
//*** Datentypen *******************************************************+*********************************************
                                                                          
typedef bit             BOOL;
typedef unsigned char   BYTE;
typedef unsigned int    UINT;
typedef unsigned int    WORD;
typedef unsigned long   ULONG;
typedef unsigned long   DWORD;

// tWORD type definition
#define MSB     0
#define LSB     1
typedef union {WORD w; UINT i; BYTE b[2]; BYTE c[2];} tWORD;

// tDWORD type definition
typedef union {DWORD z; BYTE b[4];WORD w[2];} tDWORD;
                                                                          
#endif                                       
