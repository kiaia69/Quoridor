/**--------------File Info-------------------------------------------------------------------------------
** File name:			drawing.c
** Descriptions:	implementa le funzioni che disegnano sul display
**------------------------------------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "drawing.h"
#include "GLCD/GLCD.h" 
#include "joystick/joystick.h"
#include "RIT/RIT.h"
#include "timer/timer.h"
#include "gioco.h"
#include <stdio.h>




/* Private variables ---------------------------------------------------------*/
static uint8_t k=32; //costante che separa i vari quadrati della griglia
static uint8_t size = 7; //griglia 7x7
static uint8_t num_quadranti=3;
static uint8_t num_bottoni=2;


/* External variables ---------------------------------------------------------*/
extern int secondi;
extern int p1_wall;
extern int p2_wall;
extern int posizione_player2[2]; 
extern int posizione_player1[2];
extern int posizione_wall[2]; 
extern int game_mode;


/*******************************************************************************
* Function Name  : colora_quadrato
* Description    : Colora una specifica casella della griglia con il colore specificato
* Input          : riga e colonna della casella ed il colore
* Return         : None
*******************************************************************************/

void colora_quadrato(uint16_t riga, uint16_t colonna, uint16_t colore) {

		uint16_t lato_quadrato = 24;

    // Calcola le coordinate del punto in alto a sinistra del quadrato
    uint16_t x = 11 + (k * colonna);
    uint16_t y = 11 + (k * riga);

    uint16_t i = x;
    uint16_t j = y;

    // Colora il quadrato utilizzando la funzione LCD_SetPoint
    for (; i < x + lato_quadrato; i++) {
        for (; j < y + lato_quadrato; j++) {
            LCD_SetPoint(i, j, colore);
        }
        j = y;
    }
}


/*******************************************************************************
* Function Name  : create_griglia
* Description    : Disegna tutto il necessario per iniziare il gioco
* Input          : None
* Return         : None
*******************************************************************************/

void create_griglia(void) {
    uint16_t riga;
    uint16_t colonna;
    char str[3];
    uint8_t i = 0;
    k=32;
    LCD_Clear(White);
	
		// disegno la griglia
    for (colonna = 0; colonna < size; colonna++) {
        for (riga = 0; riga < size; riga++) {
            
            LCD_DrawLine(10 + (k * colonna), 10 + (k * riga), 10 + (k * colonna), 35 + (k * riga), Black);  // |
            LCD_DrawLine(10 + (k * colonna), 35 + (k * riga), 35 + (k * colonna), 35 + (k * riga), Black);  // |_
            LCD_DrawLine(35 + (k * colonna), 35 + (k * riga), 35 + (k * colonna), 10 + (k * riga), Black);  // |_|
            LCD_DrawLine(35 + (k * colonna), 10 + (k * riga), 10 + (k * colonna), 10 + (k * riga), Black);  // [ ]
        }
    }

    k = 77;

    // disegno i quadranti
    for (i = 0; i < num_quadranti; i++) {
        LCD_DrawLine(10 + (k * i), 260, 10 + (k * i), 310, Black);
        LCD_DrawLine(10 + (k * i), 310, 74 + (k * i), 310, Black);
        LCD_DrawLine(74 + (k * i), 310, 74 + (k * i), 260, Black);
        LCD_DrawLine(74 + (k * i), 260, 10 + (k * i), 260, Black);
    }
		
		k = 32,

    GUI_Text(15, 270, (uint8_t *)"P1 WALL", Black, White);
    GUI_Text(170, 270, (uint8_t *)"P2 WALL", Black, White);

    sprintf(str, "%d", p1_wall);
    GUI_Text(35, 290, (uint8_t *)str, Black, White);

    sprintf(str, "%d", p2_wall);
    GUI_Text(191, 290, (uint8_t *)str, Black, White);

    // disegno le pedine
    colora_quadrato(6, 3, Black); // player 1
    colora_quadrato(0, 3, Red);   // player 2

    GUI_Text(70, 240, (uint8_t *)"TURNO: PLAYER1", Black, White);

    sprintf(str, "%d s", secondi);
    GUI_Text(103, 275, (uint8_t *)str, Black, White);
}


