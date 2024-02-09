/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "timer.h"
#include "../gioco.h"
#include "../drawing.h"
#include "../GLCD/GLCD.h"
#include "../RIT/RIT.h"
#include <stdio.h>


/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

/* External variables ---------------------------------------------------------*/
extern int secondi;
extern int turno;
extern int posizione_player2[2];
extern int posizione_player1[2];
extern int mosso;
extern int wall;
extern int ruotato;
extern int posizione_wall[2];
extern uint8_t cancella_warning;
extern uint8_t opposite_player;
extern int game_mode;
extern uint8_t modalita_player;
extern uint8_t ospite;

void TIMER0_IRQHandler (void)
{
	LCD_Clear(Black);
	LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	disable_RIT();
	GUI_Text(2, 150, (uint8_t *) "ERROR CANNOT FIND SECOND BOARD", Red, Black);
  LPC_TIM0->IR = 1;			/* clear interrupt flag */
  return;
}


/******************************************************************************
** Function name:		Timer1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER1_IRQHandler(void) {
    char str[3];

    if (secondi != 0 && mosso != 1) {
        secondi--;
    } else {
        if (turno) { // finisce il turno player1
            colora_caselle_vicine(posizione_player1, White, turno);
						if(!cancella_warning) {
								GUI_Text(70, 240, (uint8_t *)"TURNO: PLAYER2", Red, White);
						}
						else{
							  GUI_Text(6, 240, (uint8_t *)"No walls left, move the token", White, White);
								GUI_Text(70, 240, (uint8_t *)"TURNO: PLAYER2", Red, White);
							cancella_warning=0;

						}
            turno = 0;
            if((opposite_player==1 && game_mode==1) || (game_mode==2 && ospite==1 && modalita_player==1)) colora_caselle_vicine(posizione_player2, Yellow, turno);

            if (!mosso && !ruotato) { // non ha fatto nessuna mossa
                controlla_presenza_orizzontale();
            } else if (!mosso && ruotato) { // non ha fatto nessuna mossa
                controlla_presenza_verticale();
            }
        } else { // finisce il turno player2
            colora_caselle_vicine(posizione_player2, White, turno);
            if(!cancella_warning) {
								GUI_Text(70, 240, (uint8_t *)"TURNO: PLAYER1", Black, White);
						}
						else{
							  GUI_Text(6, 240, (uint8_t *)"No walls left, move the token", White, White);
								GUI_Text(70, 240, (uint8_t *)"TURNO: PLAYER1", Black, White);
								cancella_warning=0; 

						}
            turno = 1;
              if(game_mode==1 || (modalita_player==1 && game_mode==2 && ospite==0) ) colora_caselle_vicine(posizione_player1, Yellow, turno);

            if (!mosso && wall && !ruotato) { // se non ha fatto nessuna mossa e stava posizionando il muro devo toglierlo
                controlla_presenza_orizzontale(); // orizzontale
            } else if (!mosso && wall && ruotato) { 
                controlla_presenza_verticale();  // verticale
            }
						
						//se non ha fatto nulla devo registrare la mossa qui
						if(!mosso) registra_mossa(turno,0,1,0,0); //ha perso il turno

        }
        secondi = 20;
        mosso = 0;
        wall = 0;
        ruotato = 0;
        posizione_wall[0] = 2;
        posizione_wall[1] = 3;
    }
    if (secondi >= 10) {
        sprintf(str, "%d", secondi);
        GUI_Text(103, 275, (uint8_t *)str, Black, White);
    } else {
        sprintf(str, " %d", secondi);
        GUI_Text(103, 275, (uint8_t *)str, Black, White);
    }

    LPC_TIM1->IR = 1; /* clear interrupt flag */
    return;
}


/******************************************************************************
**                            End Of File
******************************************************************************/
