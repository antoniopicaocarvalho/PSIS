#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "UI_library.h"
#include "sock_init.h"
#include <pthread.h>
#include <stdlib.h> 
#include <SDL2/SDL.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


#define MAX_SIZE 300

//STRUCTS
typedef struct pos_board{
	char object;
	int sock_id;
	int r;
	int g;
	int b;

	int x_next;
	int y_next;
	int x;
	int y; 
	int time;

	int k;

}pos_board;

typedef struct colour{
	int r;
	int g;
	int b;
}colour;

typedef struct play{
	int x;
	int y;
	char character;
}play;

typedef struct player_id{
	int player_pid;
	int player_n;
	int pos_pacman[2];
	int pos_monster[2];
	int rgb[3];
	int sock_id;
	pthread_t thread_id;
	struct player_id * next;
} player_id;


typedef struct sock_adds{ 
	int player_sock;
	int dim_board[2];
}sock_adds;




//----------------------------SERVER----------------------------

pos_board ** board_read();

void send_board(int client_sock, pos_board ** new_board);

player_id * init_player (player_id * new_player, pos_board ** new_board, colour c_colour, int client_sock, pthread_t player_thread);

void board_update (pos_board ** new_board, player_id  * new_player);

void * clock_time(void * input);

void * comms_Thread(void * input);

void * thirty_reset(void * input);

void spawn_fruits (int n_player, pos_board ** new_board);


//-----------------------------CLIENT-----------------------------


void * sync_receiver();

pos_board check_new_pos(int x_next, int y_next, int x, int y, char ** board, int dim[2]);

