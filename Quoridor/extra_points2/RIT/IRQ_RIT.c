/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "RIT.h"
#include "../led/led.h"
#include "../GLCD/GLCD.h"
#include "../gioco.h"
#include "../drawing.h"
#include "../timer/timer.h"
#include "../CAN/CAN.h"
#include <stdio.h>

/* External variables ---------------------------------------------------------*/
extern int wall;
extern int turno;
extern int ruotato;
extern int posizione_player2[2];
extern int posizione_player1[2];
extern int p1_wall;
extern int p2_wall;
extern int vittoria;
extern int game_mode;
extern MossaNPC mossa__npc;
extern uint8_t ospite;
extern uint8_t modalita_player;

/* Variables ---------------------------------------------------------*/
PosizioneMuro muri_definitivi[16];
int counter_muri=0; //indice della variabile precedente
int mosso=0; //=1 
int posizione_wall[2]={2,3}; //posizione iniziale (2,3)
int movimento_in_diagonale=0; //=1 quando il giocatore prova ad andare in digonale
uint8_t opposite_player=0; //=0 non selezionato =1 Human =2 NPC
uint8_t cancella_warning=0; //=1 un giocatore ha finito i muri ed è necessario cnacellare il warning, 0 altrimenti
volatile int down_key1=0;
volatile int joystick_premuto=0;
volatile int joystick_sinistra=0;
volatile int down_key2=0;
volatile int down_key0=0;
volatile uint8_t pronti=0;
volatile uint8_t first=1;

/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
	
