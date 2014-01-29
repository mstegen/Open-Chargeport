/*------------------------------------------------------------------------------
 *                          Silicon Laboratories, Inc.
 *                           http://www.silabs.com
 *                               Copyright 2010
 *------------------------------------------------------------------------------
 *
 *    FILE:       keyfob_demo_main.c
 *    TARGET:     Si4010
 *    TOOLCHAIN:  Keil
 *    RELEASE:    1.1 ; ROM version 02.00
 *    AUTHOR:     Miklos Lukacs, 
 *                modified by Michael Stegen for use as Open Chargeport
 *    DATE:       October 07, 2013
 *
 *------------------------------------------------------------------------------
 *
 *    DESCRIPTION:
 *      This file contains the main function for the Keil toolchain
 *      sample keyfob_demo project.
 *
 *      BL51 linker directives for building this application:
 *
 *      BL51: PL(68) PW(78) IXREF RS(256)
 *            CODE (0x0-0x08FF)
 *            DATA (0x40)
 *            IDATA (0x80)
 *            XDATA (0x0900-0x107F)
 *            STACK (?STACK(0x8A))
 *
 *------------------------------------------------------------------------------
 *
 *    INCLUDES:
 */
#include <stdlib.h>
#include "si4010.h"
#include "si4010_api_rom.h"

/* Demo header */
#include "keyfob_demo.h"

/*------------------------------------------------------------------------------
 *
 *    FUNCTION DESCRIPTION:
 *      This function tunes the si4010 for transmission and transmits frames.
 *      It consists of two fundamental phases, setup and the transmission loop.
 *
 *      Setup Phase:
 *        The Setup Phase calls all setup function for the need API modules.
 *        These functions are to be called once per boot of the chip.
 *          vSys_Setup
 *          vPa_Setup
 *          vOds_Setup
 *          vStl_EncodeSetup
 *          vFCast_Setup
 *
 *      Transmission Loop Phase:
 *        The Transmission Loop Phase is a loop which transmits the proper frame
 *		  once a button is pushed. Frequency casting, FSK adjustment
 *        and PA tuning are all performed between each transmission.
 *
 *------------------------------------------------------------------------------
 */
void main(void)
{
/*------------------------------------------------------------------------------
 *    SETUP PHASE
 *------------------------------------------------------------------------------
 */
//Set DMD interrupt to high priority,
// any other interrupts have to stay low priority
  PDMD=1;

// Parameters Initialization. 
  vAppInitial();


// Call the system setup. This just for initialization.
// Argument of 1 just configures the SYS module such that the
// bandgap can be turned off if needed.
  vSys_Setup( 1 );

// Setup the bandgap for working with the temperature sensor here 
  vSys_BandGapLdo( 1 );
 
// Setup the BSR 
  vBsr_Setup( &rBsrSetup );

// Setup the RTC to tick every 5ms and clear it. Keep it disabled. 
  RTC_CTRL = (0x07 << B_RTC_DIV) | M_RTC_CLR;

// Enable the RTC 
  RTC_CTRL |= M_RTC_ENA;

// Enable the RTC interrupt and global interrupt enable 
  ERTC = 1;
  EA = 1;

// Setup the PA 
  vPa_Setup( &rPaSetup );

// ODS setup 
  vOds_Setup( &rOdsSetup );

// Setup the STL encoding to NoneNrz code. we
// leave the pointer at NULL. 
//  vStl_EncodeSetup( bEnc_Manchester_c, NULL );
  vStl_EncodeSetup(   bEnc_NoneNrz_c, NULL );


// Setup frequency casting .. needed to be called once per boot 
  vFCast_Setup();



/*------------------------------------------------------------------------------
 *    TRANSMISSION LOOP PHASE
 *------------------------------------------------------------------------------
 */

 // Application loop, including push button state analyzation and transmission.
  while(1)
  {
    // Buttons analyzation and update task flag. 
    vButtonCheck();

    if (bAp_ButtonState)
    {
      // Set frame size and frame header pointer 
      vPacketAssemble();
	  if (lPayLoad)
	  {
      // Start to transmit
      vRtl_RepeatTxLoop();
      }
    }
    else if( (lSys_GetMasterTime() >> 5) > bMaxWaitForPush_c )
    {
      // Shutdown if timeout, currently after about 1.6s,
      // For debugging with IDE, comment it out since it does not support boot.
      vSys_Shutdown();
    }
  }
}


