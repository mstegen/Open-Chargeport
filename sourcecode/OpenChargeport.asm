;******************************************************************************
;   Open Chargeport                         						          *
;   For use with Tesla Model S                                                *
;******************************************************************************
;                                                                             *
;    Filename:     OpenChargeport.asm                                         *
;    Date:         4-9-13                                                    *
;    File Version: 1.00                                                       *
;                                                                             *
;    Author:       M. Stegen                                                  *
;    Company:      Stegen Electronics                                         *
;                                                                             *
;                                                                             *
;******************************************************************************
;                                                                             *
;    *
;    *
;	 
;																			  *
;******************************************************************************
;                                                                             *
;    Revision History:                                                        *
;	 1.00 		initial release												  *
;																			  *
;******************************************************************************
;
;
;
;	       +----_----+
;	 VDD --| 1     8 |-- GND 
;	 NC  --| 2     7 |-- TX Out
;	 NC  --| 3     6 |-- NC
;	 NC  --| 4     5 |-- NC
;	       +---------+
;
;
;*****************************************************************************



	list		p=12F1822      ; list directive to define processor
	#include	<p12F1822.inc> ; processor specific variable definitions

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

    __CONFIG _CONFIG1, _FOSC_INTOSC & _WDTE_OFF & _PWRTE_ON & _MCLRE_OFF & _CP_ON & _CPD_ON& _BOREN_OFF & _CLKOUTEN_OFF & _IESO_OFF & _FCMEN_OFF
    __CONFIG _CONFIG2, _WRT_OFF & _PLLEN_OFF & _STVREN_OFF & _BORV_19 & _LVP_OFF



;------------------------------------------------------------------------------
; VARIABLE DEFINITIONS
;
; Available Data Memory divided into Bank 0-15.  Each Bank may contain 
; Special Function Registers, General Purpose Registers, and Access RAM 
;
;------------------------------------------------------------------------------

; RAM-Variables


Mcount      EQU    	0x71
Count		EQU		0x72
Serdata		EQU		0x73

TX			EQU		0		; pin 7	TX output

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
		BCF		PORTA,TX		; Signal LOW 	=> 01
		BTFSC	Serdata,7		; check MSB
		BSF		PORTA,TX		; Signal HIGH 	=> 10
MC1_400us
		MOVF	TMR0,W
		XORLW	.25				; 0.000016 * 25= 400 uS
		BTFSS	STATUS,Z
		GOTO	MC1_400us

		BTFSC	Serdata,7		; check MSB
		BCF		PORTA,TX		; Signal LOW	=> 01
		BTFSS	Serdata,7		; check MSB
		BSF		PORTA,TX		; Signal HIGH	=> 10

MC2_400us
		MOVF	TMR0,W
		XORLW	.50				; 0.000016 * 50= 800 uS total
		BTFSS	STATUS,Z
		GOTO	MC2_400us

		RLF		Serdata,F
		decfsz	Count,F
		goto	MC_TXLOOP	
		return


;------------------------------------------------------------------------------
;
SEND_SYNC
		BANKSEL PORTA

		movlw	.13
		movwf	Count
sync_loop
		CLRF	TMR0
		BSF		PORTA,TX		; High
high_400
		MOVF	TMR0,W
		XORLW	.25				; 0.000016 * 25= 400us
		BTFSS	STATUS,Z
		GOTO	high_400

		BCF		PORTA,TX		; Low
low_400
		MOVF	TMR0,W
		XORLW	.50				; 0.000016 * 50= 800us
		BTFSS	STATUS,Z
		GOTO	low_400
		decfsz	Count,F			; repeat
		goto	sync_loop
		return	

;------------------------------------------------------------------------------
;
delay400us
		CLRF	TMR0
dly400us
		MOVF	TMR0,W
		XORLW	.25				; 0.000016 * 25= 400 uS
		BTFSS	STATUS,Z
		GOTO	dly400us
		return

delay25ms
		movlw	.62
		movwf	Count
dly25ms		
		call	delay400us
		decfsz	Count,F
		goto	dly25ms
		return
	

;------------------------------------------------------------------------------
; INITIALIZE all REGISTERS
;------------------------------------------------------------------------------

INIT	
		BANKSEL OPTION_REG
		movlw	B'11000110'
		movwf	OPTION_REG		; prescaler 1:128, Weak pull-ups disabled
								; same bank
		movlw	b'11110000'		
		movwf	OSCCON			; setup 32Mhz internal oscillator

		BANKSEL PORTA 
		CLRF 	PORTA 			; Init PORTA
		BANKSEL LATA 			; Data Latch
		CLRF 	LATA 
		BANKSEL ANSELA 
		CLRF 	ANSELA 			; digital I/O
		BANKSEL TRISA 		
		MOVLW 	B'00111110' 	; set pin 7 as output (TX)
		MOVWF 	TRISA 			; the rest as input



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