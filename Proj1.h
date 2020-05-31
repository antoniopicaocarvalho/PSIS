#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>


//ESTRUTURAS


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
}pos_board;




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




//----------------------------SERVIDOR----------------------------

pos_board ** board_read();


void send_board(int client_sock, pos_board ** new_board);

player_id * init_player (player_id * new_player, pos_board ** new_board, pid_t npid, int client_sock, pthread_t player_thread);

void board_update (pos_board ** new_board, player_id  * new_player);

void * comms_Thread(void * input);

void * thirty_reset(void * input);
/*
player_id * list_player(player_id * new_player, player_id* head);

player_id* find_player (player_id * head, pid_t npid);

void send_spawn(player_id * player, player_id * head);
*/

//-----------------------------CLIENT-----------------------------




void * sync_receiver();
pos_board check_new_pos(int x_next, int y_next, int x, int y, char ** board, int dim[2]);

