/**--------------File Info-------------------------------------------------------------------------------
** File name:			gioco.c
** Descriptions:	implementa le funzioni utili per la logica del gioco
**------------------------------------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "gioco.h"
#include "drawing.h"
#include "GLCD/GLCD.h" 
#include "joystick/joystick.h"
#include "RIT/RIT.h"
#include "timer/timer.h"
#include "CAN/CAN.h"
#include <stdlib.h>
#include <stdio.h>



//costanti per identificare le diverse informazioni
#define PLAYER_ID_MASK             0xFF000000
#define PLAYER_MOVE_WALL_MASK      0x00F00000
#define VERTICAL_HORIZONTAL_MASK   0x000F0000
#define Y_COORDINATE_MASK          0x0000FF00
#define X_COORDINATE_MASK          0x000000FF

/* Private variables ---------------------------------------------------------*/
static uint32_t mossa = 0; //dove viene salavata ogni volta l'ulitma mossa
int vittoria = 0; //per fermare l'accensione del timer1 nel RIT
MossaNPC mossa__npc;


/* External variables ---------------------------------------------------------*/
extern int counter_muri;
extern PosizioneMuro muri_definitivi[16];
extern int posizione_wall[2];
extern int posizione_player2[2];
extern int posizione_player1[2];
extern int ruotato;
extern int p1_wall;
extern int p2_wall;
extern int turno;
extern uint8_t ospite;
extern int mosso;
extern int game_mode;


/*******************************************************************************
* Function Name  : is_wall
* Description    : Determina se un giocatore può effettivamente nella direzione richiesta o
									 è presente un muro
* Input          : riga e colonna sia di arrivo che di partenza
* Return         : 1 se è presente un muro tra le due posizioni, 0 altrimenti
*******************************************************************************/

int is_wall(int riga_casella_arrivo, int colonna_casella_arrivo, int riga_casella_partenza, int colonna_casella_partenza) {
	
    int result = 0;
    uint16_t i;
	
	// NOTA: aggiungo alle posizione dei muri 2 (nella riga) e 3 (nella colonna) affinché possano essere comparate le posizione dei giocatori con quelle dei muri
	// dato che la posizione inziale di un muro dopo la pressione del bottone sarebbe (2,3)

    for (i = 0; i < counter_muri; i++) {
        if (muri_definitivi[i].orientamento == 'H') { // Logica per i muri orizzontali
            if (riga_casella_arrivo != riga_casella_partenza) { // Si sta spostando in verticale
                // Controlla se il muro copre il percorso della pedina
                if ((riga_casella_partenza == muri_definitivi[i].riga || riga_casella_partenza == muri_definitivi[i].riga+1) &&
                    (colonna_casella_partenza == muri_definitivi[i].colonna || colonna_casella_partenza == muri_definitivi[i].colonna + 1) &&
                    (riga_casella_arrivo == muri_definitivi[i].riga || riga_casella_arrivo == muri_definitivi[i].riga + 1)) {
                    result = 1;
                }
            }
        } else { // Logica per i muri verticali
            if (colonna_casella_arrivo != colonna_casella_partenza) { // Si sta spostando in orizzontale
                // Controlla se il muro copre il percorso della pedina
                if ((colonna_casella_partenza == muri_definitivi[i].colonna || colonna_casella_partenza == muri_definitivi[i].colonna + 1) &&
                    (riga_casella_partenza == muri_definitivi[i].riga || riga_casella_partenza == muri_definitivi[i].riga + 1) &&
                    (colonna_casella_arrivo == muri_definitivi[i].colonna || colonna_casella_arrivo == muri_definitivi[i].colonna + 1)) {
                    result = 1;
                }
            }
        }
    }

    return result;
}


/*******************************************************************************
* Function Name  : is_finished
* Description    : Controlla che nessuna abbia vinto, in caso annuncia il vincitore sul display
* Input          : posizione del giocatore e turno per capire chi ha vinto
* Return         : None
*******************************************************************************/

void is_finished(int* pos, int turno) {
	
    if (turno) { // player1
        if (pos[0] == 0) { // è arrivato alla fine
            disable_RIT();
            disable_timer(1);
						NVIC_DisableIRQ(EINT0_IRQn);
						NVIC_DisableIRQ(EINT1_IRQn);
						NVIC_DisableIRQ(EINT2_IRQn);
            LCD_Clear(White);
            GUI_Text(40, 150, (uint8_t *)"PLAYER1 WIN", Black, White);
						vittoria=1;
        }
    } else { // player2
        if (pos[0] == 6) { // è arrivato alla fine
            disable_RIT();
            disable_timer(1);
            NVIC_DisableIRQ(EINT0_IRQn);
						NVIC_DisableIRQ(EINT1_IRQn);
						NVIC_DisableIRQ(EINT2_IRQn);
						LCD_Clear(White);
            GUI_Text(40, 150, (uint8_t *)"PLAYER2 WIN", Red, White);
						vittoria=1;
        }
    }
}


