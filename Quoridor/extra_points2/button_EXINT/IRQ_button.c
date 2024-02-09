#include "button.h"
#include "lpc17xx.h"
#include "../timer/timer.h"
#include "../RIT/RIT.h"
#include "../GLCD/GLCD.h"
#include "../drawing.h"
#include "../gioco.h"


/* Variables ---------------------------------------------------------*/
int p1_wall; //numero di muri rimanenti
int p2_wall;
int secondi = 20;
int turno = 1;  //=1 player1, =0 player2 
int posizione_player1[2]={6,3};  //la posizione di una giocatore è definita da riga e colonna
int posizione_player2[2]={0,3};
int wall=0; //=0 il giocatore sta muovendo la pedina, =1 il giocatore sta posizionando un muro
int ruotato=0; //=0 il muo che sta posizionando è orizzontale, =1 il muro che sta posizionando è verticale
uint8_t game_mode=0; //=0 ancora non scelto =1 scelto single board =2 scelto two boards
extern int down_key1;
extern int down_key0;
extern int down_key2;

void EINT0_IRQHandler(void) {
	NVIC_DisableIRQ(EINT0_IRQn);
	LPC_PINCON->PINSEL4	&=~(1 << 20);
	down_key0=1;
  LPC_SC->EXTINT &= (1 << 0); /* clear pending interrupt */
}



void EINT1_IRQHandler(void) { /* KEY1 */
   NVIC_DisableIRQ(EINT1_IRQn);
	 LPC_PINCON->PINSEL4	&=~(1 << 22);
	 down_key1=1;
   LPC_SC->EXTINT &= (1 << 1); /* clear pending interrupt */
}


void EINT2_IRQHandler(void) { /* KEY2 */
	 NVIC_DisableIRQ(EINT2_IRQn);
	 LPC_PINCON->PINSEL4	&=~(1 << 24);
	 down_key2=1;
   LPC_SC->EXTINT &= (1 << 2); /* clear pending interrupt */
}


