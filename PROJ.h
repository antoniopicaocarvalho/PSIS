 #include <sys/types.h>
 #include <unistd.h>


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
	int board[1000][1000];

} board_info;

typedef struct sock_adds{ 
	int player_sock;
	int dim_board[2];

}sock_adds;


player_id* new_player(pid_t npid, int client_sock, player_id* head, int n_player, int dim[]);