/*------------------------------------------------------------------------------
 *
 *    FUNCTION DESCRIPTION:
 *      Initialization of Application parameters. 
 *
 *------------------------------------------------------------------------------
 */
 void vAppInitial(void)
{
// Disable the Matrix and Roff modes on GPIO[3:1] 
  PORT_CTRL &= ~(M_PORT_MATRIX | M_PORT_ROFF | M_PORT_STROBE);
  PORT_CTRL |=  M_PORT_STROBE;
  PORT_CTRL &= (~M_PORT_STROBE);

// Turn LED control off 
  GPIO_LED = 0;
// Set LED intensity .. acceptable values are 0 (off) or 1, 2, and 3 
  vSys_LedIntensity( 0 );

  // Initial debouce time. 
  bIsr_DebounceCount = 0;
  // debounce time definition, the unit is defined by the RTC interrupt time
  //(5ms in the demo).It means that the interval between calls to the
  // vBsr_Service() function is 5*8=40ms.
  // BSR parameters initialization 
  rBsrSetup.bButtonMask = 0x1F;	// GPIO0 only
  rBsrSetup.pbPtsReserveHead = abBsrPtsPlaceHolder;
  rBsrSetup.bPtsSize = 10;
  rBsrSetup.bPushQualThresh = 3;

// Setup the PA.
// fAlpha and fBeta has to be set based on antenna configuration.
// Chose a PA level and nominal cap. Both values come from
// the calculation spreadsheet. 
  rPaSetup.fAlpha      = 0.0;
  rPaSetup.fBeta       = 0.3593;
  rPaSetup.bLevel      = 0;
  rPaSetup.wNominalCap = 10;
  rPaSetup.bMaxDrv     = 0;

 // Setup the ODS 
  rOdsSetup.bModulationType = bRke_OokMod_c; // Use OOK 
//  rOdsSetup.bModulationType = bRke_FskMod_c; // Use FSK 
  rOdsSetup.bClkDiv         = 5;
  rOdsSetup.bEdgeRate       = 0;

// Set group width to 7, which means 8 bits/encoded byte to be transmitted.
// The value must match the output width of the data encoding function
// set by the vStl_EncodeSetup() below! 
  rOdsSetup.bGroupWidth     = 7;
// 24MHz / (bClkDiv+1) / 9.6kbps = 417 
  rOdsSetup.wBitRate        = 1600; // 1,25kbps

// Configure the warm up intervals LC: 8, DIV: 4, PA: 4 
  rOdsSetup.bLcWarmInt  = 8;
  rOdsSetup.bDivWarmInt = 5;
  rOdsSetup.bPaWarmInt  = 4;

// Set external crystal oscillator parameters - 10MHz, 12pF 
// currently XO is not used in this example
  rFCast_XoSetup.fXoFreq = 10000000;//12729633;
  rFCast_XoSetup.bLowCap = 1;

  return;
}

/*------------------------------------------------------------------------------
 *
 *    FUNCTION DESCRIPTION:
 *      This function contains the loop which consists of three procedures,
 *      tx setup, tx loop and tx shutdown in the application.
 *      During waiting for repeat transmission, check button state.
 *      Once any new push button is detected, then transmit the new packet 
 *      instead of the current packet.
 *
 *------------------------------------------------------------------------------ 
 */
void vRtl_RepeatTxLoop(void)

{ 

// Repeat packet 8 times for each button packet transmission. 
  bRepeatCount = bRepeatCount_c;


 
  do
  {
    // Setup the bandgap  
    vSys_BandGapLdo( 1 );

    // Save current timestamp for repeat transmission interval checking later. 
    lTimestamp = lSys_GetMasterTime();

    // Run at 433.92 MHz (frequency).Input is in [Hz]. 
	// The function also calculates necessary data to be used by the single Tx loop 
	// main function, vStl_SingleTxLoop(), for fine frequency correction during TX
    vFCast_Tune( fOperatingFrequency );

    // bFskDeviation comes from the calculation spreadsheet.
   // vFCast_FskAdj( bFskDeviation ); 	

    // Now wait until there is a temperature value  
    while ( 0 == bDmdTs_GetSamplesTaken() ){}

    // Tune the PA with the current temperature as an argument 
    vPa_Tune( iDmdTs_GetLatestTemp());

    // Sets up the temperature sensor and demodulator,
    // enable ODS before entering the single tx loop. 
    vStl_PreLoop();

    // Then assign the first address of the frame buffer to
    // the packet pointer used for transmission. 
    pbRke_FrameHead = abButtonFrame;

    // call the single tx loop function 
    vStl_SingleTxLoop(pbRke_FrameHead,
                      bFrameSize_c);

    // Disable the temperature sensor and ODS
    vStl_PostLoop();

    // Disable Bandgap and LDO 
     vSys_BandGapLdo(0);


     //turn LED on
	 GPIO_LED = 1; 
	       
    // Wait repeat interval. 
    while ( (lSys_GetMasterTime() - lTimestamp) < wRepeatInterval );

  	//turn LED off
 	GPIO_LED = 0;
  }while(--bRepeatCount);

  // After repeat transmission, clear all tasks flag. 
  bAp_ButtonState = 0;

  // Clear time value for next button push detecting. 
  vSys_SetMasterTime(0);
  return;
}
 