/*******************************************************************************
* Function Name  : menu_game_mode
* Description    : Disegna il menu per scegliere la modalità di gioco
* Input          : None
* Return         : None
*******************************************************************************/

void menu_game_mode(void) {
    uint8_t i = 0;
    k = 60;
    LCD_Clear(White);

    GUI_Text(70, 30, (uint8_t *)"Select the", Black, White);
    GUI_Text(73, 44, (uint8_t *)"GAME MODE", Green, White);

    // Disegno i bottoni e le frecce
    for (i = 0; i < num_bottoni; i++) {
          LCD_DrawLine(50, 120 + (k * i), 50, 160 + (k * i), Red);
        LCD_DrawLine(50, 160 + (k * i), 170, 160 + (k * i), Red);
        LCD_DrawLine(170, 160 + (k * i), 170, 120 + (k * i), Red);
        LCD_DrawLine(170, 120 + (k * i), 50, 120 + (k * i), Red);
    }
		
		GUI_Text(20, 130, (uint8_t *)"-->", Black, White);
    GUI_Text(60, 130, (uint8_t *)"Single Board", Black, White);
    GUI_Text(60, 190, (uint8_t *)"Two Boards", Black, White);
}


/*******************************************************************************
* Function Name  : menu_opposite_player
* Description    : Disegna il menu per scegliere l'avversario con il quale giocare
* Input          : None
* Return         : None
*******************************************************************************/

void menu_opposite_player(void) {
    uint8_t i = 0;
    k = 60;
    LCD_Clear(White);

		if(game_mode){ //single board
			GUI_Text(50, 30, (uint8_t *)"Single Board: select", Black, White);
			GUI_Text(50, 44, (uint8_t *)"the opposite player", Black, White);
		}
		else{
			GUI_Text(60, 30, (uint8_t *)"Two Boards: select", Black, White);
			GUI_Text(65, 44, (uint8_t *)"your player", Black, White);
		}

    // Disegno i bottoni e le frecce
    for (i = 0; i < num_bottoni; i++) {
          LCD_DrawLine(50, 120 + (k * i), 50, 160 + (k * i), Red);
        LCD_DrawLine(50, 160 + (k * i), 170, 160 + (k * i), Red);
        LCD_DrawLine(170, 160 + (k * i), 170, 120 + (k * i), Red);
        LCD_DrawLine(170, 120 + (k * i), 50, 120 + (k * i), Red);
    }
		
		GUI_Text(20, 130, (uint8_t *)"-->", Black, White);
    GUI_Text(90, 130, (uint8_t *)"Human", Black, White);
    GUI_Text(95, 190, (uint8_t *)"NPC", Black, White);
}
	
	
	

/*******************************************************************************
* Function Name  : create_wall
* Description    : Disegna un muro orizzontale in base alla posizione_wall
* Input          : colore del muro (se bianco lo cancella)
* Return         : None
*******************************************************************************/

void create_wall(uint16_t colore) {
  
    uint16_t lato_quadrato = 24;
    uint16_t i;
    uint16_t j;

    for (j = (k * 2) + lato_quadrato * 1.55 + (posizione_wall[0]-2) * k; j < (k * 3) + (lato_quadrato / 2) - 2 + (posizione_wall[0]-2) * k; j++) {
        for (i = (k * 2) + 16 + lato_quadrato + (posizione_wall[1]-3) * k; i < (k * 3 - 2) + lato_quadrato * 3 + (posizione_wall[1]-3) * k; i++) {
            LCD_SetPoint(i, j, colore);
        }
    }
}


/*******************************************************************************
* Function Name  : ruota_muro_90
* Description    : Disegna un muro verticale in base alla posizione_wall
* Input          : colore del muro (se bianco lo cancella)
* Return         : None
*******************************************************************************/

