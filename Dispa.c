/****************************************************************************
	Copyright (C) 2010 Stefan Bormann, Olaf Funke, Heiko Herholz, Martin Pischky

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************

	IMPORTANT:

	Note: The sale any LocoNet device hardware (including bare PCB's) that
	uses this or any other LocoNet software, requires testing and certification
	by Digitrax Inc. and will be subject to a licensing agreement.

	Please contact Digitrax Inc. for details.

*****************************************************************************

	Title :   Main module of "Dispa" project.
	Author:   Heiko Herholz <heiko@modellbahnfrokler.de>
	Date:     04-September-2010
	Software: AVR-GCC
	Target:   AtMega168 (Prototype on "Watti", newer Hardware intended)

	DESCRIPTION
	This module implements the main loop, lcd access and loconet handling.
	"Dispa" is intended for setting the loco address and mode that is
	controlled by a simple loconet throttle as FRED, FREDI via the "dispatch"
	protocol of loconet.

*****************************************************************************/




#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "sysdef.h"


#include "common_defs.h"
#include "ln_interface.h"
#include "loconet.h"
//#include "systimer.h"
#include "Timer.h"

#include "lcd.h"
#include "i2cmaster.h"

#include "MasterReply.h"


typedef struct
{
	unsigned char ucSTAT;   // slot status
	unsigned char ucADR;    // least significant 7 bits or short address
	unsigned char ucSPD;    // Speed
	unsigned char ucDIRF;   // direction & lower functions
	unsigned char ucSS2;    // slot status 2
	unsigned char ucADR2;   // most significant 7 bit or zero for short address
	unsigned char ucSND;    // the good old "upper" function group
	unsigned char ucID1;
	unsigned char ucID2;
}
SSlotData_t;


const unsigned char SLOTMAX=28;
static SSlotData_t SlotTabelle[28];
static unsigned char trk =0x05;
static unsigned char dispslot;
LN_STATUS LnStatus ;
int zahl= 0;
int stelle = 0;
int temp, temp2;
int fahrstufen;

uint8_t debounce(volatile uint8_t *port, uint8_t pin)
{
    if ( ! (*port & (1 << pin)) )
    {
        /* Pin wurde auf Masse gezogen, 100ms warten   */
        _delay_ms(5);  // max. 262.1 ms / F_CPU in MHz
        _delay_ms(5); 
        if ( *port & (1 << pin) )
        {
            /* Anwender Zeit zum Loslassen des Tasters geben */
            _delay_ms(5);
            _delay_ms(5); 
            return 1;
        }
    }
    return 0;
}





static char mySendReply(lnMsg *pstMessage)
{
	performLocoNetBusy(0);  // switch off sendout of OPC_BUSY

	for (;;)
	{
		static LN_STATUS enReturn;
		enReturn = sendLocoNetPacketTry(pstMessage, LN_CARRIER_TICKS+1);
		switch (enReturn)
		{
			case LN_CD_BACKOFF:
			case LN_PRIO_BACKOFF:
				continue;  // still waiting to send master reply

			case LN_DONE:
				return 1;  // success

			default:
				return 0;  // error
		}
	}
}


char MasterReplyLAck(unsigned char ucRequestOpc, unsigned char ucParam)
{
	lnMsg stMessage;

	stMessage.lack.command = OPC_LONG_ACK;
	stMessage.lack.opcode  = ucRequestOpc & 0x7F;
	stMessage.lack.ack1    = ucParam      & 0x7F;
	return mySendReply(&stMessage);
}




int ret_slot(int adr, int adr2)
{
	int n;
	int m;
	m = 0;
	for (n=1; n < SLOTMAX;n++)
	{
		if ((SlotTabelle[n].ucADR == adr)&&(SlotTabelle[n].ucADR2 == adr2))
		{
			m =n;
			n = SLOTMAX;
		}
	}
	return m;
}

/*
int get_slot(int adr, int adr2){
	
		SlotTabelle[1].ucADR=adr;
		SlotTabelle[1].ucADR2=adr2;
	
	return 1;
}
*/