void RIT_IRQHandler (void)
{				
	static int up=0;
	static int down=0;
	static int right=0;
	static int left=0;
	static int diagonal_up_left=0;
	static int diagonal_up_right=0;
	static int diagonal_down_right=0;
	static int diagonal_down_left=0;

	
	
	char str[3];

	if(!game_mode){ //contemplate solo le messo verso alto o basso del joystick quando sceglie nel menu
		
		static int s_t=0; //=0 la freccia sta su single board =1 altrimenti
		
		if ((LPC_GPIO1->FIOPIN & (1 << 29)) == 0 && s_t){ //va in alto per selezionare single board
			
					GUI_Text(20, 190, (uint8_t *)"-->", White, White);
					GUI_Text(20, 130, (uint8_t *)"-->", Black, White);
					s_t=0;
					joystick_premuto=0;
		}
		
		else if((LPC_GPIO1->FIOPIN & (1 << 26)) == 0 && !s_t){ //va in basso per selezionare two boards

					GUI_Text(20, 130, (uint8_t *)"-->", White, White);
					GUI_Text(20, 190, (uint8_t *)"-->", Black, White);
					s_t=1;
					joystick_premuto=0;
		
		}
		else if((LPC_GPIO1->FIOPIN & (1 << 25)) == 0){ //premuto per selezionare definitivamente
			joystick_premuto++;
			switch(joystick_premuto){
				case 1:
					if(!s_t) game_mode=1; //single board
					else {
						game_mode=2; //two boards 
						CAN_TxMsg.data[0] = 0xFF; 
						CAN_TxMsg.len = 1;
						CAN_TxMsg.id = 1;
						CAN_TxMsg.format = STANDARD_FORMAT;
						CAN_TxMsg.type = DATA_FRAME;
						CAN_wrMsg (1, &CAN_TxMsg);               /* transmit message */
						init_timer(0, 0x05F5E100); //aspetto per 4 secondi la risposta altrimenti errore
						enable_timer(0);	
					}
					menu_opposite_player();
					break;
				default: break;
			}
		}
		else{
					joystick_premuto=0;
		}
			
	}
	else if(!opposite_player){  //contemplate solo le messo verso alto o basso del joystick quando sceglie nel menu

		static int h_n=0; //=0 la freccia sta su human =1 altrimenti
		
		if ((LPC_GPIO1->FIOPIN & (1 << 29)) == 0 && h_n){ //va in alto per selezionare single board
			
					GUI_Text(20, 190, (uint8_t *)"-->", White, White);
					GUI_Text(20, 130, (uint8_t *)"-->", Black, White);
					h_n=0;
					joystick_premuto=0;
		}
		
		else if((LPC_GPIO1->FIOPIN & (1 << 26)) == 0 && !h_n){ //va in basso per selezionare two boards
			
					GUI_Text(20, 130, (uint8_t *)"-->", White, White);
					GUI_Text(20, 190, (uint8_t *)"-->", Black, White);
					h_n=1;
					joystick_premuto=0;
		}
		else if((LPC_GPIO1->FIOPIN & (1 << 25)) == 0){ //premuto per selezionare definitivamente
			joystick_premuto++;			
			switch(joystick_premuto){
				case 1:
					if(!h_n) opposite_player=1; //human
					else opposite_player=2; //npc
					if(game_mode==2){//multiboard
					if(first){ //sono il primo che sceglie nel menu
					CAN_TxMsg.data[0] =(opposite_player == 1) ? 0x01 : 0x02;  
					CAN_TxMsg.len = 1;
					CAN_TxMsg.id = 1+ospite;
					CAN_TxMsg.format = STANDARD_FORMAT;
					CAN_TxMsg.type = DATA_FRAME;
					CAN_wrMsg (1, &CAN_TxMsg);               /* transmit message */
					LCD_Clear(White);
					GUI_Text(10, 150, (uint8_t *) "WAITING FOR THE OTHER PLAYER", Black, White);
					}
					else{
					CAN_TxMsg.data[0] =(opposite_player == 1) ? 0xA : 0xB; 
					CAN_TxMsg.len = 1;
					CAN_TxMsg.id = 1+ospite;
					CAN_TxMsg.format = STANDARD_FORMAT;
					CAN_TxMsg.type = DATA_FRAME;
					CAN_wrMsg (1, &CAN_TxMsg);               /* transmit message */
					inizia_partita(game_mode, opposite_player);
					}
					}
					else inizia_partita(game_mode, opposite_player);

					break;
				default: break;
			}
		}
		else{
					joystick_premuto=0;

		}
	}
	else{  //sto giocando  
		if((game_mode==2 && modalita_player==1 && turno!=ospite) || (game_mode==1 && opposite_player==1 && turno==0) || (game_mode==1 && turno==1)){ //tocca ad un umano

			if(wall==0){ //non è stato cliccto il bottone per mettere il muro
	 // Verifica stato del joystick
	 // Prima controllo eventuali movimenti in diagonali
	
		 if ((LPC_GPIO1->FIOPIN & (1 << 29)) == 0 && (LPC_GPIO1->FIOPIN & (1 << 27)) == 0 ) { //in alto a sinistra
			 if(turno){
					if(!controlla_casella_giocatore(posizione_player2, posizione_player1[0]-1, posizione_player1[1]) && is_wall(posizione_player2[0]-1, posizione_player2[1], posizione_player2[0], posizione_player2[1]) && !is_wall(posizione_player2[0], posizione_player2[1]-1, posizione_player2[0], posizione_player2[1])){ //si trovano uno davanti all'altro ed il giocatore da scavalcare ha un muro dietro
							diagonal_up_left=1;
							
					} //posso andare in questa direzione della diagonale in due casi quindi devo controllarli entrambi
					else if(!controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]-1) && is_wall(posizione_player2[0], posizione_player2[1]-1, posizione_player2[0], posizione_player2[1]) && !is_wall(posizione_player2[0]-1, posizione_player2[1], posizione_player2[0], posizione_player2[1])){
							diagonal_up_left=1;
					}
			 }
			 else{
				 	if(!controlla_casella_giocatore(posizione_player1, posizione_player2[0]-1, posizione_player2[1]) && is_wall(posizione_player1[0]-1, posizione_player1[1], posizione_player1[0], posizione_player1[1]) && !is_wall(posizione_player1[0], posizione_player1[1]-1, posizione_player1[0], posizione_player1[1])){ //si trovano uno davanti all'altro ed il giocatore da scavalcare ha un muro dietro
							diagonal_up_left=1;
					}
					else if(!controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]-1) && is_wall(posizione_player1[0], posizione_player1[1]-1, posizione_player1[0], posizione_player1[1]) && !is_wall(posizione_player1[0]-1, posizione_player1[1], posizione_player1[0], posizione_player1[1])){
							diagonal_up_left=1;
					}
			 }
							up=0;
							down=0;
							right=0;
							left=0;
							diagonal_up_right=0;
							diagonal_down_right=0;
							diagonal_down_left=0;
							joystick_premuto=0;
		 }
		 else if ((LPC_GPIO1->FIOPIN & (1 << 29)) == 0 && (LPC_GPIO1->FIOPIN & (1 << 28)) == 0 ) { //in alto a destra
			 if(turno){
					if(!controlla_casella_giocatore(posizione_player2, posizione_player1[0]-1, posizione_player1[1]) && is_wall(posizione_player2[0]-1, posizione_player2[1], posizione_player2[0], posizione_player2[1]) && !is_wall(posizione_player2[0], posizione_player2[1]+1, posizione_player2[0], posizione_player2[1])){ //si trovano uno davanti all'altro ed il giocatore da scavalcare ha un muro dietro
							diagonal_up_right=1;
					}
					else if(!controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]+1) && is_wall(posizione_player2[0], posizione_player2[1]+1, posizione_player2[0], posizione_player2[1]) && !is_wall(posizione_player2[0]-1, posizione_player2[1], posizione_player2[0], posizione_player2[1])){
							diagonal_up_right=1;
					}
			 }
			 else{
				 	if(!controlla_casella_giocatore(posizione_player1, posizione_player2[0]-1, posizione_player2[1]) && is_wall(posizione_player1[0]-1, posizione_player1[1], posizione_player1[0], posizione_player1[1]) && !is_wall(posizione_player1[0], posizione_player1[1]+1, posizione_player1[0], posizione_player1[1])){ //si trovano uno davanti all'altro ed il giocatore da scavalcare ha un muro dietro
							diagonal_up_right=1;
					}
					else if(!controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]+1) && is_wall(posizione_player1[0], posizione_player1[1]+1, posizione_player1[0], posizione_player1[1]) && !is_wall(posizione_player1[0]-1, posizione_player1[1], posizione_player1[0], posizione_player1[1])){
							diagonal_up_right=1;
					}
			 }
							up=0;
							down=0;
							right=0;
							left=0;
							diagonal_up_left=0;
							diagonal_down_right=0;
							diagonal_down_left=0;
							joystick_premuto=0;
		 }
		 else if ((LPC_GPIO1->FIOPIN & (1 << 26)) == 0 && (LPC_GPIO1->FIOPIN & (1 << 27)) == 0 ) { //in basso a sinistra
			 if(turno){
					if(!controlla_casella_giocatore(posizione_player2, posizione_player1[0]+1, posizione_player1[1]) && is_wall(posizione_player2[0]+1, posizione_player2[1], posizione_player2[0], posizione_player2[1]) && !is_wall(posizione_player2[0], posizione_player2[1]-1, posizione_player2[0], posizione_player2[1])){ //si trovano uno davanti all'altro ed il giocatore da scavalcare ha un muro dietro
							diagonal_down_left=1;
					}
					else if(!controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]-1) && is_wall(posizione_player2[0], posizione_player2[1]-1, posizione_player2[0], posizione_player2[1]) && !is_wall(posizione_player2[0]+1, posizione_player2[1], posizione_player2[0], posizione_player2[1])){
							diagonal_down_left=1;
					}
			 }
			 else{
				 	if(!controlla_casella_giocatore(posizione_player1, posizione_player2[0]+1, posizione_player2[1]) && is_wall(posizione_player1[0]+1, posizione_player1[1], posizione_player1[0], posizione_player1[1]) && !is_wall(posizione_player1[0], posizione_player1[1]-1, posizione_player1[0], posizione_player1[1])){ //si trovano uno davanti all'altro ed il giocatore da scavalcare ha un muro dietro
							diagonal_down_left=1;
							
					}
					else if(!controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]-1) && is_wall(posizione_player1[0], posizione_player1[1]-1, posizione_player1[0], posizione_player1[1]) && !is_wall(posizione_player1[0]+1, posizione_player1[1], posizione_player1[0], posizione_player1[1])){
							diagonal_down_left=1;
					}
			 }
							up=0;
							down=0;
							right=0;
							left=0;
							diagonal_up_left=0;
							diagonal_down_right=0;
							diagonal_up_right=0;
							joystick_premuto=0;
		 }
		 else if ((LPC_GPIO1->FIOPIN & (1 << 26)) == 0 && (LPC_GPIO1->FIOPIN & (1 << 28)) == 0 ) { //in basso a destra
			 if(turno){
					if(!controlla_casella_giocatore(posizione_player2, posizione_player1[0]+1, posizione_player1[1]) && is_wall(posizione_player2[0]+1, posizione_player2[1], posizione_player2[0], posizione_player2[1]) && !is_wall(posizione_player2[0], posizione_player2[1]+1, posizione_player2[0], posizione_player2[1])){ //si trovano uno davanti all'altro ed il giocatore da scavalcare ha un muro dietro
							diagonal_down_right=1;
					}
					else if(!controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]+1) && is_wall(posizione_player2[0], posizione_player2[1]+1, posizione_player2[0], posizione_player2[1]) && !is_wall(posizione_player2[0]-1, posizione_player2[1], posizione_player2[0], posizione_player2[1])){
							diagonal_down_right=1;
					}
			 }
			 else{
				 	if(!controlla_casella_giocatore(posizione_player1, posizione_player2[0]+1, posizione_player2[1]) && is_wall(posizione_player1[0]+1, posizione_player1[1], posizione_player1[0], posizione_player1[1]) && !is_wall(posizione_player1[0], posizione_player1[1]+1, posizione_player1[0], posizione_player1[1])){ //si trovano uno davanti all'altro ed il giocatore da scavalcare ha un muro dietro
							diagonal_down_right=1;
					}
					else if(!controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]+1) && is_wall(posizione_player1[0], posizione_player1[1]+1, posizione_player1[0], posizione_player1[1]) && !is_wall(posizione_player1[0]-1, posizione_player1[1], posizione_player1[0], posizione_player1[1])){ 
							diagonal_down_right=1;
					}
			 }
			 				up=0;
							down=0;
							right=0;
							left=0;
							diagonal_up_left=0;
							diagonal_down_left=0;
							diagonal_up_right=0;
							joystick_premuto=0;
		 } 
   else if ((LPC_GPIO1->FIOPIN & (1 << 29)) == 0) {  // Joystick verso l'alto
			up=1;
			down=0;
			right=0;
			left=0;
			diagonal_up_left=0;
			diagonal_up_right=0;
			diagonal_down_right=0;
			diagonal_down_left=0;
			joystick_premuto=0;
			
		}
		else if((LPC_GPIO1->FIOPIN & (1 << 25)) == 0 && mosso!=1){ // joystick premuto
			joystick_premuto++;
			switch(joystick_premuto){
				case 1:
			if(diagonal_up_left){ //in questo caso ho già fatto i controlli prima
						if(turno){
										colora_quadrato(posizione_player1[0], posizione_player1[1], White);
										colora_caselle_vicine(posizione_player1,White, turno);
										posizione_player1[0]=posizione_player1[0]-1;
										posizione_player1[1]=posizione_player1[1]-1;
										muovi_giocatore(posizione_player1[0], posizione_player1[1], 1);
										registra_mossa(turno,wall,0,posizione_player1[0], posizione_player1[1]);
										mosso=1;
						}
						else{
										colora_quadrato(posizione_player2[0], posizione_player2[1], White);
										colora_caselle_vicine(posizione_player2,White, turno);
										posizione_player2[0]=posizione_player2[0]-1;
										posizione_player2[1]=posizione_player2[1]-1;
										muovi_giocatore(posizione_player2[0], posizione_player2[1], 0);
										registra_mossa(turno,wall,0,posizione_player2[0], posizione_player2[1]);
										mosso=1;
						}
			}
			else if(diagonal_up_right){ //in questo caso ho già fatto i controlli prima
						if(turno){
										colora_quadrato(posizione_player1[0], posizione_player1[1], White);
										colora_caselle_vicine(posizione_player1,White, turno);
										posizione_player1[0]=posizione_player1[0]-1;
										posizione_player1[1]=posizione_player1[1]+1;
										muovi_giocatore(posizione_player1[0], posizione_player1[1], 1);
										registra_mossa(turno,wall,0,posizione_player1[0], posizione_player1[1]);
										mosso=1;
				}
						else{
										colora_quadrato(posizione_player2[0], posizione_player2[1], White);
										colora_caselle_vicine(posizione_player2,White, turno);
										posizione_player2[0]=posizione_player2[0]-1;
										posizione_player2[1]=posizione_player2[1]+1;
										muovi_giocatore(posizione_player2[0], posizione_player2[1], 0);
										registra_mossa(turno,wall,0,posizione_player2[0], posizione_player2[1]);
										mosso=1;
				}
			}
			else if(diagonal_down_left){ //in questo caso ho già fatto i controlli prima
						if(turno){
										colora_quadrato(posizione_player1[0], posizione_player1[1], White);
										colora_caselle_vicine(posizione_player1,White, turno);
										posizione_player1[0]=posizione_player1[0]+1;
										posizione_player1[1]=posizione_player1[1]-1;
										muovi_giocatore(posizione_player1[0], posizione_player1[1], 1);
										registra_mossa(turno,wall,0,posizione_player1[0], posizione_player1[1]);
										mosso=1;
				}
						else{
										colora_quadrato(posizione_player2[0], posizione_player2[1], White);
										colora_caselle_vicine(posizione_player2,White, turno);
										posizione_player2[0]=posizione_player2[0]+1;
										posizione_player2[1]=posizione_player2[1]-1;
										muovi_giocatore(posizione_player2[0], posizione_player2[1], 0);
										registra_mossa(turno,wall,0,posizione_player2[0], posizione_player2[1]);
										mosso=1;
				}
			}
			else if(diagonal_down_right){ //in questo caso ho già fatto i controlli prima
						if(turno){
										colora_quadrato(posizione_player1[0], posizione_player1[1], White);
										colora_caselle_vicine(posizione_player1,White, turno);
										posizione_player1[0]=posizione_player1[0]+1;
										posizione_player1[1]=posizione_player1[1]+1;
										muovi_giocatore(posizione_player1[0], posizione_player1[1], 1);
										registra_mossa(turno,wall,0,posizione_player1[0], posizione_player1[1]);
										mosso=1;
				}
						else{
										colora_quadrato(posizione_player2[0], posizione_player2[1], White);
										colora_caselle_vicine(posizione_player2,White, turno);
										posizione_player2[0]=posizione_player2[0]+1;
										posizione_player2[1]=posizione_player2[1]+1;
										muovi_giocatore(posizione_player2[0], posizione_player2[1], 0);
										registra_mossa(turno,wall,0,posizione_player2[0], posizione_player2[1]);
										mosso=1;
				}
			}
			else if(up){
						if(turno){
								if (posizione_player1[0] > 0 && !is_wall(posizione_player1[0]-1, posizione_player1[1], posizione_player1[0], posizione_player1[1])) {
									if(!controlla_casella_giocatore(posizione_player2, posizione_player1[0]-1, posizione_player1[1]) && scavalca(posizione_player2[0], posizione_player2[1], 'u') && posizione_player2[0]!=0){  //scavalcare
										colora_quadrato(posizione_player1[0], posizione_player1[1], White);
										colora_caselle_vicine(posizione_player1,White, turno);
										posizione_player1[0]=posizione_player1[0]-2;
										muovi_giocatore(posizione_player1[0], posizione_player1[1], 1);
										registra_mossa(turno,wall,0,posizione_player1[0], posizione_player1[1]);
										mosso=1;
									}
									else if(controlla_casella_giocatore(posizione_player2, posizione_player1[0]-1, posizione_player1[1])){
										colora_quadrato(posizione_player1[0], posizione_player1[1], White); //copro la posizione vecchia
										colora_caselle_vicine(posizione_player1,White, turno); //copro le caselle evidenziate
										posizione_player1[0]--;
										muovi_giocatore(posizione_player1[0], posizione_player1[1], 1);
										registra_mossa(turno,wall,0,posizione_player1[0], posizione_player1[1]);
										mosso=1;
									}
            }
					}
						else{
							if (posizione_player2[0] > 0 && !is_wall(posizione_player2[0]-1, posizione_player2[1], posizione_player2[0], posizione_player2[1])) {
								if(!controlla_casella_giocatore(posizione_player1, posizione_player2[0]-1, posizione_player2[1]) && scavalca(posizione_player1[0], posizione_player1[1], 'u') && posizione_player1[0]!=0){  //scavalcare
										colora_quadrato(posizione_player2[0], posizione_player2[1], White);
										colora_caselle_vicine(posizione_player2,White, turno);
										posizione_player2[0]=posizione_player2[0]-2;
										muovi_giocatore(posizione_player2[0], posizione_player2[1], 0);
										registra_mossa(turno,wall,0,posizione_player2[0], posizione_player2[1]);
										mosso=1;
								}
								else if(controlla_casella_giocatore(posizione_player1, posizione_player2[0]-1, posizione_player2[1])){
										colora_quadrato(posizione_player2[0], posizione_player2[1], White);
										colora_caselle_vicine(posizione_player2,White, turno);
										posizione_player2[0]--;
										muovi_giocatore(posizione_player2[0], posizione_player2[1], 0);
										registra_mossa(turno,wall,0,posizione_player2[0], posizione_player2[1]);
										mosso=1;
								}
            }
					}
		}
			else if(down){
				
						if(turno){
								if (posizione_player1[0] < 6 && !is_wall(posizione_player1[0]+1, posizione_player1[1], posizione_player1[0], posizione_player1[1])) {
									if(!controlla_casella_giocatore(posizione_player2, posizione_player1[0]+1, posizione_player1[1]) && scavalca(posizione_player2[0], posizione_player2[1], 'd') && posizione_player2[0]!=6){  //scavalcare
										colora_quadrato(posizione_player1[0], posizione_player1[1], White);
										colora_caselle_vicine(posizione_player1,White,turno);
										posizione_player1[0]=posizione_player1[0]+2;
										muovi_giocatore(posizione_player1[0], posizione_player1[1], 1);
										registra_mossa(turno,wall,0,posizione_player1[0], posizione_player1[1]);
										mosso=1;
									}
									else if(controlla_casella_giocatore(posizione_player2, posizione_player1[0]+1, posizione_player1[1])){
										colora_quadrato(posizione_player1[0], posizione_player1[1], White);
										colora_caselle_vicine(posizione_player1,White, turno);
										posizione_player1[0]++;
										muovi_giocatore(posizione_player1[0], posizione_player1[1], 1);
										registra_mossa(turno,wall,0,posizione_player1[0], posizione_player1[1]);
										mosso=1;
									}
            }
					}
						else{
							if (posizione_player2[0] < 6 && !is_wall(posizione_player2[0]+1, posizione_player2[1], posizione_player2[0], posizione_player2[1])) {
								if(!controlla_casella_giocatore(posizione_player1, posizione_player2[0]+1, posizione_player2[1]) && scavalca(posizione_player1[0], posizione_player1[1], 'd') &&  posizione_player1[0]!=6){  //scavalcare
										colora_quadrato(posizione_player2[0], posizione_player2[1], White);
										colora_caselle_vicine(posizione_player2,White, turno);
										posizione_player2[0]=posizione_player2[0]+2;
										muovi_giocatore(posizione_player2[0], posizione_player2[1], 0);
										registra_mossa(turno,wall,0,posizione_player2[0], posizione_player2[1]);
										mosso=1;
								}
								else if(controlla_casella_giocatore(posizione_player1, posizione_player2[0]+1, posizione_player2[1])){
										colora_quadrato(posizione_player2[0], posizione_player2[1], White);
										colora_caselle_vicine(posizione_player2,White, turno);
										posizione_player2[0]++;
										muovi_giocatore(posizione_player2[0], posizione_player2[1], 0);
										registra_mossa(turno,wall,0,posizione_player2[0], posizione_player2[1]);
										mosso=1;
								}
            }
					}				
				
			}
			else if(right){
				
				    if(turno){
								if (posizione_player1[1] < 6 && !is_wall(posizione_player1[0], posizione_player1[1]+1, posizione_player1[0],posizione_player1[1])) {
									if(!controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]+1) && scavalca(posizione_player2[0], posizione_player2[1], 'r') && posizione_player2[1]!=6){  //scavalcare
										colora_quadrato(posizione_player1[0], posizione_player1[1], White);
										colora_caselle_vicine(posizione_player1,White, turno);
										posizione_player1[1]=posizione_player1[1]+2;
										muovi_giocatore(posizione_player1[0], posizione_player1[1], 1);
										registra_mossa(turno,wall,0,posizione_player1[0], posizione_player1[1]);
										mosso=1;
									}
									else if(controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]+1)){
										colora_quadrato(posizione_player1[0], posizione_player1[1], White);
										colora_caselle_vicine(posizione_player1,White, turno);
										posizione_player1[1]++;
										muovi_giocatore(posizione_player1[0], posizione_player1[1], 1);
										registra_mossa(turno,wall,0,posizione_player1[0], posizione_player1[1]);
										mosso=1;
									}
								}
					}
						else{
							if (posizione_player2[1] < 6  && !is_wall(posizione_player2[0], posizione_player2[1]+1, posizione_player2[0], posizione_player2[1])) {
								if(!controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]+1) && scavalca(posizione_player1[0], posizione_player1[1], 'r') && posizione_player1[1]!=6){  //scavalcare
									colora_quadrato(posizione_player2[0], posizione_player2[1], White);
									colora_caselle_vicine(posizione_player2,White, turno);
									posizione_player2[1]=posizione_player2[1]+2;
									muovi_giocatore(posizione_player2[0], posizione_player2[1], 0);
									registra_mossa(turno,wall,0,posizione_player2[0], posizione_player2[1]);
									mosso=1;
								}
								else if(controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]+1)){
									colora_quadrato(posizione_player2[0], posizione_player2[1], White);
									colora_caselle_vicine(posizione_player2,White, turno);
									posizione_player2[1]++;
									muovi_giocatore(posizione_player2[0], posizione_player2[1], 0);
									registra_mossa(turno,wall,0,posizione_player2[0], posizione_player2[1]);
									mosso=1;
								}
            }
					}
			}
			else if(left){
						if(turno){
								if (posizione_player1[1] > 0 && !is_wall(posizione_player1[0], posizione_player1[1]-1, posizione_player1[0], posizione_player2[0])) {
										if(!controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]-1) && scavalca(posizione_player2[0], posizione_player2[1], 'l') && posizione_player2[1]!=0){  //scavalcare
										colora_quadrato(posizione_player1[0], posizione_player1[1], White);
										colora_caselle_vicine(posizione_player1,White, turno);
										posizione_player1[1]=posizione_player1[1]-2;
										muovi_giocatore(posizione_player1[0], posizione_player1[1], 1);
										registra_mossa(turno,wall,0,posizione_player1[0], posizione_player1[1]);
										mosso=1;
									}
									else if(controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]-1)){
										colora_quadrato(posizione_player1[0], posizione_player1[1], White);
										colora_caselle_vicine(posizione_player1,White, turno);
										posizione_player1[1]--;
										muovi_giocatore(posizione_player1[0], posizione_player1[1], 1);
										registra_mossa(turno,wall,0,posizione_player1[0], posizione_player1[1]);
										mosso=1;
									}
								}
						}
						else{
								if (posizione_player2[1] > 0 && !is_wall(posizione_player2[0], posizione_player2[1]-1, posizione_player2[0], posizione_player2[1])) {
									if(!controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]-1) && scavalca(posizione_player1[0], posizione_player1[1], 'l') && posizione_player1[1]!=0){  //scavalcare
										colora_quadrato(posizione_player2[0], posizione_player2[1], White);
										colora_caselle_vicine(posizione_player2,White, turno);
										posizione_player2[1]=posizione_player2[1]-2;
										muovi_giocatore(posizione_player2[0], posizione_player2[1], 0);
										registra_mossa(turno,wall,0,posizione_player2[0], posizione_player2[1]);
										mosso=1;
									}
									else if(controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]-1)){
										colora_quadrato(posizione_player2[0], posizione_player2[1], White);
										colora_caselle_vicine(posizione_player2,White, turno);
										posizione_player2[1]--;
										muovi_giocatore(posizione_player2[0], posizione_player2[1], 0);
										registra_mossa(turno,wall,0,posizione_player2[0], posizione_player2[1]);
										mosso=1;
									}
							}
					}
			}
			up=0;
			down=0;
			right=0;
			left=0;
			diagonal_up_left=0;
			diagonal_up_right=0;
			diagonal_down_right=0;
			diagonal_down_left=0;
		
			break;
			default: break;
		}
	}
	else{ //rilascio del joystick
		joystick_premuto=0;
	}
			if ((LPC_GPIO1->FIOPIN & (1 << 26)) == 0){ //joystick verso basso
			up=0;
			down=1;
			right=0;
			left=0;
			diagonal_up_left=0;
			diagonal_up_right=0;
			diagonal_down_right=0;
			diagonal_down_left=0;
			joystick_premuto=0;

		 }
		 else if ((LPC_GPIO1->FIOPIN & (1 << 27)) == 0){ //joystick verso sinistra
			up=0;
			down=0;
			right=0;
			left=1;
			diagonal_up_left=0;
			diagonal_up_right=0;
			diagonal_down_right=0;
			diagonal_down_left=0;
			joystick_premuto=0;

			 
	 }
		 else if ((LPC_GPIO1->FIOPIN & (1 << 28)) == 0){  //joystick verso destra
			up=0;
			down=0;
			right=1;
			left=0;
			diagonal_up_left=0;
			diagonal_up_right=0;
			diagonal_down_right=0;
			diagonal_down_left=0;
			joystick_premuto=0;
		 }
	 }
	else{ //sta posizionando un muro 
		if ((LPC_GPIO1->FIOPIN & (1 << 29)) == 0) {  // Joystick verso l'alto
			
					if(turno){
							if(!ruotato){ //orizzontale
								if (posizione_wall[0] > 0 ){ 
											controlla_presenza_orizzontale();
											controlla_presenza_verticale();
											posizione_wall[0]--;
											create_wall(Black);
								}
							}
							else{	//verticale
											if (posizione_wall[0] > 0 ){ 
													ruota_muro_90(White);
													controlla_presenza_orizzontale();
													controlla_presenza_verticale();
													posizione_wall[0]--;
													ruota_muro_90(Black);
													if(!controlla_presenza_orizzontale()) ruota_muro_90(Black);
								}
							}
						}
					else{
							if(!ruotato){ //orizzontale
								if (posizione_wall[0] > 0 ){ 
											controlla_presenza_orizzontale();
											controlla_presenza_verticale();
											posizione_wall[0]--;
											create_wall(Red);
								}
							}
							else{ //verticale
											if (posizione_wall[0] > 0 ){ 
													ruota_muro_90(White);
													controlla_presenza_orizzontale();
													controlla_presenza_verticale();
													posizione_wall[0]--;
													ruota_muro_90(Red);
													if(!controlla_presenza_orizzontale()) ruota_muro_90(Red);
											}
						}
				}
		
	}
		if((LPC_GPIO1->FIOPIN & (1 << 25)) == 0 && mosso!=1){ // joystick premuto
			
			
			if(!ruotato){ //orizzontale
				if(controlla_presenza_orizzontale()==0 && controlla_blocco_giocatori(posizione_wall[0], posizione_wall[1],'H')==0 && 	controlla_presenza_verticale()==0){
					
						if(!turno)		create_wall(Red);
						else					create_wall(Black);
						mosso=1;
						muri_definitivi[counter_muri].riga=posizione_wall[0];
						muri_definitivi[counter_muri].colonna=posizione_wall[1];
					
						if (turno)	muri_definitivi[counter_muri].colore=Black;
						else 				muri_definitivi[counter_muri].colore=Red;
					
						muri_definitivi[counter_muri].orientamento='H';
						counter_muri++;
						if(turno){ 
							p1_wall--;
							sprintf(str, "%d", p1_wall);
							GUI_Text(35, 290,  (uint8_t *) str,Black,White);
						}
						else{
							p2_wall--;
							sprintf(str, "%d", p2_wall);
							GUI_Text(191, 290,  (uint8_t *) str,Black,White);
						}
						registra_mossa(turno,wall,!ruotato,muri_definitivi[counter_muri-1].riga, muri_definitivi[counter_muri-1].colonna);
				}
					if(!turno)		create_wall(Red);
					else					create_wall(Black);
			}
				else{ //verticale
						if(controlla_presenza_verticale()==0 && controlla_blocco_giocatori(posizione_wall[0], posizione_wall[1],'V')==0 && controlla_presenza_orizzontale()==0){

							if(!turno)	ruota_muro_90(Red);
								else 				ruota_muro_90(Black);
								mosso=1;
								muri_definitivi[counter_muri].riga=posizione_wall[0];
								muri_definitivi[counter_muri].colonna=posizione_wall[1];
							
								if (turno) muri_definitivi[counter_muri].colore=Black;
								else muri_definitivi[counter_muri].colore=Red;
							
								muri_definitivi[counter_muri].orientamento='V';
								counter_muri++;
								if(turno){ 
										p1_wall--;
										sprintf(str, "%d", p1_wall);
										GUI_Text(35, 290,  (uint8_t *) str,Black,White);
								}
								else{
										p2_wall--;
										sprintf(str, "%d", p2_wall);
										GUI_Text(191, 290,  (uint8_t *) str,Black,White);
						}
								registra_mossa(turno,wall,!ruotato,muri_definitivi[counter_muri-1].riga, muri_definitivi[counter_muri-1].colonna);
				}
								if(!turno)	ruota_muro_90(Red);  //aggiungere nell'if mosso!=1 forse
								else 				ruota_muro_90(Black);
			}
			
		}
							
		if ((LPC_GPIO1->FIOPIN & (1 << 26)) == 0){ //joystick verso basso
			
					if(turno){
							if(!ruotato){ //orizzontale
									if (posizione_wall[0] < 5) {
											controlla_presenza_orizzontale();
											controlla_presenza_verticale();						
											posizione_wall[0]++;
											create_wall(Black);
									}
							}
							else{ //verticale
								if (posizione_wall[0] < 5) {
														ruota_muro_90(White);
														controlla_presenza_orizzontale();
														controlla_presenza_verticale();					
														posizione_wall[0]++;
														ruota_muro_90(Black);
														if(!controlla_presenza_orizzontale()) ruota_muro_90(Black);
									}
								
							}
					}
					else{
						if(!ruotato){ //orizzontale
									if (posizione_wall[0] < 5) {
													controlla_presenza_orizzontale();
													controlla_presenza_verticale();
													posizione_wall[0]++;
													create_wall(Red);
									}
            }
						else{ //verticale
											if (posizione_wall[0] < 5) {
														ruota_muro_90(White);
														controlla_presenza_orizzontale();
														controlla_presenza_verticale();					
														posizione_wall[0]++;
														ruota_muro_90(Red);
														if(!controlla_presenza_orizzontale()) ruota_muro_90(Red);
									}
									
							
						}
				}
			
		}
		if ((LPC_GPIO1->FIOPIN & (1 << 27)) == 0){ //joystick verso sinistra
			
					if(turno){
							if(!ruotato){ //orizzontale
										if (posizione_wall[1]> 0) {
													controlla_presenza_orizzontale();
													controlla_presenza_verticale();
													posizione_wall[1]--;
													create_wall(Black);
									}
							}
							else{ //verticale
										if (posizione_wall[1] > 0) {
														ruota_muro_90(White);
														controlla_presenza_orizzontale();
														controlla_presenza_verticale();							
														posizione_wall[1]--;
														ruota_muro_90(Black);		
														if(!controlla_presenza_orizzontale()) ruota_muro_90(Black);
										}
							}
						}
					else{
						if(!ruotato){ //orizzontale
										if (posizione_wall[1]> 0) {
													controlla_presenza_orizzontale();
													controlla_presenza_verticale();								
													posizione_wall[1]--;
													create_wall(Red);
									}
						}	
						else{ //verticale
										if (posizione_wall[1] > 0) {
														ruota_muro_90(White);
														controlla_presenza_orizzontale();
														controlla_presenza_verticale();					
														posizione_wall[1]--;
														ruota_muro_90(Red);		
													if(!controlla_presenza_orizzontale()) ruota_muro_90(Red);
										}
							}
					}
					
				}
		if ((LPC_GPIO1->FIOPIN & (1 << 28)) == 0){  //joystick verso destra
			
					if(turno){
						if(!ruotato){ //orizzontale
									if (posizione_wall[1] < 5) {
												controlla_presenza_orizzontale();
												controlla_presenza_verticale();								
												posizione_wall[1]++;
												create_wall(Black);
									}
						}
						else{ //verticale
									if (posizione_wall[1] < 5) {
												ruota_muro_90(White);
												controlla_presenza_orizzontale();
												controlla_presenza_verticale();						
												posizione_wall[1]++;
												ruota_muro_90(Black);	
												if(!controlla_presenza_orizzontale()) ruota_muro_90(Black);
										}
							}
							
						}
						
					else{
								if(!ruotato){ //orizzontale
										if (posizione_wall[1] < 5) {
												controlla_presenza_orizzontale();
												controlla_presenza_verticale();							
												posizione_wall[1]++;
												create_wall(Red);
						}
					}
								else{ //verticale
										if (posizione_wall[1] < 5) {
												ruota_muro_90(White);
												controlla_presenza_orizzontale();
												controlla_presenza_verticale();	
												posizione_wall[1]++;
												ruota_muro_90(Red);	
												if(!controlla_presenza_orizzontale()) ruota_muro_90(Red);
										}
								}
					}
			
				}
		}
	}
		else if(((game_mode==2 && modalita_player==2 && turno!=ospite) || (game_mode==1 && opposite_player==2 && turno==0)) && !mosso){ //turno npc (forse bisogna aggiungere !mosso
 			if(turno) mossa_npc(posizione_player2,p1_wall,posizione_player1);
			else mossa_npc(posizione_player1,p2_wall,posizione_player2);
				if(!mossa__npc.muro){
									if(turno){
										colora_quadrato(posizione_player1[0], posizione_player1[1], White);
										colora_caselle_vicine(posizione_player1,White, turno);
										posizione_player1[0]=mossa__npc.riga;
										posizione_player1[1]=mossa__npc.colonna;
										muovi_giocatore(mossa__npc.riga, mossa__npc.colonna, 1);
										registra_mossa(turno,0,0,mossa__npc.riga, mossa__npc.colonna);
										mosso=1;
									}
									else{
										colora_quadrato(posizione_player2[0], posizione_player2[1], White);
										colora_caselle_vicine(posizione_player2,White, turno);
										posizione_player2[0]=mossa__npc.riga;
										posizione_player2[1]=mossa__npc.colonna;
										muovi_giocatore(mossa__npc.riga, mossa__npc.colonna, 0);
										registra_mossa(turno,0,0,mossa__npc.riga, mossa__npc.colonna);
										mosso=1;
									}
				}
				else{
								if(mossa__npc.orientamento == 'H'){
									
										posizione_wall[0]=mossa__npc.riga;
										posizione_wall[1]=mossa__npc.colonna;
									
										if(turno)create_wall(Black);
										else create_wall(Red);
										
										mosso=1;
										muri_definitivi[counter_muri].riga=mossa__npc.riga;
										muri_definitivi[counter_muri].colonna=mossa__npc.colonna;
										muri_definitivi[counter_muri].colore=(!turno) ? Red : Black;
										muri_definitivi[counter_muri].orientamento='H';
										counter_muri++;					
									
										if(turno) {
										p1_wall--;
										sprintf(str, "%d", p1_wall);
										GUI_Text(191, 290,  (uint8_t *) str,Black,White);
										}
										else{
										p2_wall--;
										sprintf(str, "%d", p2_wall);
										GUI_Text(191, 290,  (uint8_t *) str,Black,White);
										}
										registra_mossa(turno,1,1,muri_definitivi[counter_muri-1].riga, muri_definitivi[counter_muri-1].colonna);
								}
								else{
										
										posizione_wall[0]=mossa__npc.riga;
										posizione_wall[1]=mossa__npc.colonna;
									
										if(turno) ruota_muro_90(Black);
										else ruota_muro_90(Red);
										
										mosso=1;
										muri_definitivi[counter_muri].riga=mossa__npc.riga;
										muri_definitivi[counter_muri].colonna=mossa__npc.colonna;
										muri_definitivi[counter_muri].colore=(!turno) ? Red : Black;
										muri_definitivi[counter_muri].orientamento='V';
										counter_muri++;					
									
										if(turno) {
										p1_wall--;
										sprintf(str, "%d", p1_wall);
										GUI_Text(191, 290,  (uint8_t *) str,Black,White);
										}
										else{
										p2_wall--;
										sprintf(str, "%d", p2_wall);
										GUI_Text(191, 290,  (uint8_t *) str,Black,White);
										}
										
										registra_mossa(turno,1,0,muri_definitivi[counter_muri-1].riga, muri_definitivi[counter_muri-1].colonna); 
									
								}
					}
		}
}
	 
