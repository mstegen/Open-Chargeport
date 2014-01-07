;******************************************************************************
;   Open Chargeport                         						          *
;   For use with Tesla Model S                                                *
;******************************************************************************
;                                                                             *
;    Filename:     OpenChargeport.asm                                         *
;    Date:         10-9-13                                                    *
;    File Version: 1.01                                                       *
;                                                                             *
;    Author:       M. Stegen                                                  *
;    Company:      Stegen Electronics                                         *
;                                                                             *
;                                                                             *
;******************************************************************************
;																			  *
;******************************************************************************
;                                                                             *
;    Revision History:                                                        *
;	 1.00 		initial release												  *
;	 1.01		version for PIC 10F200/202/204/206							  *
;																			  *
;******************************************************************************
;
;		PIC 10F200/202/204/206
;
;	PDIP pinout:
;	       +----_----+
;	 NC  --| 1     8 |-- NC 
;	 VCC --| 2     7 |-- GND
;	 NC  --| 3     6 |-- NC
;	 NC  --| 4     5 |-- TX out
;	       +---------+
;
;	SOT23 pinout:
;	       +------+
;  TXout --|*1  6 |-- NC
;    GND --| 2  5 |-- VCC
;     NC --| 3  4 |-- NC
;          +------+
;
;*****************************************************************************



	list		p=10F204      ; list directive to define processor
	#include	<p10F204.inc> ; processor specific variable definitions

;------------------------------------------------------------------------------
;
; CONFIGURATION WORD SETUP
;
; The 'CONFIG' directive is used to embed the configuration word within the 
; .asm file. The lables following the directive are located in the respective 
; .inc file.  See the data sheet for additional information on configuration 
; word settings.
;
;------------------------------------------------------------------------------    

    __CONFIG _WDT_OFF & _MCLRE_OFF & _CP_OFF

;------------------------------------------------------------------------------
; RAM-Variables


Mcount      EQU    	0x10
Count		EQU		0x11
Serdata		EQU		0x12

TX			EQU		0		; TX output

;------------------------------------------------------------------------------
; RESET VECTOR
;------------------------------------------------------------------------------

    ORG     0x0000            ; processor reset vector

	goto    INIT              ; When using debug header, first inst.
                              ; may be passed over by ICD2.  

;-------------------------------------------------------------------------
; Send 3 bits (in W) (MSB)

MC_TX3
		movwf	Serdata
		MOVLW 	3
		MOVWF 	Count			; 3 BITS TO send
		goto	MC_TXLOOP

;-------------------------------------------------------------------------
; Send one byte (in W)
MC_TX
		movwf	Serdata
		MOVLW 	8
		MOVWF 	Count			; 8 BITS TO send
		
MC_TXLOOP
		CLRF	TMR0

		BTFSS	Serdata,7		; check MSB
		BCF		GPIO,TX			; Signal LOW 	=> 01
		BTFSC	Serdata,7		; check MSB
		BSF		GPIO,TX			; Signal HIGH 	=> 10
MC1_400us
		MOVF	TMR0,W
		XORLW	.25				; 0.000016 * 25= 400 uS
		BTFSS	STATUS,Z
		GOTO	MC1_400us

		BTFSC	Serdata,7		; check MSB
		BCF		GPIO,TX			; Signal LOW	=> 01
		BTFSS	Serdata,7		; check MSB
		BSF		GPIO,TX			; Signal HIGH	=> 10

MC2_400us
		MOVF	TMR0,W
		XORLW	.50				; 0.000016 * 50= 800 uS total
		BTFSS	STATUS,Z
		GOTO	MC2_400us

		RLF		Serdata,F
		decfsz	Count,F
		goto	MC_TXLOOP	
		retlw 	0x00


;------------------------------------------------------------------------------
;
SEND_SYNC

		movlw	.13
		movwf	Count
sync_loop
		CLRF	TMR0
		BSF		GPIO,TX			; High
high_400
		MOVF	TMR0,W
		XORLW	.25				; 0.000016 * 25= 400us
		BTFSS	STATUS,Z
		GOTO	high_400

		BCF		GPIO,TX			; Low
low_400
		MOVF	TMR0,W
		XORLW	.50				; 0.000016 * 50= 800us
		BTFSS	STATUS,Z
		GOTO	low_400
		decfsz	Count,F			; repeat
		goto	sync_loop
		retlw 	0x00

;------------------------------------------------------------------------------
;
delay400us
		CLRF	TMR0
dly400us
		MOVF	TMR0,W
		XORLW	.25				; 0.000016 * 25= 400 uS
		BTFSS	STATUS,Z
		GOTO	dly400us
		retlw 	0x00

delay25ms
		movlw	.62
		movwf	Count
dly25ms		
		call	delay400us
		decfsz	Count,F
		goto	dly25ms
		retlw 	0x00
	

;------------------------------------------------------------------------------
; INITIALIZE all REGISTERS
;------------------------------------------------------------------------------

INIT	

		movwf	OSCCAL			; calibrate internal oscillator
		movlw	B'11000011'
		OPTION					; prescaler 1:16, Weak pull-ups disabled

		CLRF	CMCON0			; disable comperator (PIC10F204/206 only)
		CLRF 	GPIO 			; Init GPIO
		movlw 	B'00001110'		; set I/O
		TRIS	GPIO

MAIN
		call	SEND_SYNC		; send 13 sync bits
		movlw	.3
		movwf	Mcount
Mloop
		call	delay400us
		movlw	0x12			
		call	MC_TX
		movlw	0x95			
		call	MC_TX
		movlw	0x53			
		call	MC_TX
		movlw	0x67			
		call	MC_TX
		movlw	0x1B			
		call	MC_TX
		movlw	0x43			
		call	MC_TX
		movlw	0x20			
		call	MC_TX3
		decfsz	Mcount,F
		goto	Mloop

		call	delay25ms

		goto 	MAIN			; loop forever
    


    END