int get_slot(int adr, int adr2)
{
	int n;
	int m;
	m = 0;
	for (n=1; n < SLOTMAX;n++)
	{
		if (((!(SlotTabelle[n].ucSTAT &(1<<5))))&&(!(SlotTabelle[n].ucSTAT &(1<<4))))
		{		
			SlotTabelle[n].ucADR=adr;
			SlotTabelle[n].ucADR2=adr2;
			m = n;
			n = SLOTMAX;
		}
	}
	return m;
}

typedef struct 
{
	unsigned char stat1Val;
	char stringVal[4];
} StatTable_t;


#define COUNT 6
static const StatTable_t statTable[COUNT] =
{
	{ DEC_MODE_128A,  "128A" },
	{ DEC_MODE_28A,   "28A " },
	{ DEC_MODE_128,   "128" },
	{ DEC_MODE_28,    "28 " },
	{ DEC_MODE_14,    "14 " },
	{ DEC_MODE_28TRI, "M**" }
};

static const char * Stat1ToString( unsigned char s ) 
{
    for(int i = 0; i<COUNT; i++) 
	{
		if( ( s & DEC_MODE_MASK ) == statTable[ i ].stat1Val ) 
		{
			return statTable[ i ].stringVal;
		}
	}
 	return "?  ";
}


static void adr_lcd(int slot)
{
	lcd_clrxy(0,0,16);
	lcd_goto(0,0);
	lcd_puts("Alt: ");
	if (SlotTabelle[slot].ucADR2>0)
	{
		lcd_word((SlotTabelle[slot].ucADR2*128+SlotTabelle[slot].ucADR),4);
	}
	else 
		lcd_word(SlotTabelle[slot].ucADR,4);


	lcd_goto( 12,0 );
	lcd_puts( Stat1ToString( SlotTabelle[slot].ucSTAT ) );


//SlotTabelle[1].ucSTAT &=~((1<<5)|(1<<4));
//SlotTabelle[1].ucSTAT = 0x23;
//SlotTabelle[1].ucID1= 0x00;
//SlotTabelle[1].ucID2= 0x00;
}


static void Slot_SL_RD(int slot){

		
		
		lnMsg slread;
		slread.sd.command = 0xe7;
		slread.sd.mesg_size=0x0e;
		slread.sd.slot = slot;
		slread.sd.stat= SlotTabelle[slot].ucSTAT;
		slread.sd.adr= SlotTabelle[slot].ucADR;
		slread.sd.spd= SlotTabelle[slot].ucSPD;
		slread.sd.dirf= SlotTabelle[slot].ucDIRF;
		slread.sd.trk= 0x05; //trk;
		slread.sd.ss2= SlotTabelle[slot].ucSS2;
		slread.sd.adr2= SlotTabelle[slot].ucADR2;
		slread.sd.snd= SlotTabelle[slot].ucSND;
		slread.sd.id1= SlotTabelle[slot].ucID1;
		slread.sd.id2= SlotTabelle[slot].ucID2;
		
		//LnStatus = sendLocoNetPacket(&slread);
		mySendReply(&slread);
}




static void SlotManager(int adr, int adr2){

	int slot = 0;
	
	slot = ret_slot(adr, adr2);
	if (slot == 0) slot = get_slot(adr, adr2);
	if (slot > 0) Slot_SL_RD(slot);
	
	//Slot_SL_RD(1);
}


static void write_SL_Data(rwSlotDataMsg SL_Data){

	int slot = SL_Data.slot;
	SlotTabelle[slot].ucSTAT = SL_Data.stat;
	SlotTabelle[slot].ucADR = SL_Data.adr;
	SlotTabelle[slot].ucSPD = SL_Data.spd;
	SlotTabelle[slot].ucDIRF = SL_Data.dirf;
	SlotTabelle[slot].ucSS2 = SL_Data.ss2;
	SlotTabelle[slot].ucADR2 = SL_Data.adr2;
	SlotTabelle[slot].ucSND = SL_Data.snd;
	SlotTabelle[slot].ucID1 = SL_Data.id1;
	SlotTabelle[slot].ucID2 = SL_Data.id2;

	MasterReplyLAck(0x6f, 0x7f);
	//mySendReply(0xb4, 0x6F, 0x7f);
	//LnStatus = sendLocoNet4BytePacket(0xb4, 0x6F, 0x7f);
	adr_lcd(slot);
}