/*******************************************************************************
* Function Name  : controlla_casella_giocatore
* Description    : Controlla che un giocatore non possa andare su una casella occupata dall'altro
* Input          : posizione del primo, riga e colonna del secondo
* Return         : 1 se è presente già un giocatore, 0 altrimenti
*******************************************************************************/

int controlla_casella_giocatore(int* pos, uint16_t riga, uint16_t colonna) {
	
    if (pos[0] == riga && pos[1] == colonna) {
        return 0;
    } else {
        return 1;
    }
}


/*******************************************************************************
* Function Name  : scavalca
* Description    : Controlla se, quando i due giocatori si trovano uno davanti all'altro, è
									 possibile scavalcarlo o è presente un muro dietro di lui
* Input          : riga e colonna della posizione attuale del giocatore da scavalcare, movimento
* Return         : 1 se è presente già un giocatore, 0 altrimenti
*******************************************************************************/

int scavalca(uint16_t riga_iniziale, uint16_t colonna_inziale, char movimento){ 
	
	//movimento = 'u' || 'd' || 'l' || 'r' || 'dlu' || 'dru' || 'dld' || 'drd'
	// up, down, left, right
	//l'idea è di controllare se il giocatore da scavalcare sarebbe potuto andare in quella casella o meno
	switch(movimento){
		case 'u': {
			if(!(is_wall(riga_iniziale-1,colonna_inziale,riga_iniziale,colonna_inziale))) return 1; 
			else return 0;
		}
		case 'd': {
			if(!(is_wall(riga_iniziale+1,colonna_inziale,riga_iniziale,colonna_inziale))) return 1;
			else return 0;
		}
		case 'l': {
			if(!(is_wall(riga_iniziale,colonna_inziale-1,riga_iniziale,colonna_inziale))) return 1;
			else return 0;
		}
		case 'r': {
			if(!(is_wall(riga_iniziale,colonna_inziale+1,riga_iniziale,colonna_inziale))) return 1;
			else return 0;
		}
		default: return 0;
		}
}


/*******************************************************************************
* Function Name  : controlla_presenza_orizzontale
* Description    : Questa funzione ha una doppia funzione. Evita che un muro che deve essere ancora
									 definitivamente posizionato cancelli un muro già posizionato. 
									 Inoltre fa in modo che non venga posizionato un muro dove n'è già presente uno 
* Input          : 
* Return         : 1 se è presente già un muro, 0 altrimenti
*******************************************************************************/

int controlla_presenza_orizzontale(void) {
	
    uint8_t i;
    int pos[2];
    int result = 0;

    create_wall(White);

    for (i = 0; i < counter_muri; i++) {
        if (muri_definitivi[i].riga == posizione_wall[0] && muri_definitivi[i].colonna == posizione_wall[1] && muri_definitivi[i].orientamento == 'H') {
            pos[0] = muri_definitivi[i].riga;
            pos[1] = muri_definitivi[i].colonna;
            disegna_pezzetto_orizzontale(muri_definitivi[i].colore, pos);
            result = 1;
        }
    }

    for (i = 0; i < counter_muri; i++) {
        if (muri_definitivi[i].riga == posizione_wall[0] && abs(posizione_wall[1] - muri_definitivi[i].colonna) == 1 && muri_definitivi[i].orientamento == 'H') {
            pos[0] = muri_definitivi[i].riga;
            pos[1] = muri_definitivi[i].colonna;
            disegna_pezzetto_orizzontale(muri_definitivi[i].colore, pos);
            if (!ruotato) result = 1;
        }
    }

    return result;
}


/*******************************************************************************
* Function Name  : controlla_presenza_verticale
* Description    : Questa funzione ha una doppia funzione. Evita che un muro che deve essere ancora
									 definitivamente posizionato cancelli un muro già posizionato. 
									 Inoltre fa in modo che non venga posizionato un muro dove n'è già presente uno 
* Input          : 
* Return         : 1 se è presente già un muro, 0 altrimenti
*******************************************************************************/

int controlla_presenza_verticale(void) {
    uint8_t i;
    int pos[2];
    int result = 0;

    ruota_muro_90(White);

    for (i = 0; i < counter_muri; i++) {
        if (muri_definitivi[i].riga == posizione_wall[0] && muri_definitivi[i].colonna == posizione_wall[1] && muri_definitivi[i].orientamento == 'V') {
            pos[0] = muri_definitivi[i].riga;
            pos[1] = muri_definitivi[i].colonna;
            disegna_pezzetto_verticale(muri_definitivi[i].colore, pos);
            result = 1;
        }
    }

    for (i = 0; i < counter_muri; i++) {
        if (muri_definitivi[i].colonna == posizione_wall[1] && abs(posizione_wall[0] - muri_definitivi[i].riga) == 1 && muri_definitivi[i].orientamento == 'V') {
            pos[0] = muri_definitivi[i].riga;
            pos[1] = muri_definitivi[i].colonna;
            disegna_pezzetto_verticale(muri_definitivi[i].colore, pos);
            if (ruotato) result = 1;
        }
    }

    return result;
}