void ruota_muro_90(uint16_t colore){

		uint16_t lato_quadrato = 24;
    uint16_t i;
    uint16_t j;
	
	
	 for (j = (k * 3) + lato_quadrato * 1.55+ (posizione_wall[1]-3)*k; j < (k * 4) + (lato_quadrato / 2) - 2+ (posizione_wall[1]-3)*k; j++) {
        for (i = (k * 1) + 16 + lato_quadrato + (posizione_wall[0]-2)*k; i < (k * 2 - 2) + lato_quadrato * 3 + (posizione_wall[0]-2)*k; i++) {
            LCD_SetPoint(j, i, colore);
        }
					}
}


/*******************************************************************************
* Function Name  : disegna_pezzetto_orizzontale
* Description    : Utile per non far cancellare muri orizzontali già posizionati in caso un muro
									 che sta venendo mosso gli passi sopra
* Input          : colore del muro, posizione del muro (riga e colonna)
* Return         : None
*******************************************************************************/

void disegna_pezzetto_orizzontale(uint16_t colore, int *pos){

		uint16_t lato_quadrato = 24;
    uint16_t i;
    uint16_t j;
	

    for (j = (k * 2) + lato_quadrato * 1.55+ (pos[0]-2)*k; j < (k * 3) + (lato_quadrato / 2) - 2+ (pos[0]-2)*k; j++) {
        for (i = (k * 2) + 16 + lato_quadrato + (pos[1]-3)*k; i < (k * 3 - 2) + lato_quadrato * 3 + (pos[1]-3)*k; i++) {
            LCD_SetPoint(i, j, colore);
        }		
	}
}


/*******************************************************************************
* Function Name  : disegna_pezzetto_verticale
* Description    : Utile per non far cancellare muri verticali già posizionati in caso un muro
									 che sta venendo mosso gli passi sopra
* Input          : colore del muro, posizione del muro (riga e colonna)
* Return         : None
*******************************************************************************/
	

void disegna_pezzetto_verticale(uint16_t colore, int *pos){
	  
    uint16_t lato_quadrato = 24;
    uint16_t i;
    uint16_t j;
	

	 for (j = (k * 3) + lato_quadrato * 1.55+ (pos[1]-3)*k; j < (k * 4) + (lato_quadrato / 2) - 2+ (pos[1]-3)*k; j++) {
        for (i = (k * 1) + 16 + lato_quadrato + (pos[0]-2)*k; i < (k * 2 - 2) + lato_quadrato * 3 + (pos[0]-2)*k; i++) {
            LCD_SetPoint(j, i, colore);
        }
		}
	
}


/*******************************************************************************
* Function Name  : muovi_giocatore
* Description    : Muove un giocatore
* Input          : riga e colonna della posizione futura, turno
* Return         : None
*******************************************************************************/

void muovi_giocatore(uint16_t riga, uint16_t colonna, uint16_t turno){
	
	if(turno){ //non posso muovermi in una casella già occupata
			colora_quadrato(riga, colonna, Black);
			is_finished(posizione_player1, turno);
	}
	else {		//non posso muovermi in una casella già occupata
			colora_quadrato(riga, colonna, Red);
			is_finished(posizione_player2, turno);
	}
}


/*******************************************************************************
* Function Name  : colora_caselle_vicine
* Description    : Colora le caselle adiacenti alla casella del giocatore che non siano bloccate da un muro.
									 Colora anche la casella più lontana in caso in cui ci sia la possibiltà di scavalcare un gicatore
* Input          : posizione attuale del giocatore, colore (giallo o bianco), turno
* Return         : None
*******************************************************************************/