static void LedOn()
{
	lcd_goto(0,0);
	lcd_putc('E');
}

static void LedOff()
{
	lcd_goto(0,0);
	lcd_putc('A');
}





void HandleLoconetReception(void)
{
	// timer to recover from requests that we are not going to answer
	static unsigned long ulBusyWatchdog; 

	if (sendingLocoNetBusy())
	{
		switch (CheckTimer(&ulBusyWatchdog))
		{
			case 0:  // elapsed -> switch off OPC_BUSY
				performLocoNetBusy(0);
			//	LedOff();
				break;

			case -1: // not init -> trigger now
			//	LedOn();
				SetTimer(&ulBusyWatchdog, 300);
				break;
		}
	}
	else
	{
	//	LedOff();
		ResetTimer(&ulBusyWatchdog);  // not sending busy -> reset timer
	}

	lnMsg *pstLnPacket = recvLocoNetPacket();

	if (pstLnPacket)
	{
		switch (pstLnPacket->data[0])
		{
			case OPC_LOCO_ADR:
				SlotManager(pstLnPacket->data[2], pstLnPacket->data[1]);
//				ReceiveSlotOnLocoAddr(pstLnPacket->la.adr_lo, pstLnPacket->la.adr_hi);
				ResetTimer(&ulBusyWatchdog);  // should have answered
				break;

			case OPC_RQ_SL_DATA:
				Slot_SL_RD(pstLnPacket->data[1]);
//				ReceiveSlotOnRqSlData(pstLnPacket->sr.slot);
				ResetTimer(&ulBusyWatchdog);  // should have answered
				break;

			case OPC_MOVE_SLOTS:
				if (pstLnPacket->data[2]==pstLnPacket->data[1]) {
					if (pstLnPacket->data[1]!=0x00){
						SlotTabelle[pstLnPacket->data[1]].ucSTAT |=(1<<5)|(1<<4);
						Slot_SL_RD(pstLnPacket->data[1]);
					}
					else Slot_SL_RD(2);
				}
				if ((pstLnPacket->data[1]!=0x00)&&(pstLnPacket->data[2]==0x00)){
					dispslot = 	pstLnPacket->data[1];
					SlotTabelle[dispslot].ucSTAT &=~(1<<4);
					SlotTabelle[dispslot].ucSTAT |=(1<<5);
				}					
			
//				ReceiveSlotOnMoveSlots(pstLnPacket->sm.src, pstLnPacket->sm.dest);
				ResetTimer(&ulBusyWatchdog);  // should have answered
				break;

			case OPC_WR_SL_DATA:
				write_SL_Data(pstLnPacket->sd);
//				ReceiveSlotOnWrSlData(&pstLnPacket->sd);
				ResetTimer(&ulBusyWatchdog);  // should have answered
				break;

			case OPC_LONG_ACK:
			case OPC_SL_RD_DATA:
				break;  // ignore our own replies silently

			default:  // ignore packets that we don't want to handle
//				_printf_P(PSTR("master ignores %02X\r\n"), pstLnPacket->data[0]);
				break;
		}
	}
}



