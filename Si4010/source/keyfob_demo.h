/*------------------------------------------------------------------------------
 *                          Silicon Laboratories, Inc.
 *                               Copyright 2010
 *                         CONFIDENTIAL & PROPRIETARY
 *------------------------------------------------------------------------------
 *
 *    FILE:       keyfob_demo.h
 *    TARGET:     Si4010
 *    TOOLCHAIN:  Keil
 *    RELEASE:    1.1 ; ROM version 02.00
 *    AUTHOR:     Miklos Lukacs
 *    DATE:       Jun 07, 2010
 *
 *------------------------------------------------------------------------------
 *
 *    DESCRIPTION:
 *      Header file for the keyfob_demo module.
 *
 *------------------------------------------------------------------------------*/
#ifndef _KEYFOB_DEMO_H
#define _KEYFOB_DEMO_H


/*------------------------------------------------------------------------------
 *
 *    INCLUDES:
 */
#include "si4010_types.h"

/*------------------------------------------------------------------------------
 *
 *    Definitions:
 */

#define bRke_EncodeNoneNrz_c       0   /* No encoding */
#define bRke_EncodeManchester_c    1   /* Manchester encoding */
#define bRke_Encode4b5b_c          2   /* 4b-5b encoding */

#define bRke_OokMod_c        (0x00)
#define bRke_FskMod_c        (0x01)

// button 'identifiers'
#define bButton1_c              0x01
#define bButton2_c              0x02
#define bButton3_c              0x04
#define bButton4_c              0x08
#define bButton5_c              0x10

// definition of the RF packet lenght
#define bFrameSize_c            42

// RF packet fields
#define lFramePreamble_c        0x555555


// Amount of time (x32ms), the application will wait after boot without
// getting a qualified button push before giving up and shutting the chip down.
// Sets the maximum wait time for push as 1.6s; the variable is unit in 32ms.
#define	bMaxWaitForPush_c		50

// This specifies the number of times that the RTC ISR is called between 
// each call to the button service routine.  The actual time between
// calls is dependent on the setting of RTC_CTRL.RTC_DIV which dictates how
// often the RTC ISR is called.    
#define	bDebounceInterval_c		8

// Sets target frequency to 433.92MHz
#define fOperatingFrequency  	433920000

// Sets FSK deviation ~60kHz
//#define bFskDeviation 			104

// The time of packet transmission for the demo:
// Here set the packet interval about (40 * 3) +15 = 130 ms.
#define wRepeatInterval 		135+25

// Repeat packet 4 times for each button packet transmission.
#define bRepeatCount_c  		4




/*------------------------------------------------------------------------------
 *
 *    VARIABLES:
 */

// Packet transmit repeat counter 
 BYTE xdata bRepeatCount;
 
 LWORD xdata lTimestamp;

// storing the button which is under processing
 BYTE xdata bAp_ButtonState;

// Pointer to the head of the frame to be sent out.
// the vStl_SingleTxLoop() API function expects a pointer residing in the Data memory
 BYTE xdata *pbRke_FrameHead;

// Structure for setting up the ODS .. must be in XDATA //
  tOds_Setup xdata rOdsSetup;

// Structure for setting up the PA .. must be in XDATA //
  tPa_Setup xdata  rPaSetup;

// Battery measurement .. delete if battery measurement is not used //
  int           iBatteryMv;
  BYTE          bBatteryWait;   // Wait time after loading in 17us increment //

// BSR control structure //
  tBsr_Setup      xdata rBsrSetup;
  BYTE xdata abBsrPtsPlaceHolder [20] = {0};

// setup structure for external crystal oscilator //
  tFCast_XoSetup rFCast_XoSetup;

 BYTE bIsr_DebounceCount;
 BYTE bdata bPrevBtn = 0;

// array for storing the TX packet
// needs to be present in the global variable area because it is used in more 
// than one Overly Module
 BYTE  xdata abButtonFrame[bFrameSize_c];

// storing the payload of the RF packet, and used as a return status of the 
// vPacketAssemble() function
 LWORD xdata lPayLoad;



/*-------------------------------------------------------------------------
 *
 *    FUNCTION PROTOTYPES:										
 */

void vAppInitial(void);

void vButtonCheck(void);

void vPacketAssemble(void);

void vButtonCheck(void);

void vRtl_RepeatTxLoop(void);



#endif /* _KEYFOB_DEMO_H */