/*
 *------------------------------------------------------------------------------
 *
 *    FUNCTION DESCRIPTION:
 *      Update the master time by 5, every time this isr is run, and call the 
 * 		Button Service Routine
 *
 */

void isr_rtc (void) interrupt INTERRUPT_RTC using 1

{ 

  // clear the RTC_INT 
  RTC_CTRL &= ~M_RTC_INT;
  vSys_IncMasterTime(5); 
  bIsr_DebounceCount ++;
  if ((bIsr_DebounceCount % bDebounceInterval_c) == 0)
  {
    vBsr_Service();
  }
  return;
}

/*
 *------------------------------------------------------------------------------
 *
 *    FUNCTION DESCRIPTION:
 *      Update bAp_ButtonState which indicates what to be transmitted.
 *      Check the elements on PTS (push tracking strcture) to see if any GPIO
 *      has been pressed or released.
 *      If any new pressed button has detected, the corresponding flag will be set and
 *      the associated information will be transmitted in the application loop procedure.
 *
 *------------------------------------------------------------------------------ 
 */
void vButtonCheck( void)
{ 
  ERTC = 0;		// Disable RTC interrupt, otherwise button state might be corrupted.
  bAp_ButtonState = 0;
  if (bBsr_GetPtsItemCnt())
  {							// GPIO status have been changed.
							// Some buttons may have been pressed or released. 
	bAp_ButtonState = wBsr_Pop() & 0xFF;
	if (bPrevBtn)
	{
	bPrevBtn = bAp_ButtonState;
	bAp_ButtonState = 0;
	}
	else
	{
	bPrevBtn = bAp_ButtonState;
	}
  }
  ERTC = 1;                 // Enable RTC interrupt
  return;
}

/*------------------------------------------------------------------------------
 *
 *    FUNCTION DESCRIPTION:
 *      This function is used to update the packet for transmission,
 *      it is called after vAp_AnalyzeButton(), according to the
 *      the result of buttons analyzation to decide which packet
 *      to be transmited.
 *
 *------------------------------------------------------------------------------ 
 */
void vPacketAssemble ( void)
{
  /* Normal packet transmission mode, send the associated packet
   *
   * Note that Output Data Serializer (ODS) is in little endian fashion,
   * bit 0 first and inverted. So each byte of the packet data should be
   * converted properly before writing into frame buffer.
   */

 /*   switch (bAp_ButtonState)
    {
    case bButton1_c:
      lPayLoad = lFramePayLoad1_c;
      break;
    case bButton2_c:
      lPayLoad = lFramePayLoad2_c;
      break;
    case bButton3_c:
      lPayLoad = lFramePayLoad3_c;
      break;
    case bButton4_c:
      lPayLoad = lFramePayLoad4_c;
      break;
    case bButton5_c:
      lPayLoad = lFramePayLoad5_c;
      break;
    default:
      lPayLoad = 0;
      break;
    }
*/
    // Fill frame buffer with the corresponding button data.
    abButtonFrame[0] = 0x55;
    abButtonFrame[1] = 0x55;
    abButtonFrame[2] = 0x55;

    abButtonFrame[3] = 0x51;	// Tesla specific data
    abButtonFrame[4] = 0xd3;
    abButtonFrame[5] = 0x4c;
    abButtonFrame[6] = 0x33;
    abButtonFrame[7] = 0x33;
    abButtonFrame[8] = 0xd3;
    abButtonFrame[9] = 0xb2;
    abButtonFrame[10] = 0xb4;
    abButtonFrame[11] = 0x52;
    abButtonFrame[12] = 0xcb;
    abButtonFrame[13] = 0x32;
    abButtonFrame[14] = 0xd5;
    abButtonFrame[15] = 0xd2;

	abButtonFrame[16] = 0xA8;
	abButtonFrame[17] = 0x69;
	abButtonFrame[18] = 0xa6;
	abButtonFrame[19] = 0x99;
	abButtonFrame[20] = 0x99;
	abButtonFrame[21] = 0x69;
	abButtonFrame[22] = 0x59;
	abButtonFrame[23] = 0x5a;
	abButtonFrame[24] = 0xa9;
	abButtonFrame[25] = 0x65;
	abButtonFrame[26] = 0x99;
	abButtonFrame[27] = 0x6a;
	abButtonFrame[28] = 0x69;

	abButtonFrame[29] = 0xd4;
	abButtonFrame[30] = 0x34;
	abButtonFrame[31] = 0xd3;
	abButtonFrame[32] = 0xcc;
	abButtonFrame[33] = 0xcc;
	abButtonFrame[34] = 0xb4;
	abButtonFrame[35] = 0x2c;
	abButtonFrame[36] = 0xad;
	abButtonFrame[37] = 0xd4;
	abButtonFrame[38] = 0xb2;
	abButtonFrame[39] = 0x4c;
	abButtonFrame[40] = 0xb5;
	abButtonFrame[41] = 0x34;	




	return;
}