/*
static void HandleLoconetReception(void)
{
	lnMsg *LnPacket = recvLocoNetPacket();
		if( LnPacket->sd.command == OPC_LOCO_ADR){
		
	//	LnStatus = sendLocoNetPacket(OPC_BUSY);
	 
			SlotManager(LnPacket->data[2], LnPacket->data[1]);
			}

		if(LnPacket->sd.command == OPC_MOVE_SLOTS){
		
			if (LnPacket->data[2]==LnPacket->data[1]) {
				if (LnPacket->data[1]!=0x00){
					SlotTabelle[LnPacket->data[1]].ucSTAT |=(1<<5)|(1<<4);
					Slot_SL_RD(LnPacket->data[1]);
					}
				else {Slot_SL_RD(2);}
				}
			if ((LnPacket->data[1]!=0x00)&&(LnPacket->data[2]==0x00)){
			dispslot = 	LnPacket->data[1];
			SlotTabelle[dispslot].ucSTAT &=~(1<<4);
			SlotTabelle[dispslot].ucSTAT |=(1<<5);
				}					
			}

		if( LnPacket->sd.command == OPC_WR_SL_DATA){
			 write_SL_Data(LnPacket->sd);
			}

		if( LnPacket->sd.command == OPC_LOCO_SPD){
			 SlotTabelle[LnPacket->data[1]].ucSPD=LnPacket->data[2];
			 SlotTabelle[LnPacket->data[1]].ucSTAT |=(1<<5)|(1<<4);
			}

		if( LnPacket->sd.command == OPC_LOCO_DIRF){
			 SlotTabelle[LnPacket->data[1]].ucDIRF=LnPacket->data[2];
			 SlotTabelle[LnPacket->data[1]].ucSTAT |=(1<<5)|(1<<4);
			}

		if( LnPacket->sd.command == OPC_LOCO_SND){
			 SlotTabelle[LnPacket->data[1]].ucSND=LnPacket->data[2];
			 SlotTabelle[LnPacket->data[1]].ucSTAT |=(1<<5)|(1<<4);

			 
		}
		if( LnPacket->sd.command == OPC_RQ_SL_DATA){
			 Slot_SL_RD(LnPacket->data[1]);
			}
		if ( LnPacket->sd.command == OPC_SW_REQ){
			}
		if ( LnPacket->sd.command == OPC_SW_ACK){
				LnStatus = sendLocoNet4BytePacket(0xb4, 0x3d, 0x7f);
			}
		if ( LnPacket->sd.command == OPC_SW_STATE){
				}
		if ( LnPacket->sd.command == OPC_GPOFF){
				trk=0x06;  // Das ist der Track-Status bei IB. -> Chief testen
			
				}
		if ( LnPacket->sd.command == OPC_GPON){
				trk=0x07;
		
				}
		if ( LnPacket->sd.command == OPC_SLOT_STAT1){
				SlotTabelle[LnPacket->data[1]].ucSTAT=LnPacket->data[2];
				}
		if ( LnPacket->sd.command == OPC_IMM_PACKET){
				}
		if( LnPacket->sd.command == 0xA3){

			}
		if(( LnPacket->sd.command == 0xD4)&&(LnPacket->data[1] == 0x20)&&(LnPacket->data[3] == 0x08)){
			}
		if(( LnPacket->sd.command == 0xD4)&&(LnPacket->data[1] == 0x20)&&(LnPacket->data[3] == 0x09)){
			}
		if(( LnPacket->sd.command == 0xD4)&&(LnPacket->data[1] == 0x20)&&(LnPacket->data[3] == 0x05)){
			}
		if(( LnPacket->sd.command == 0xD4)&&(!(LnPacket->data[1] == 0x20))){
			}


} //eo HandleLoconetReception
*/

static void disp_put(){

		temp = zahl/128;
		temp2 = zahl-(temp*128);
		SlotTabelle[2].ucADR=temp2;
		SlotTabelle[2].ucADR2 =temp;
	//	SlotTabelle[2].ucSTAT = 0x33;
		SlotTabelle[2].ucSPD =0x1;
	//	lcd_goto(15,1);
	//	lcd_putc('D');


}



static void readzahl(unsigned int aktwert){

if (stelle == 0){
	zahl = aktwert;}
	else if (stelle<4){
		zahl=zahl*10;
		zahl=zahl+aktwert;
		}
	stelle++;
	lcd_clrxy(5,1,4);
	lcd_goto(5,1);
	lcd_word(zahl,4);
	disp_put();
}


/*
static void tastatur_lcd(char taste){
	//lcd_clrxy(0,0,16);
	lcd_goto(9,1);
	//lcd_puts("Adresse: ");
	//lcd_word(SlotTabelle[1].ucADR,4);
	lcd_putc(taste);
}
*/