void colora_caselle_vicine( int* pos, uint16_t colore, int turno) {
	
	int riga = pos[0];   //posizione attuale del
	int colonna = pos[1];//giocatore
	
	
	//effettuo i controlli affinché non venga evidenziata una casella in cui è presente l'altro player
	//implementare anche lo scavalcamento dell'altro player se non c'è un muro dietro
	//la logica è controllare se dietro la pedina che devo scavalcare c'è un muro
	
	
	if(riga!=6 && riga!=0 && colonna!=0 && colonna !=6){ //casella non particolare
		if(!(is_wall(riga+1, colonna, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga+1, colonna) && controlla_casella_giocatore(posizione_player2, riga+1, colonna)) colora_quadrato(riga+1, colonna, colore);
		else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0]+1, posizione_player1[1]) && posizione_player2[0]!=6) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0]+1, posizione_player2[1]) && posizione_player1[0]!=6))){
			if((turno && !(is_wall(posizione_player2[0]+1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]+1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga+2, colonna, colore);
			}
			else{ //coloro le diagonali se possibile
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]-1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]-1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna-1,colore);
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]+1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]+1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna+1,colore);
			}
		}
	}
		
		if(!(is_wall(riga, colonna+1, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga, colonna+1) && controlla_casella_giocatore(posizione_player2, riga, colonna+1)) colora_quadrato(riga, colonna+1, colore);
		else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]+1) && posizione_player2[1]!=6) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]+1) && posizione_player1[1]!=6))){
			if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]+1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]+1,posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga, colonna+2, colore);
			}
			else{ //coloro le diagonali se possibile
				if((turno && !(is_wall(posizione_player2[0]-1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]-1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna+1,colore);
				if((turno && !(is_wall(posizione_player2[0]+1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]+1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna+1,colore);
			}
		}
	}
		
		if(!(is_wall(riga-1, colonna, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga-1, colonna) && controlla_casella_giocatore(posizione_player2, riga-1, colonna)) colora_quadrato(riga-1, colonna, colore);
		else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0]-1, posizione_player1[1]) && posizione_player2[0]!=0) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0]-1, posizione_player2[1]) && posizione_player1[0]!=0))){
			if((turno && !(is_wall(posizione_player2[0]-1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]-1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga-2, colonna, colore);
			}
			else{ //coloro le diagonali se possibile
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]-1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]-1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna-1,colore);
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]+1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]+1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna+1,colore);
			}
		}
	}
		
		if(!(is_wall(riga, colonna-1, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga, colonna-1) && controlla_casella_giocatore(posizione_player2, riga, colonna-1))colora_quadrato(riga, colonna-1, colore);		
		else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]-1) && posizione_player2[1]!=0) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]-1) && posizione_player1[1]!=0))){
			if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]-1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]-1,posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga, colonna-2, colore);
			}
			else{ //coloro le diagonali se possibile
				if((turno && !(is_wall(posizione_player2[0]-1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]-1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna-1,colore);
				if((turno && !(is_wall(posizione_player2[0]+1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]+1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna-1,colore);
			}
		}
	}
}
	
	if(riga==6 && colonna==6){ //nell'angolo destro in basso
		
		if(!(is_wall(riga-1, colonna, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga-1, colonna) && controlla_casella_giocatore(posizione_player2, riga-1, colonna)) colora_quadrato(riga-1, colonna, colore);
		else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0]-1, posizione_player1[1]) && posizione_player2[0]!=0) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0]-1, posizione_player2[1]) && posizione_player1[0]!=0))){
			if((turno && !(is_wall(posizione_player2[0]-1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]-1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga-2, colonna, colore);
			}
			else{ //coloro la diagonale se possibile
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]-1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]-1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna-1,colore);
			}
		}
	}
		
		if(!(is_wall(riga, colonna-1, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga, colonna-1) && controlla_casella_giocatore(posizione_player2, riga, colonna-1)) colora_quadrato(riga, colonna-1, colore);
		else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]-1) && posizione_player2[1]!=0) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]-1) && posizione_player1[1]!=0))){
			if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]-1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]-1,posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga, colonna-2, colore);
			}
			else{ //coloro la diagonale se possibile
				if((turno && !(is_wall(posizione_player2[0]-1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]-1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna-1,colore);
			}
		}
	}
}

	if(riga==6 && colonna==0){ //nell'angolo sinistro in basso
		
		if(!(is_wall(riga-1, colonna, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga-1, colonna) && controlla_casella_giocatore(posizione_player2, riga-1, colonna)) colora_quadrato(riga-1, colonna, colore);
		else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0]-1, posizione_player1[1]) && posizione_player2[0]!=0) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0]-1, posizione_player2[1]) && posizione_player1[0]!=0))){
			if((turno && !(is_wall(posizione_player2[0]-1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]-1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga-2, colonna, colore);
			}
		else{ //coloro la diagonale se possibile
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]+1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]+1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna+1,colore);
			}
		}
	}
		
		if(!(is_wall(riga, colonna+1, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga, colonna+1) && controlla_casella_giocatore(posizione_player2, riga, colonna+1)) colora_quadrato(riga, colonna+1, colore);
		else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]+1) && posizione_player2[1]!=6) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]+1) && posizione_player1[1]!=6))){
			if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]+1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]+1,posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga, colonna+2, colore);
			}
			else{ //coloro la diagonale se possibile
				if((turno && !(is_wall(posizione_player2[0]-1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]-1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna+1,colore);
			}
		}
	}
}
	
	if(riga==0 && colonna==6){ //nell'angolo destro in alto
		
		if(!(is_wall(riga+1, colonna, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga+1, colonna) && controlla_casella_giocatore(posizione_player2, riga+1, colonna)) colora_quadrato(riga+1, colonna, colore);
		else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0]+1, posizione_player1[1]) && posizione_player2[0]!=6) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0]+1, posizione_player2[1]) && posizione_player1[0]!=6))){
			if((turno && !(is_wall(posizione_player2[0]+1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]+1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga+2, colonna, colore);
			}
			else{ //coloro la diagonale se possibile
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]-1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]-1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna-1,colore);
			}
		}
	}
		
		if(!(is_wall(riga, colonna-1, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga, colonna-1) && controlla_casella_giocatore(posizione_player2, riga, colonna-1)) colora_quadrato(riga, colonna-1, colore);
		else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]-1) && posizione_player2[1]!=0) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]-1) && posizione_player1[1]!=0))){
			if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]-1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]-1,posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga, colonna-2, colore);
			}
			else{ //coloro la diagonale se possibile
				if((turno && !(is_wall(posizione_player2[0]+1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]+1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna-1,colore);
			}
		}
	}
}
	
	if(riga==0 && colonna==0){ //nell'angolo sinistro in alto
		
		if(!(is_wall(riga+1, colonna, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga+1, colonna) && controlla_casella_giocatore(posizione_player2, riga+1, colonna)) colora_quadrato(riga+1, colonna, colore);
		else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0]+1, posizione_player1[1]) && posizione_player2[0]!=6) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0]+1, posizione_player2[1]) && posizione_player1[0]!=6))){
			if((turno && !(is_wall(posizione_player2[0]+1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]+1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga+2, colonna, colore);
			}
			else{ //coloro la diagonale se possibile
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]+1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]+1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna+1,colore);
			}
		}
	}
		if(!(is_wall(riga, colonna+1, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga, colonna+1) && controlla_casella_giocatore(posizione_player2, riga, colonna+1)) colora_quadrato(riga, colonna+1, colore);
		else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]+1) && posizione_player2[1]!=6) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]+1) && posizione_player1[1]!=6))){
			if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]+1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]+1,posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga, colonna+2, colore);
			}
			else{ //coloro la diagonale se possibile
				if((turno && !(is_wall(posizione_player2[0]+1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]+1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna+1,colore);
			}
		}
	}
}
	
	if(riga==6 && colonna!=0 && colonna!=6){ //ultima riga ma non negli angoli
		
		if(!(is_wall(riga-1, colonna, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga-1, colonna) && controlla_casella_giocatore(posizione_player2, riga-1, colonna)) colora_quadrato(riga-1, colonna, colore);
		else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0]-1, posizione_player1[1]) && posizione_player2[0]!=0) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0]-1, posizione_player2[1]) && posizione_player1[0]!=0))){
			if((turno && !(is_wall(posizione_player2[0]-1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]-1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga-2, colonna, colore);
			}
			else{ //coloro le diagonali se possibile
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]-1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]-1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna-1,colore);
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]+1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]+1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna+1,colore);
				}
		}
	}
		
		if(!(is_wall(riga, colonna-1, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga, colonna-1) && controlla_casella_giocatore(posizione_player2, riga, colonna-1)) colora_quadrato(riga, colonna-1, colore);
		else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]-1) && posizione_player2[1]!=0) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]-1) && posizione_player1[1]!=0))){
			if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]-1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]-1,posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga, colonna-2, colore);
			}
			else{ //coloro le diagonali se possibile
				if((turno && !(is_wall(posizione_player2[0]-1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]-1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna-1,colore);
				if((turno && !(is_wall(posizione_player2[0]+1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]+1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna-1,colore);
			}
		}
	}
			
		if(!(is_wall(riga+1, colonna, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga, colonna+1) && controlla_casella_giocatore(posizione_player2, riga+1, colonna+1)) colora_quadrato(riga, colonna+1, colore);
		else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]+1) && posizione_player2[1]!=6) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]+1) && posizione_player1[1]!=6))){
			if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]+1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]+1,posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga, colonna+2, colore);
			}
			else{ //coloro le diagonali se possibile
				if((turno && !(is_wall(posizione_player2[0]-1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]-1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna+1,colore);
				if((turno && !(is_wall(posizione_player2[0]+1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]+1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna+1,colore);
			}
		}
	}
}
	
	if(riga==0 && colonna!=0 && colonna!=6){ //prima riga ma non negli angoli
		
		if(!(is_wall(riga+1, colonna, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga+1, colonna) && controlla_casella_giocatore(posizione_player2, riga+1, colonna)) colora_quadrato(riga+1, colonna, colore);
		else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0]+1, posizione_player1[1]) && posizione_player2[0]!=6) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0]+1, posizione_player2[1]) && posizione_player1[0]!=6))){
			if((turno && !(is_wall(posizione_player2[0]+1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]+1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga+2, colonna, colore);
			}
			else{ //coloro le diagonali se possibile
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]-1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]-1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna-1,colore);
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]+1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]+1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna+1,colore);
			}
		}
	}
		
	if(!(is_wall(riga, colonna-1, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga, colonna-1) && controlla_casella_giocatore(posizione_player2, riga, colonna-1)) colora_quadrato(riga, colonna-1, colore);
		else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]-1) && posizione_player2[1]!=0) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]-1) && posizione_player1[1]!=0))){
			if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]-1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]-1,posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga, colonna-2, colore);
			}
			else{ //coloro le diagonali se possibile
				if((turno && !(is_wall(posizione_player2[0]-1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]-1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna-1,colore);
				if((turno && !(is_wall(posizione_player2[0]+1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]+1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna-1,colore);
			}
		}
	}
	
		if(!(is_wall(riga, colonna+1, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga, colonna+1) && controlla_casella_giocatore(posizione_player2, riga, colonna+1)) colora_quadrato(riga, colonna+1, colore);
		else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]+1) && posizione_player2[1]!=6) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]+1) && posizione_player1[1]!=6))){
			if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]+1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]+1,posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga, colonna+2, colore);
			}
			else{ //coloro le diagonali se possibile
				if((turno && !(is_wall(posizione_player2[0]-1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]-1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna+1,colore);
				if((turno && !(is_wall(posizione_player2[0]+1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]+1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna+1,colore);
			}
		}
	}
}
	
	if(colonna==6 && riga!=0 && riga!=6){ //ultima colonna ma non negli angoli
		
		if(!(is_wall(riga-1, colonna, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga-1, colonna) && controlla_casella_giocatore(posizione_player2, riga-1, colonna)) colora_quadrato(riga-1, colonna, colore);
			else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0]-1, posizione_player1[1]) && posizione_player2[0]!=0) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0]-1, posizione_player2[1]) && posizione_player1[0]!=0))){
			if((turno && !(is_wall(posizione_player2[0]-1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]-1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga-2, colonna, colore);
			}
			else{ //coloro le diagonali se possibile
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]-1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]-1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna-1,colore);
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]+1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]+1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna+1,colore);
				}
			}
		}
		
		if(!(is_wall(riga+1, colonna, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga+1, colonna) && controlla_casella_giocatore(posizione_player2, riga+1, colonna)) colora_quadrato(riga+1, colonna, colore);
			else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0]+1, posizione_player1[1]) && posizione_player2[0]!=6) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0]+1, posizione_player2[1]) && posizione_player1[0]!=6))){
			if((turno && !(is_wall(posizione_player2[0]+1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]+1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga+2, colonna, colore);
			}
			else{ //coloro le diagonali se possibile
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]-1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]-1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna-1,colore);
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]+1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]+1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna+1,colore);
			}
		}
	}
		if(!(is_wall(riga, colonna-1, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga, colonna-1) && controlla_casella_giocatore(posizione_player2, riga, colonna-1)) colora_quadrato(riga, colonna-1, colore);
			else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]-1) && posizione_player2[1]!=0) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]-1) && posizione_player1[1]!=0))){
			if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]-1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]-1,posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga, colonna-2, colore);
			}
			else{ //coloro le diagonali se possibile
				if((turno && !(is_wall(posizione_player2[0]-1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]-1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna-1,colore);
				if((turno && !(is_wall(posizione_player2[0]+1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]+1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna-1,colore);
			}
		}
	}
}
	
	if(colonna==0 && riga!=0 && riga!=6){ //prima colonna ma non negli angoli
		
		if(!(is_wall(riga-1, colonna, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga-1, colonna) && controlla_casella_giocatore(posizione_player2, riga-1, colonna)) colora_quadrato(riga-1, colonna, colore);
			else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0]-1, posizione_player1[1]) && posizione_player2[0]!=0) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0]-1, posizione_player2[1]) && posizione_player1[0]!=0))){
			if((turno && !(is_wall(posizione_player2[0]-1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]-1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga-2, colonna, colore);
			}
			else{ //coloro le diagonali se possibile
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]-1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]-1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna-1,colore);
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]+1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]+1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna+1,colore);
				}
		}
	}
		
		if(!(is_wall(riga, colonna+1, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga, colonna+1) && controlla_casella_giocatore(posizione_player2, riga, colonna+1)) colora_quadrato(riga, colonna+1, colore);
			else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0], posizione_player1[1]+1) && posizione_player2[1]!=6) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0], posizione_player2[1]+1) && posizione_player1[1]!=6))){
			if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]+1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]+1,posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga, colonna+2, colore);
			}
			else{ //coloro le diagonali se possibile
				if((turno && !(is_wall(posizione_player2[0]-1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]-1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga-1,colonna+1,colore);
				if((turno && !(is_wall(posizione_player2[0]+1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]+1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna+1,colore);
			}
		}
	}
		
		if(!(is_wall(riga+1, colonna, riga, colonna))){
		if( controlla_casella_giocatore(posizione_player1, riga+1, colonna) && controlla_casella_giocatore(posizione_player2, riga+1, colonna)) colora_quadrato(riga+1, colonna, colore);
			else if(((turno && !controlla_casella_giocatore(posizione_player2, posizione_player1[0]+1, posizione_player1[1]) && posizione_player2[0]!=6) || (!turno && !controlla_casella_giocatore(posizione_player1, posizione_player2[0]+1, posizione_player2[1]) && posizione_player1[0]!=6))){
			if((turno && !(is_wall(posizione_player2[0]+1,posizione_player2[1],posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0]+1,posizione_player1[1],posizione_player1[0],posizione_player1[1])))){
				colora_quadrato(riga+2, colonna, colore);
			}
			else{ //coloro le diagonali se possibile
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]-1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]-1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna-1,colore);
				if((turno && !(is_wall(posizione_player2[0],posizione_player2[1]+1,posizione_player2[0],posizione_player2[1]))) || (!turno && !(is_wall(posizione_player1[0],posizione_player1[1]+1,posizione_player1[0],posizione_player1[1])))) colora_quadrato(riga+1,colonna+1,colore);
			}
		}
	}
}
	
}




