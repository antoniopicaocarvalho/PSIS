#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>


//ESTRUTURAS
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

typedef struct board_info{
	int cols;
	int lines;
	char ** board;
	int bricks;
} board_info;

typedef struct sock_adds{ 
	int player_sock;
	int dim_board[2];
}sock_adds;





/*----------------------------SERVIDOR----------------------------*/

board_info board_read();

void send_board(int client_sock, board_info new_board);

player_id * init_player (player_id * new_player, board_info new_board, pid_t npid, int n_player, int client_sock, pthread_t player_thread);

board_info board_update (char item, board_info board, int pos[2]);

player_id * list_player(player_id * new_player, player_id* head);

player_id* find_player (player_id * head, pid_t npid);

void send_spawn(player_id * player, player_id * head);


/*-----------------------------CLIENT-----------------------------*/

void create_board(board_info new_board);

struct board_info un_serialize(int msg[]);

void * sync_receiver();