int get_Tastatur(){

	char ret;
	if (debounce(&PINC, PC3)){
			return 99;
	}
	if (debounce(&PINC, PC2)){
			return 7;
	}
	if (debounce(&PINC, PC1)){
			return 4;
	}
	if (debounce(&PINC, PC0)){
			return 1;
	}
	if (debounce(&PINB, PB5)){
			return 0;
	}
	if (debounce(&PINB, PB4)){
			return 8;
	}
	if (debounce(&PINB, PB3)){
			return 5;
	}
	if (debounce(&PINB, PB2)){
			return 2;
	}
	if (debounce(&PINB, PB1)){
			return 98;
	}
	i2c_rep_start(0x70+I2C_READ);
	ret = i2c_readNak();                    
   
	
	if (!(ret&(1<<0))) {
		_delay_ms(300);
		ret = i2c_readNak();
 		i2c_stop();
		if ((ret&(1<<0))) return 9;
	}
	if (!(ret&(1<<1))) {
		_delay_ms(300);
		ret = i2c_readNak();
 		i2c_stop();
	 	if ((ret&(1<<1))) return 6;
		}
	if (!(ret&(1<<2))){
		_delay_ms(300);
		ret = i2c_readNak();
 		i2c_stop();
	 	if ((ret&(1<<2))) return 3;
		}
	return 66;
}

void fahrstufenCHG(){

fahrstufen++;
if (fahrstufen==COUNT) fahrstufen=0;
SlotTabelle[2].ucSTAT &= ~DEC_MODE_MASK;
SlotTabelle[2].ucSTAT |= statTable[fahrstufen].stat1Val;

lcd_goto( 12,1 ); //
lcd_puts( Stat1ToString( SlotTabelle[2].ucSTAT ) );
}

void handle_Tastatur(){
	int taste = get_Tastatur();
	if (taste == 99) {
		stelle = 0;
		zahl = 0;
		taste = 66;
		lcd_clrxy(5,1,7);
	}
	if (taste == 98) {
		taste = 66;
		fahrstufenCHG();

/*		if (SlotTabelle[2].ucSTAT == 0x33){
			SlotTabelle[2].ucSTAT = 0x30;
			}
		else {
			SlotTabelle[2].ucSTAT = 0x33;

		}
	*/
	
	/*	temp = zahl/128;
		temp2 = zahl-(temp*128);
		SlotTabelle[2].ucADR=temp2;
		SlotTabelle[2].ucADR2 =temp;
		SlotTabelle[2].ucSTAT = 0x33;
		SlotTabelle[2].ucSPD =0x1;
		lcd_goto(15,1);
		lcd_putc('D');*/
	}
	if (taste != 66) readzahl(taste);
}


int main (void){


DDRD = 0xff;
	DDRB = 0x00;
	DDRC = 0x00;
PORTB = 0xff;
PORTC = 0xff;

i2c_init();
//i2c_start_wait(0x70+I2C_WRITE);
//i2c_write(0xff);
//i2c_stop();

//InitLCD();
//SetDataLCD('a');



int lauf;
for (lauf = 0; lauf<SLOTMAX;lauf++)
{
SlotTabelle[lauf].ucSTAT=0x00;
}

static LnBuf LnBuffer;
trk=0x07;
SlotTabelle[2].ucSTAT = 0x33;
sei();

//---- init LocoNet
	initLnBuf( &LnBuffer ) ;
	initLocoNet( &LnBuffer ) ;
sbi(LN_TX_DDR, LN_TX_BIT);
//	initTimer();				/* Uses Timer0	*/
//	HwTimerInit();    // slave timer proxy for Timer.c


lcd_init(LCD_DISP_ON);
//lcd_putc('a');

lcd_puts("Dispa 1.0");
lcd_goto(0,1);
lcd_puts("(c)Heiko Herholz");
_delay_ms(2000);
lcd_clear();
lcd_goto(0,1);
lcd_puts("Neu: ");
fahrstufen=1;
fahrstufenCHG();

for (;;){

	HandleLoconetReception();
	handle_Tastatur();
	}
} //eo main