//bouncing bottoni
if(down_key1!=0 && (turno!=ospite || game_mode==1)){
	if((LPC_GPIO2->FIOPIN & (1<<11)) == 0){ //KEY1 PRESSED
		down_key1++;
		switch(down_key1){
			case 2: 
		if (turno && p1_wall == 0 && wall == 0) { // finiti i muri
        GUI_Text(6, 240, (uint8_t *)"No walls left, move the token", Black, White);
				cancella_warning=1;
        LPC_SC->EXTINT &= (1 << 1);
        return;
    }
    if (!turno && p2_wall == 0 && wall == 0) { // finiti i muri
        GUI_Text(6, 240, (uint8_t *)"No walls left, move the token", Red, White);
				cancella_warning=1;
        LPC_SC->EXTINT &= (1 << 1);
        return;
    }

    if (turno && p1_wall != 0 && wall == 0) { // si entra nello stato posizionamento muro
        colora_caselle_vicine(posizione_player1, White, turno);
        create_wall(Black);
        wall = 1;
    } else if (p2_wall != 0 && wall == 0) { // si entra nello stato posizionamento muro
        colora_caselle_vicine(posizione_player2, White, turno);
        create_wall(Red);
        wall = 1;
    } else { //si torna nello stato inziale
        wall = 0;

        if (!ruotato)
            controlla_presenza_orizzontale(); // mi cancella il muro che stava posizionando il giocatore ed in caso ci fosse già un muro non me lo fa sparire
        else
            controlla_presenza_verticale(); // stessa cosa ma per i muri verticali

        if (turno)
            colora_caselle_vicine(posizione_player1, Yellow, turno);
        else
            colora_caselle_vicine(posizione_player2, Yellow, turno);

        ruotato = 0;
    }
		break;
			default: break;
		}
	}
	else{ //buitton released
		down_key1=0;
		NVIC_EnableIRQ(EINT1_IRQn);
		LPC_PINCON->PINSEL4	|=(1 << 22);
		
	}
}
if(down_key0!=0 && game_mode==0){
	if((LPC_GPIO2->FIOPIN & (1<<10)) == 0){ //KEY0 PRESSED
		down_key0++;
		switch(down_key0){
			case 2: 
			modalita_player=1;
			menu_game_mode();
			break;
			default: break;
		}
	}
	else{ //buitton released
		down_key0=0;
		NVIC_EnableIRQ(EINT0_IRQn);
		LPC_PINCON->PINSEL4	|=(1 << 20);
		
	}
}
if(down_key2!=0 && (turno!=ospite || game_mode==1)){
	if((LPC_GPIO2->FIOPIN & (1<<12)) == 0){ //KEY2 PRESSED
		down_key2++;
		switch(down_key2){
			case 2: 
			  if (wall && !ruotato) { // controllo se sono nella modalità di inserimento del muro e se devo ruotare in verticale
					ruotato = 1;
					if (turno) {
							ruota_muro_90(Black);
							if (!controlla_presenza_orizzontale()) //cancella quello orizzontale ma controlla anche che non si cancelli un muro magari già posizionato
                ruota_muro_90(Black);
					} else {
							ruota_muro_90(Red);
							if (!controlla_presenza_orizzontale()) //cancella quello orizzontale ma controlla anche che non si cancelli un muro magari già posizionato
									ruota_muro_90(Red);
					}
				} else if (wall && ruotato) { // in questo caso da verticale lo ripristino orizzontale
						ruotato = 0;
						ruota_muro_90(White); // cancello quello verticale
						controlla_presenza_verticale(); //controllo che non abbia cancellato altri muri (pezzi) già posizionati
						if (turno)
								create_wall(Black); //disegno quello orizzontale
						else
								create_wall(Red);
					}
			break;
			default: break;
		}
	}
	else{ //buitton released
		down_key2=0;
		NVIC_EnableIRQ(EINT2_IRQn);
		LPC_PINCON->PINSEL4	|=(1 << 24);
		
	}
}
	
	//alla fine controllo se sono nella modalità multiplayer e se si mando il messaggio con la mossa fatta
				
	LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
