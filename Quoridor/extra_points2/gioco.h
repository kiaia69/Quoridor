/**--------------File Info-------------------------------------------------------------------------------
** File name:			gioco.h
** Descriptions:	contiene tutte le funzioni utili per la logica del gioco
**------------------------------------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "LPC17xx.h"

/* Private function prototypes -----------------------------------------------*/

int is_wall(int riga_casella, int colonna_casella, int riga_casella_partenza, int colonna_casella_partenza);
void is_finished(int* pos, int turno);
int controlla_casella_giocatore(int* pos, uint16_t riga, uint16_t colonna);
int scavalca(uint16_t riga_iniziale, uint16_t colonna_inziale, char movimento); 
int controlla_presenza_orizzontale(void);
int controlla_presenza_verticale(void);
int controlla_blocco_giocatori(uint32_t riga_muro, uint32_t colonna_muro, char orientamento_muro);
void registra_mossa(uint8_t player_id, uint8_t move_wall, uint8_t vertical_horizontal, uint8_t y_coordinate, uint8_t x_coordinate);
void inizia_partita(uint8_t game_mode, uint8_t opposite_player);
void mossa_npc(int* posizione_avversario, int p2_wall, int* posizione_npc);
void effettua_mossa(uint32_t mossa);


typedef struct {
    int riga;
    int colonna;
		uint16_t colore;
    char orientamento;  // 'V' per verticale, 'H' per orizzontale
} PosizioneMuro;

typedef struct {
    int riga;
    int colonna;
		uint16_t muro; //=0 sta muovendo la pedina =1 sta mettendo un muro
    char orientamento;  // 'V' per verticale, 'H' per orizzontale
} MossaNPC;
