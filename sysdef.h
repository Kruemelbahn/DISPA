#ifndef _SYSDEF_H_
#define _SYSDEF_H_


#define BOARD_DEFINED_IN_SYSDEF



	/* Hardware UART */
	

	
	//--------------- configuration for ln_swtx_hwrx_uart.c --------------------------
	// using UART 0, Timer 1 with ouput capture A
	
	#define LN_RX_PORT   PIND   // PD0 = RXD
	#define LN_RX_BIT    PD0
	
	#define LN_TX_PORT   PORTD  // PD1 = TXD
	#define LN_TX_DDR    DDRD
	#define LN_TX_BIT    PD1
	
	#define LN_PCI_NO    16  // PD0 = RXD = PCINT16		//Original-Watti



//Original-Watti	
	#define LN_SW_UART_SET_TX_LOW  cbi(LN_TX_PORT, LN_TX_BIT);  // to pull down LN line to drive low level
	#define LN_SW_UART_SET_TX_HIGH sbi(LN_TX_PORT, LN_TX_BIT);  // master pull up will take care of high LN level






	#define LN_SB_SIGNAL          TIMER1_CAPT_vect
	#define LN_SB_INT_ENABLE_REG  TIMSK1
	#define LN_SB_INT_ENABLE_BIT  ICIE1
	#define LN_SB_INT_STATUS_REG  TIFR1
	#define LN_SB_INT_STATUS_BIT  ICF1
	
	#define LN_TMR_SIGNAL         TIMER1_COMPA_vect
	#define LN_TMR_INT_ENABLE_REG TIMSK1
	#define LN_TMR_INT_ENABLE_BIT OCIE1A
	#define LN_TMR_INT_STATUS_REG TIFR1
	#define LN_TMR_INT_STATUS_BIT OCF1A
	#define LN_TMR_OUTP_CAPT_REG  OCR1A
	#define LN_TMR_COUNT_REG      TCNT1
	#define LN_TMR_CONTROL_REG    TCCR1B
	
	#define LN_TMR_PRESCALER      1
	
	#define LN_HW_UART_CONTROL_REGA UCSR0A
	#define LN_HW_UART_CONTROL_REGB UCSR0B
	#define LN_HW_UART_BAUDRATE_REG UBRR0L
	#define LN_HW_UART_RX_SIGNAL    SIG_USART_RECV
	//#define LN_HW_UART_TX_SIGNAL    SIG_UART_DATA
	#define LN_HW_UART_DATA_REG     UDR0
	
	#define LN_SW_UART_TX_NON_INVERTED  //Für Original Watti-Hardware
	#define LOCONET_MASTER



//--------------------------- HwTimer --------------------------------------


#define TIMER_RESOLUTION_MS 1   // 1ms resolution for Timer.c, systimer.c implements it anyway
#define USE_TIMER_0

//----------------------------- misc ----------------------------------------


#define SERVO_COUNT 4

#define SV_MAX_NUMBER 499   // TBC count of SV space

//----------------------------- Identification for SV and IPL ---------------

// moved to makefile, because it is needed for software update there
// information is also copied to project files for bootloader and
// application (latter is still needed for debug sessions)

//----------------------------- legacy definitions --------------------------


#define BV(bit) _BV(bit)
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))


//#define F_CPU 					 11059200
//#define F_CPU            14745600
//#define F_CPU 8000000


#define OK		0
#define ERR		-1
#define true	1
#define false	0


#endif


#define LCD_DATA_PORT    PORTD     /* port for Databits */
#define LCD_DATA_DDR     DDRD      /* port for Databits */

#define LCD_DATA_D4_PIN  4
#define LCD_DATA_D5_PIN  5
#define LCD_DATA_D6_PIN  6
#define LCD_DATA_D7_PIN  7

#define LCD_RS_PORT      PORTD     /* port for RS line */
#define LCD_RS_DDR       DDRD      /* port for RS line */
#define LCD_RS_PIN       2

#define LCD_E_PORT       PORTD  	 /* port for Enable line */
#define LCD_E_DDR        DDRD      /* port for Enable line */
#define LCD_E_PIN        3



// from makefile
#define SOFTWARE_VERSION 1   // this should be changed with the releases!!!
#define HARDWARE_VERSION 1   // not used at this time
#define MANUFACTURER_ID  0x0d   // DIY-ID
#define DEVELOPER_ID     8   // Heiko Herholz
#define PRODUCT_ID       2   // this device  

