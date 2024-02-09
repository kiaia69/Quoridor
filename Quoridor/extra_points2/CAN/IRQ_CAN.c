/*----------------------------------------------------------------------------
 * Name:    Can.c
 * Purpose: CAN interface for for LPC17xx with MCB1700
 * Note(s): see also http://www.port.de/engl/canprod/sv_req_form.html
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <lpc17xx.h>                  /* LPC17xx definitions */
#include "CAN.h"                      /* LPC17xx CAN adaption layer */
#include "../GLCD/GLCD.h"
#include "../gioco.h"
#include "../timer/timer.h"
#include "../drawing.h"

extern uint8_t icr ; 										//icr and result must be global in order to work with both real and simulated landtiger.
extern uint32_t result;
extern CAN_msg       CAN_TxMsg;    /* CAN message for sending */
extern CAN_msg       CAN_RxMsg;    /* CAN message for receiving */  
extern int game_mode;
extern uint8_t opposite_player;
extern uint8_t first;

static volatile uint8_t handshake_ricevuto=0;
volatile uint8_t ospite=0; //mi serve per capire chi è stato a premere int0 per primo
static uint32_t data=0;
volatile uint8_t modalita_player=0; //=1 human, =2 npc
uint8_t player1_pronto=0;
uint8_t player2_pronto=0;


/*----------------------------------------------------------------------------
  CAN interrupt handler
 *----------------------------------------------------------------------------*/
void CAN_IRQHandler (void)  {

  /* check CAN controller 1 */
	icr = 0;
	icr = (LPC_CAN1->ICR | icr) & 0xFF;             /* clear interrupts */

	if (icr & (1 << 0)) {                          	/* CAN Controller #2 meassage is received */
		CAN_rdMsg (1, &CAN_RxMsg);	                		/* Read the message */
    LPC_CAN1->CMR = (1 << 2);                    		/* Release receive buffer */
		
		if(handshake_ricevuto && game_mode==2){ //stiamo giocando
				data = 	(CAN_RxMsg.data[0] << 24) |
                (CAN_RxMsg.data[1] << 16) |
                (CAN_RxMsg.data[2] << 8)  |
                CAN_RxMsg.data[3];
				
				effettua_mossa(data);
		}
		else{ //fase di handshake
			data=CAN_RxMsg.data[0]; 
			if(data == 0xFF){ //io sono il player2
				game_mode=2;
				ospite=1;
				NVIC_DisableIRQ(EINT0_IRQn);
				//mando risposta
				CAN_TxMsg.data[0] = 0x0F; 
				CAN_TxMsg.len = 1;
				CAN_TxMsg.id = ospite+1;
				CAN_TxMsg.format = STANDARD_FORMAT;
				CAN_TxMsg.type = DATA_FRAME;
				CAN_wrMsg (1, &CAN_TxMsg);               /* transmit message */
				menu_opposite_player();
			}
			else if(data == 0x0F){ //ricevuto risposta dopo che sono stato io a cliccare int0
				ospite=0; //sono il player 1
				//handshake_ricevuto=1;		
				disable_timer(0);
			}
			else{
				if(data == 0x01) {
					modalita_player=1;
					first=0;
				}
				if(data == 0x02) {
					modalita_player=2;
					first=0;
				}
				game_mode=2;
				handshake_ricevuto=1;
				if(data == 0xA){ 
					modalita_player=1;
					inizia_partita(game_mode,opposite_player);
					handshake_ricevuto=1;
				}
				if(data == 0xB){ 
					modalita_player=2;
					inizia_partita(game_mode,opposite_player);
					handshake_ricevuto=1;
				}
			}
			}
		}
		
	
	if (icr & (1 << 1)) {                         /* CAN Controller #2 meassage is transmitted */
		// do nothing in this example
	}
}
