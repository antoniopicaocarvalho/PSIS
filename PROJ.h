 #include <sys/types.h>
 #include <unistd.h>

#define MAX_SIZE 100000



typedef struct ex4_message{
	int character;
	int x;
	int y;
} ex4_message;


typedef struct player_id{
	int player_pid;
	int player_n;
	int pos_pacman[2];
	int pos_monster[2];
	int rgb[3];
	int sock_id;
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


player_id* new_player(pid_t npid, int client_sock, player_id* head, int n_player, int dim[], board_info new_board);

char** board_init(board_info new_board);

void send_board(int client_sock, board_info new_board);

board_info board_read();

board_info board_update (char item, board_info board, int pos[2]);

player_id* find_player (player_id* head, pid_t npid);


player_id * list_player(player_id * new_player, player_id* head);

player_id * init_player (player_id * new_player, board_info new_board, pid_t npid, int n_player, int client_sock);