/*******************************************************************************
* Function Name  : controlla_blocco_giocatori
* Description    : Controlla che il posizionamento di un muro non blocchi nessun giocatore
									 dal poter raggiungere il traguardo
* Input          : 
* Return         : 1 se blocca, 0 altrimenti
*******************************************************************************/

int controlla_blocco_giocatori(uint32_t riga_muro, uint32_t colonna_muro, char orientamento_muro) {
    int blocca = 0; //=1 se blocca

    int riga_1 = posizione_player1[0]; // riga del giocatore1
    int colonna_1 = posizione_player1[1]; // colonna

    int riga_2 = posizione_player2[0]; // riga del giocatore2
    int colonna_2 = posizione_player2[1]; // colonna

    int caselle_bloccate[7][7] = { // griglia 7x7 =1 bloccata =-1 libera
        {-1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1}
    };
    // questa matrice mi serve pr memorizzare le caselle in cui ho già provato a trovare un percorso libero ma senza riuscirci

    uint16_t counter_left = 0; //quando sono arrivato già nei due estremi sinistra e destra, devo provare a tornare indietro (verso il basso per il giocatore1, verso l'alto per il giocatore1) per provare un altro percorso

    uint8_t estremo_destro = 0;
    uint8_t estremo_sinistro = 0;
    uint8_t estremo_basso = 0;

    uint16_t i = 0;
    uint16_t j = 0;

    // inserisco il muro che vorrebbe mettere il giocatore
    muri_definitivi[counter_muri].riga = riga_muro;
    muri_definitivi[counter_muri].colonna = colonna_muro;
    muri_definitivi[counter_muri].orientamento = orientamento_muro;
    muri_definitivi[counter_muri].colore = Red; // colore a caso
    counter_muri++;                                // dovrò diminuirla alla fine perché in caso non vada bene il muro le informazioni inserite andranno sovrascritte da altre

    while (!blocca) { // la riga 0 è la posizione vincente per il giocatore 1
        blocca = 1;

        if (!is_wall(riga_1 - 1, colonna_1, riga_1, colonna_1) && caselle_bloccate[riga_1 - 1][colonna_1] != 1) { // posso andare in alto verso il traguardo
            riga_1--;
            estremo_destro = 0;
            estremo_sinistro = 0;
						counter_left=0;
            blocca = 0; // se posso muovermi non sono ancora bloccato
        } else if (estremo_destro && estremo_sinistro && estremo_basso) { // ho provato tutte le strade possibili ma sono bloccato
            blocca = 1;
        } else if (!is_wall(riga_1, colonna_1 - 1, riga_1, colonna_1) && colonna_1 > 0 && estremo_sinistro != 1 && caselle_bloccate[riga_1][colonna_1 - 1] != 1) { // mi sposto a sinistra se non posso andare in alto
            //caselle_bloccate[riga_1][colonna_1] = 1; 
            colonna_1--;
            blocca = 0;
        } else {
            caselle_bloccate[riga_1][colonna_1] = 1;
            estremo_sinistro = 1;
            if (!is_wall(riga_1, colonna_1 + 1, riga_1, colonna_1) && colonna_1 < 6 && caselle_bloccate[riga_1][colonna_1 + 1] != 1) { // mi sposto a destra se non posso andare ne in alto ne a sinistra
								caselle_bloccate[riga_1][colonna_1] = 1;
                colonna_1++;
                blocca = 0;
            } else {
                estremo_destro = 1;
                if (!is_wall(riga_1 + 1, colonna_1, riga_1, colonna_1) && riga_1 < 6) { // mi sposto in basso se non posso andare in nessuna altra direzione
										caselle_bloccate[riga_1][colonna_1] = 1;
                    riga_1++;
                    blocca = 0;
                    estremo_destro = 0; // posso riprovare ad andare
                    estremo_sinistro = 0; // a destra e sinistra per raggiungere il traguardo
										counter_left=0;

                } else {
                    if(counter_left!=6 && colonna_1>0 && !is_wall(riga_1, colonna_1-1,riga_1,colonna_1)) { //controllo che ci siano possibilità di andare indietro andando a sinistra
											colonna_1--;
											counter_left++;
											blocca=0;
										}
										else estremo_basso=1;
                }
            }
        }
        if (riga_1 == 0) { // sono arrivato al traguardo quindi  quel muro non blocca il giocatore 1
            blocca = 0;
            break;
        }
    }

		//ripristino tutto per il controllo successivo
    estremo_destro = 0;
    estremo_sinistro = 0;
    estremo_basso = 0;
		counter_left=0;

    for (i = 0; i < 7; i++) {
        for (j = 0; j < 7; j++) {
            caselle_bloccate[i][j] = -1;
        }
    }

		//se blocca è già ad 1 questo controllo non verrà eseguito
		
    while (!blocca) { // la riga 6 è la posizione vincente per il giocatore 2
        blocca = 1;

        if (!is_wall(riga_2 + 1, colonna_2, riga_2, colonna_2) && caselle_bloccate[riga_2 + 1][colonna_2] != 1) { // posso andare in basso verso il traguardo
            riga_2++;
            estremo_destro = 0;
            estremo_sinistro = 0;
						counter_left=0;
            blocca = 0; // se posso muovermi non sono ancora bloccato
        } else if (estremo_destro && estremo_sinistro && estremo_basso) { // ho provato tutte le strade possibili ma sono bloccato
            blocca = 1;
        } else if (!is_wall(riga_2, colonna_2 - 1, riga_2, colonna_2) && colonna_2 > 0 && estremo_sinistro != 1 && caselle_bloccate[riga_2][colonna_2 - 1] != 1) { // mi sposto a sinistra se non posso andare in alto
						//caselle_bloccate[riga_2][colonna_2] = 1;
            colonna_2--;
            blocca = 0;
        } else {
            caselle_bloccate[riga_2][colonna_2] = 1;
            estremo_sinistro = 1;
            if (!is_wall(riga_2, colonna_2 + 1, riga_2, colonna_2) && colonna_2 < 6 && caselle_bloccate[riga_2][colonna_2 + 1] != 1) { // mi sposto a destra se non posso andare ne in alto ne a sinistra
                caselle_bloccate[riga_2][colonna_2] = 1;
                colonna_2++;
                blocca = 0;
            } else {
                estremo_destro = 1;
                if (!is_wall(riga_2 - 1, colonna_2, riga_2, colonna_2) && riga_2 > 0) { // mi sposto in alto se non posso andare in nessuna altra direzione
									caselle_bloccate[riga_2][colonna_2] = 1;
                    riga_2--;
                    blocca = 0;
                    estremo_destro = 0; // posso riprovare ad andare
                    estremo_sinistro = 0; // a destra e sinistra per raggiungere il traguardo
										counter_left=0;

                } else {
                      if(counter_left!=6 && colonna_2>0 && !is_wall(riga_2, colonna_2-1,riga_2,colonna_2)) {
											blocca=0;
											colonna_2--;
											counter_left++;
										}
										else estremo_basso=1;
                }
            }
        }
        if (riga_2 == 6) { // sono arrivato al traguardo quindi  quel muro non blocca il giocatore 2
            blocca = 0;
            break;
        }
    }

    counter_muri--;

    return blocca;
}


