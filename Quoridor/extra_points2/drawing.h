/**--------------File Info-------------------------------------------------------------------------------
** File name:			drawing.h
** Descriptions:	contiene tutte le funzioni che disegnano sul display
**------------------------------------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "LPC17xx.h"

/* Private function prototypes -----------------------------------------------*/
void colora_quadrato(uint16_t riga, uint16_t colonna, uint16_t colore);
void create_griglia(void);
void create_wall(uint16_t colore);
void ruota_muro_90(uint16_t colore);
void disegna_pezzetto_orizzontale(uint16_t colore, int* pos);
void disegna_pezzetto_verticale(uint16_t colore, int* pos);
void muovi_giocatore(uint16_t riga, uint16_t colonna, uint16_t turno);
void colora_caselle_vicine( int* pos, uint16_t colore, int turno);
void menu_game_mode(void);
void menu_opposite_player(void);