/*******************************************************************************
* Function Name  : registra_mossa
* Description    : Specifica nel testo
* Input          : player_id, move_wall, vertical_horizontal, y_coordinate, x_coordinate
* Return         : restituisce intero a 32 bit 
*******************************************************************************/

void registra_mossa(uint8_t player_id, uint8_t move_wall, uint8_t vertical_horizontal, uint8_t y_coordinate, uint8_t x_coordinate) {
	
    mossa = 0;

    mossa |= ((uint32_t)player_id << 24) & PLAYER_ID_MASK;
    mossa |= ((uint32_t)move_wall << 20) & PLAYER_MOVE_WALL_MASK;
    mossa |= ((uint32_t)vertical_horizontal << 16) & VERTICAL_HORIZONTAL_MASK;
    mossa |= ((uint32_t)y_coordinate << 8) & Y_COORDINATE_MASK;
    mossa |= (uint32_t)x_coordinate & X_COORDINATE_MASK;
	
	if(game_mode==2){ //modalità double board

		CAN_TxMsg.data[0] = player_id;
		CAN_TxMsg.data[1] = (move_wall << 4) | (vertical_horizontal & 0x0F);
		CAN_TxMsg.data[2] = y_coordinate;
		CAN_TxMsg.data[3] = x_coordinate;
		CAN_TxMsg.len = 4;
		CAN_TxMsg.id = ospite+1;
		CAN_TxMsg.format = STANDARD_FORMAT;
		CAN_TxMsg.type = DATA_FRAME;
		CAN_wrMsg (1, &CAN_TxMsg);               /* transmit message */
	}
}


/*******************************************************************************
* Function Name  : inizia_partita
* Description    : Fa tutte le cose necessarie per iniziare la partita in base alle scelte effettuate prima
* Input          : scelte effetuate nei due menu
* Return         : None
*******************************************************************************/

void inizia_partita(uint8_t game_mode, uint8_t opposite_player){ //inizializzo il gioco in base alla modalità ed all'avversario

		p1_wall = 8;
    p2_wall = 8;
		
		 create_griglia();
    if (ospite==0) 
        colora_caselle_vicine(posizione_player1, Yellow, turno);
    /*} else {
        colora_caselle_vicine(posizione_player2, Yellow, turno);
    }*/
    // timer per i secondi
    init_timer(1, 0x017D7840);
    enable_timer(1);

}


/*******************************************************************************
* Function Name  : mossa_npc
* Description    : in base alla posizione dell'avversario e alla propria l'npc fa la sua mossa
* Input          : posizione del giocatore, muri che rimangono al npc, posizione npc
* Return         : None
*******************************************************************************/

void mossa_npc(int* posizione_avversario, int p2_wall, int* posizione_npc){ 

  	static uint8_t muro_inserito_precedentemente_2=0;
		static uint8_t estremo_destro_2 = 0;
		static uint8_t estremo_sinistro_2 = 0;
	
		static uint8_t muro_inserito_precedentemente_1=0;
		static uint8_t estremo_destro_1 = 0;
		static uint8_t estremo_sinistro_1 = 0;
		
		uint8_t true=1;
	
		static int caselle_bloccate_2[7][7] = { // griglia 7x7 =1 bloccata =-1 libera
        {-1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1}
    };
		
		static int caselle_bloccate_1[7][7] = { // griglia 7x7 =1 bloccata =-1 libera
        {-1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1}
    };
		
    // questa matrice mi serve pr memorizzare le caselle in cui ho già provato a trovare un percorso libero ma senza riuscirci
	
		uint8_t riga_avversario=posizione_avversario[0];
		uint8_t colonna_avversario=posizione_avversario[1];
		uint8_t riga_npc=posizione_npc[0];
		uint8_t colonna_npc=posizione_npc[1];
		
		
if(	turno==0){ //player2npc
while(true){
		//implementare anche il fatto di evitare di andare sempre in un percorso bloccato in maniera ciclica
		if(riga_avversario>3 || muro_inserito_precedentemente_2){ //avversario ancora lontano dal traguardo
				if(!is_wall(riga_npc+1,colonna_npc,riga_npc,colonna_npc) && caselle_bloccate_2[riga_npc + 1][colonna_npc] != 1 && !(!controlla_casella_giocatore(posizione_avversario,riga_npc+1,colonna_npc) && riga_npc==5)){ //prima opzione cercare di andare dritto
						if(!controlla_casella_giocatore(posizione_avversario,riga_npc+1,colonna_npc) && riga_npc<5){ //scavalcare
								if(!is_wall(riga_avversario+1,colonna_avversario,riga_avversario,colonna_avversario)){ 
											mossa__npc.riga=riga_npc+2;
											mossa__npc.colonna=colonna_npc;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
											
								}
								else if(!is_wall(riga_avversario,colonna_avversario-1,riga_avversario,colonna_avversario)){ //movimento in diagonale
											mossa__npc.riga=riga_npc+1;
											mossa__npc.colonna=colonna_npc-1;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
											
								}
								else if(!is_wall(riga_avversario,colonna_avversario+1,riga_avversario,colonna_avversario)){ //movimento in diagonale
											mossa__npc.riga=riga_npc+1;
											mossa__npc.colonna=colonna_npc+1;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
											
								}
					}
					else{
								mossa__npc.riga=riga_npc+1;
								mossa__npc.colonna=colonna_npc;
								mossa__npc.muro=0;
								mossa__npc.orientamento='V'; //inutile
								
					}
						estremo_destro_2 = 0;
            estremo_sinistro_2 = 0;
				}
				else if(!is_wall(riga_npc,colonna_npc-1,riga_npc,colonna_npc) && colonna_npc>0 && estremo_sinistro_2 != 1 && caselle_bloccate_2[riga_npc][colonna_npc - 1] != 1 && !(!controlla_casella_giocatore(posizione_avversario,riga_npc,colonna_npc-1) && colonna_npc==1)){ //seconda opzione andare a sinistra
					if(!controlla_casella_giocatore(posizione_avversario,riga_npc,colonna_npc-1) && colonna_npc>1){ //scavalcare
								if(!is_wall(riga_avversario,colonna_avversario-1,riga_avversario,colonna_avversario)){ 
											mossa__npc.riga=riga_npc;
											mossa__npc.colonna=colonna_npc-2;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
											
								}
								else if(!is_wall(riga_avversario-1,colonna_avversario,riga_avversario,colonna_avversario)){ //movimento in diagonale
											mossa__npc.riga=riga_npc-1;
											mossa__npc.colonna=colonna_npc-1;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
										
								}
								else if(!is_wall(riga_avversario+1,colonna_avversario,riga_avversario,colonna_avversario)){ //movimento in diagonale
											mossa__npc.riga=riga_npc+1;
											mossa__npc.colonna=colonna_npc-1;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
											
								}
					}
					else{
								mossa__npc.riga=riga_npc;
								mossa__npc.colonna=colonna_npc-1;
								mossa__npc.muro=0;
								mossa__npc.orientamento='V'; //inutile
					}				
				}
				else if(!is_wall(riga_npc,colonna_npc+1,riga_npc,colonna_npc) && colonna_npc<6 && estremo_destro_2!=1 && caselle_bloccate_2[riga_npc][colonna_npc + 1] != 1 && !(!controlla_casella_giocatore(posizione_avversario,riga_npc,colonna_npc+1) && colonna_npc==5)){ //terza opzione andare a destra
						caselle_bloccate_2[riga_npc][colonna_npc] = 1;
            estremo_sinistro_2 = 1;
					if(!controlla_casella_giocatore(posizione_avversario,riga_npc,colonna_npc+1) && colonna_npc<5){ //scavalcare
								if(!is_wall(riga_avversario,colonna_avversario+1,riga_avversario,colonna_avversario)){ 
											mossa__npc.riga=riga_npc;
											mossa__npc.colonna=colonna_npc+2;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
								}
								else if(!is_wall(riga_avversario-1,colonna_avversario,riga_avversario,colonna_avversario)){ //movimento in diagonale
											mossa__npc.riga=riga_npc-1;
											mossa__npc.colonna=colonna_npc+1;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
								}
								else if(!is_wall(riga_avversario+1,colonna_avversario,riga_avversario,colonna_avversario)){ //movimento in diagonale
											mossa__npc.riga=riga_npc+1;
											mossa__npc.colonna=colonna_npc+1;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
								}
					}
					else{
								mossa__npc.riga=riga_npc;
								mossa__npc.colonna=colonna_npc+1;
								mossa__npc.muro=0;
								mossa__npc.orientamento='V'; //inutile
					}				
				}
				else if(!is_wall(riga_npc-1,colonna_npc,riga_npc,colonna_npc) && riga_npc>0 && !(!controlla_casella_giocatore(posizione_avversario,riga_npc-1,colonna_npc) && riga_npc==1)){ //quarta opzione ritorno indietro
							caselle_bloccate_2[riga_npc][colonna_npc] = 1;
							estremo_destro_2 = 1;
						if(!controlla_casella_giocatore(posizione_avversario,riga_npc-1,colonna_npc) && riga_npc>1){ //scavalcare
								if(!is_wall(riga_avversario-1,colonna_avversario,riga_avversario,colonna_avversario)){ 
											mossa__npc.riga=riga_npc-2;
											mossa__npc.colonna=colonna_npc;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
											
								}
								else if(!is_wall(riga_avversario,colonna_avversario-1,riga_avversario,colonna_avversario)){ //movimento in diagonale
											mossa__npc.riga=riga_npc-1;
											mossa__npc.colonna=colonna_npc-1;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
											
								}
								else if(!is_wall(riga_avversario,colonna_avversario+1,riga_avversario,colonna_avversario)){ //movimento in diagonale
											mossa__npc.riga=riga_npc-1;
											mossa__npc.colonna=colonna_npc+1;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
											
								}
					}
					else{
								mossa__npc.riga=riga_npc-1;
								mossa__npc.colonna=colonna_npc;
								mossa__npc.muro=0;
								mossa__npc.orientamento='V'; //inutile
					}
				estremo_destro_2 = 0; // posso riprovare ad andare
        estremo_sinistro_2 = 0; // a destra e sinistra per raggiungere il traguardo
				}
				if(muro_inserito_precedentemente_2) muro_inserito_precedentemente_2--;
				return;
		}
		else if(p2_wall>0){ //metto un muro
					if(colonna_avversario==6) colonna_avversario--;
						posizione_wall[0]=riga_avversario-1;
						posizione_wall[1]=colonna_avversario;
					if(controlla_presenza_orizzontale()==0 && controlla_blocco_giocatori(riga_avversario-1, colonna_avversario,'H')==0 && 	controlla_presenza_verticale()==0){ //posso mettere quel muro
								mossa__npc.riga=riga_avversario-1;
								mossa__npc.colonna=colonna_avversario;
								mossa__npc.muro=1;
								mossa__npc.orientamento='H';
								muro_inserito_precedentemente_2=2;
								return;
					}
					muro_inserito_precedentemente_2=2;
		}
		else{
			muro_inserito_precedentemente_2=2;
		}
}
}
else{ //player1npc
	
while(true){
		//implementare anche il fatto di evitare di andare sempre in un percorso bloccato in maniera ciclica
		if(riga_avversario<3 || muro_inserito_precedentemente_1){ //avversario ancora lontano dal traguardo
				if(!is_wall(riga_npc-1,colonna_npc,riga_npc,colonna_npc) && caselle_bloccate_1[riga_npc - 1][colonna_npc] != 1 && !(!controlla_casella_giocatore(posizione_avversario,riga_npc-1,colonna_npc) && riga_npc==1)){ //prima opzione cercare di andare dritto
						if(!controlla_casella_giocatore(posizione_avversario,riga_npc-1,colonna_npc)){ //scavalcare
								if(!is_wall(riga_avversario-1,colonna_avversario,riga_avversario,colonna_avversario) && riga_npc<5){ 
											mossa__npc.riga=riga_npc-2;
											mossa__npc.colonna=colonna_npc;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
											
								}
								else if(!is_wall(riga_avversario,colonna_avversario-1,riga_avversario,colonna_avversario)){ //movimento in diagonale
											mossa__npc.riga=riga_npc-1;
											mossa__npc.colonna=colonna_npc-1;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
											
								}
								else if(!is_wall(riga_avversario,colonna_avversario+1,riga_avversario,colonna_avversario)){ //movimento in diagonale
											mossa__npc.riga=riga_npc-1;
											mossa__npc.colonna=colonna_npc+1;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
											
								}
					}
					else{
								mossa__npc.riga=riga_npc-1;
								mossa__npc.colonna=colonna_npc;
								mossa__npc.muro=0;
								mossa__npc.orientamento='V'; //inutile
								
					}
						estremo_destro_1 = 0;
            estremo_sinistro_1 = 0;
				}
				else if(!is_wall(riga_npc,colonna_npc-1,riga_npc,colonna_npc) && colonna_npc>0 && estremo_sinistro_1 != 1 && caselle_bloccate_1[riga_npc][colonna_npc - 1] != 1 && !(!controlla_casella_giocatore(posizione_avversario,riga_npc,colonna_npc-1) && colonna_npc==1)){ //seconda opzione andare a sinistra
					if(!controlla_casella_giocatore(posizione_avversario,riga_npc,colonna_npc-1) && colonna_npc>1){ //scavalcare
								if(!is_wall(riga_avversario,colonna_avversario-1,riga_avversario,colonna_avversario)){ 
											mossa__npc.riga=riga_npc;
											mossa__npc.colonna=colonna_npc-2;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
											
								}
								else if(!is_wall(riga_avversario-1,colonna_avversario,riga_avversario,colonna_avversario)){ //movimento in diagonale
											mossa__npc.riga=riga_npc-1;
											mossa__npc.colonna=colonna_npc-1;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
										
								}
								else if(!is_wall(riga_avversario+1,colonna_avversario,riga_avversario,colonna_avversario)){ //movimento in diagonale
											mossa__npc.riga=riga_npc+1;
											mossa__npc.colonna=colonna_npc-1;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
											
								}
					}
					else{
								mossa__npc.riga=riga_npc;
								mossa__npc.colonna=colonna_npc-1;
								mossa__npc.muro=0;
								mossa__npc.orientamento='V'; //inutile
					}				
				}
				else if(!is_wall(riga_npc,colonna_npc+1,riga_npc,colonna_npc) && colonna_npc<6 && estremo_destro_1!=1 && caselle_bloccate_1[riga_npc][colonna_npc + 1] != 1 && !(!controlla_casella_giocatore(posizione_avversario,riga_npc,colonna_npc+1) && colonna_npc==5)){ //terza opzione andare a destra
						caselle_bloccate_1[riga_npc][colonna_npc] = 1;
            estremo_sinistro_1 = 1;
					if(!controlla_casella_giocatore(posizione_avversario,riga_npc,colonna_npc+1) && colonna_npc<5){ //scavalcare
								if(!is_wall(riga_avversario,colonna_avversario+1,riga_avversario,colonna_avversario)){ 
											mossa__npc.riga=riga_npc;
											mossa__npc.colonna=colonna_npc+2;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
								}
								else if(!is_wall(riga_avversario-1,colonna_avversario,riga_avversario,colonna_avversario)){ //movimento in diagonale
											mossa__npc.riga=riga_npc-1;
											mossa__npc.colonna=colonna_npc+1;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
								}
								else if(!is_wall(riga_avversario+1,colonna_avversario,riga_avversario,colonna_avversario)){ //movimento in diagonale
											mossa__npc.riga=riga_npc+1;
											mossa__npc.colonna=colonna_npc+1;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
								}
					}
					else{
								mossa__npc.riga=riga_npc;
								mossa__npc.colonna=colonna_npc+1;
								mossa__npc.muro=0;
								mossa__npc.orientamento='V'; //inutile
					}				
				}
				else if(!is_wall(riga_npc+1,colonna_npc,riga_npc,colonna_npc) && riga_npc>0 && !(!controlla_casella_giocatore(posizione_avversario,riga_npc+1,colonna_npc) && riga_npc==5)){ //quarta opzione ritorno indietro
							caselle_bloccate_1[riga_npc][colonna_npc] = 1;
							estremo_destro_1 = 1;
						if(!controlla_casella_giocatore(posizione_avversario,riga_npc-1,colonna_npc) && riga_npc<5){ //scavalcare
								if(!is_wall(riga_avversario+1,colonna_avversario,riga_avversario,colonna_avversario)){ 
											mossa__npc.riga=riga_npc+2;
											mossa__npc.colonna=colonna_npc;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
											
								}
								else if(!is_wall(riga_avversario,colonna_avversario-1,riga_avversario,colonna_avversario)){ //movimento in diagonale
											mossa__npc.riga=riga_npc+1;
											mossa__npc.colonna=colonna_npc-1;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
											
								}
								else if(!is_wall(riga_avversario,colonna_avversario+1,riga_avversario,colonna_avversario)){ //movimento in diagonale
											mossa__npc.riga=riga_npc+1;
											mossa__npc.colonna=colonna_npc+1;
											mossa__npc.muro=0;
											mossa__npc.orientamento='V'; //inutile
											
								}
					}
					else{
								mossa__npc.riga=riga_npc+1;
								mossa__npc.colonna=colonna_npc;
								mossa__npc.muro=0;
								mossa__npc.orientamento='V'; //inutile
					}
				estremo_destro_1 = 0; // posso riprovare ad andare
        estremo_sinistro_1 = 0; // a destra e sinistra per raggiungere il traguardo
				}
				if (muro_inserito_precedentemente_1) muro_inserito_precedentemente_1--;
				return;
		}
		else if(p2_wall>0){ //metto un muro
				if(colonna_avversario==6) colonna_avversario--;
						posizione_wall[0]=riga_avversario;
						posizione_wall[1]=colonna_avversario;
					if(controlla_presenza_orizzontale()==0 && controlla_blocco_giocatori(riga_avversario, colonna_avversario,'H')==0 && 	controlla_presenza_verticale()==0){ //posso mettere quel muro
								mossa__npc.riga=riga_avversario;
								mossa__npc.colonna=colonna_avversario;
								mossa__npc.muro=1;
								mossa__npc.orientamento='H';
								muro_inserito_precedentemente_1=2;
								return;
					}
					muro_inserito_precedentemente_1=2;
			}
		else{
					muro_inserito_precedentemente_1=2;
			}
		}
	}	
}





void effettua_mossa(uint32_t mossa){
	
		char str[3];
	
		uint8_t player_id = (mossa >> 24) & 0xFF;
		uint8_t move_wall = (mossa >> 20) & 0x0F;
		uint8_t vertical_horizontal = (mossa >> 16) & 0x0F;
		uint8_t y_coordinate = (mossa >> 8) & 0xFF;
		uint8_t x_coordinate = mossa & 0xFF;
	
		
		 if(move_wall == 0 && vertical_horizontal == 0){ //moving the player
				if(player_id == 1){ //player 1
							colora_quadrato(posizione_player1[0], posizione_player1[1], White);
							colora_caselle_vicine(posizione_player1,White, player_id);
							posizione_player1[0]=y_coordinate;
							posizione_player1[1]=x_coordinate;
							muovi_giocatore(posizione_player1[0], posizione_player1[1], player_id);
							mosso=1;
				}
				else{ //player 2
							colora_quadrato(posizione_player2[0], posizione_player2[1], White);
							colora_caselle_vicine(posizione_player2,White, player_id);
							posizione_player2[0]=y_coordinate;
							posizione_player2[1]=x_coordinate;
							muovi_giocatore(posizione_player2[0], posizione_player2[1], player_id);
							mosso=1;					
				}
			}
			else if(move_wall == 0 && vertical_horizontal == 1){ //no play
							mosso=1;
			}
			else if(move_wall == 1){ //placing a wall
							if(vertical_horizontal){ //orizzontale
								posizione_wall[0] = y_coordinate;
								posizione_wall[1] = x_coordinate;
								
								if(!player_id)		create_wall(Red);
								else					create_wall(Black);
								
								muri_definitivi[counter_muri].riga=posizione_wall[0];
								muri_definitivi[counter_muri].colonna=posizione_wall[1];
					
								if (player_id)	muri_definitivi[counter_muri].colore=Black;
								else 				muri_definitivi[counter_muri].colore=Red;
					
								muri_definitivi[counter_muri].orientamento='H';
								counter_muri++;
								
								if(player_id){ 
														p1_wall--;
														sprintf(str, "%d", p1_wall);
														GUI_Text(35, 290,  (uint8_t *) str,Black,White);
								}
								else{
														p2_wall--;
														sprintf(str, "%d", p2_wall);
														GUI_Text(191, 290,  (uint8_t *) str,Black,White);
								}
								mosso=1;
								
							}
							else{ //verticale
								posizione_wall[0] = y_coordinate;
								posizione_wall[1] = x_coordinate;
								
								if(!player_id)		ruota_muro_90(Red);
								else					ruota_muro_90(Black);
								
								muri_definitivi[counter_muri].riga=posizione_wall[0];
								muri_definitivi[counter_muri].colonna=posizione_wall[1];
					
								if (player_id)	muri_definitivi[counter_muri].colore=Black;
								else 						muri_definitivi[counter_muri].colore=Red;
					
								muri_definitivi[counter_muri].orientamento='V';
								counter_muri++;
								
								if(player_id){ 
														p1_wall--;
														sprintf(str, "%d", p1_wall);
														GUI_Text(35, 290,  (uint8_t *) str,Black,White);
								}
								else{
														p2_wall--;
														sprintf(str, "%d", p2_wall);
														GUI_Text(191, 290,  (uint8_t *) str,Black,White);
								}
								mosso=1;
								
							}
			}
}
