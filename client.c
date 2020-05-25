#include "UI_library.h"
#include "sock_init.h"
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "Proj1.h"

Uint32 Event_ShowCharacter;


int main(int argc, char * argv[]){

    if (argc <2){//-----------------------------------------------------------------------falta o adress do server
      printf("second argument should be server address\n");
      exit(-1);}

    SDL_Event event;

	struct sockaddr_in server_addr = make_socket(&sock_fd);
	inet_aton(argv[1], &server_addr.sin_addr);
	sock_fd = connect_server (sock_fd, server_addr);

	int rgb[3];
	int msg[2];
	int err_rcv;
	int dim[2];

	int bricks;

	pid_t npid = getpid();
	printf("o meu pid é %d \n", npid);
	



	int n_bricks = 0;
	char ** board;

	int i, j;
	//Recebe linhas e colunas
	if(err_rcv = recv(sock_fd, &msg, sizeof(msg), 0)>0){
		
		dim[0] = msg[0];
		dim[1] = msg[1];
		create_board_window(msg[0],msg[1]);
		printf("Board criada!!!\n"); 
	}

	
	board = malloc(sizeof(char *) * (msg[0]));           
	for ( i = 0 ; i < msg[0]; i++){
		board[i] = malloc (sizeof(char) * (msg[1]));
		for (j = 0; j < msg[1]; j++){
			board[i][j] = ' ';
		}
		board[i][j] = '\0';
	}

	//Recebe numero de Bricks
	if(err_rcv = recv(sock_fd, &bricks, sizeof(int), 0)>0){
		printf("Há %d Bricks!!!\n",bricks); 
	}


	//Pinta os Bricks
	for(int i = 0; i < bricks ; i++){
		if(err_rcv = recv(sock_fd, &msg, sizeof(msg), 0)>0){
			paint_brick(msg[0],msg[1]);
			board[msg[0]][msg[1]] = 'B';
		}
	}

	printf("Board Concluida\n");
	send(sock_fd, &npid, sizeof(npid), 0);




	printf(" sock_id do lado do cliente - %d\n", sock_fd);

	pthread_t thread_id;
	pthread_create(&thread_id, NULL, sync_receiver, NULL);
	//send(sock_fd, &thread_id, sizeof(thread_id), 0);












	play jogada;
	play *event_data;
	SDL_Event new_event;
	
	event_data = malloc(sizeof(play));
	*event_data = jogada;

	SDL_zero(new_event);
	new_event.type = Event_ShowCharacter;
	new_event.user.data1 = event_data;

	SDL_PushEvent(&new_event);

	int done = 0;
	int npos[4];


	while (!done){
		while (SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) 
				done = SDL_TRUE;
			

			//when the mouse mooves the pacman also moves
	/*			if(event.type == SDL_MOUSEMOTION){
				int x_new = init_pacman[0];
				int y_new = init_pacman[1];
				//int new_pos[2];

				//this function return the place where the mouse cursor is
				get_board_place(event.motion .x, event.motion .y,
												&x_new, &y_new);

				//Rato tem de ser adjacente a posicao anterior
				if( ((x_new - x_pac == 1) && (y_new - y_pac == 0)) || 
					((x_new - x_pac == -1) && (y_new - y_pac == 0)) ||
					((x_new - x_pac == 0) && (y_new - y_pac == 1)) || 
					((x_new - x_pac == 0) && (y_new - y_pac == -1)) ){

					play jogada_p = check_new_pos(x_new, y_new, x_pac, y_pac, new_board);

					jogada_p.character = 'P';
					//the old place is cleared
					clear_place(x_pac, y_pac);
					x_pac = jogada_p.x;
					y_pac = jogada_p.y;
					
					paint_pacman(x_pac, y_pac, rgb[0], rgb[1], rgb[2]);
					
					printf("Pacman move to x-%d y-%d\n", x_pac, y_pac);
					send(sock_fd, &jogada_p, sizeof(jogada_p), 0);
				}
			}
			//Setas
			if(event.type == SDL_KEYDOWN){
					int x_new_m, y_new_m;

					if (event.key.keysym.sym == SDLK_LEFT ){

						x_new_m = init_monster[0]-1;
						y_new_m = init_monster[1];

						play jogada_m = check_new_pos(x_new_m, y_new_m, init_monster[0],
																 init_monster[1], new_board);
						jogada_m.character = 'M';
						//the old place is cleared
						clear_place(init_monster[0], init_monster[1]);

						paint_monster(jogada_m.x, jogada_m.y, rgb[0], rgb[1], rgb[2]);

						init_monster[0] = jogada_m.x;
						init_monster[1] = jogada_m.y;

						printf("Monster move to x-%d y-%d\n", jogada_m.x, jogada_m.y);
						send(sock_fd, &jogada_m, sizeof(jogada_m), 0);

					}
					if (event.key.keysym.sym == SDLK_RIGHT ){

						x_new_m = init_monster[0]+1;
						y_new_m = init_monster[1];

						play jogada_m = check_new_pos(x_new_m, y_new_m, init_monster[0],
																 init_monster[1], new_board);
						jogada_m.character = 'M';
						//the old place is cleared
						clear_place(init_monster[0], init_monster[1]);

						paint_monster(jogada_m.x, jogada_m.y, rgb[0], rgb[1], rgb[2]);

						init_monster[0] = jogada_m.x;
						init_monster[1] = jogada_m.y;

						printf("Monster move to x-%d y-%d\n", jogada_m.x, jogada_m.y);
						send(sock_fd, &jogada_m, sizeof(jogada_m), 0);

					}
					if (event.key.keysym.sym == SDLK_UP ){

						x_new_m = init_monster[0];
						y_new_m = init_monster[1]-1;

						play jogada_m = check_new_pos(x_new_m, y_new_m, init_monster[0],
																 init_monster[1], new_board);
						jogada_m.character = 'M';
						//the old place is cleared
						clear_place(init_monster[0], init_monster[1]);

						paint_monster(jogada_m.x, jogada_m.y, rgb[0], rgb[1], rgb[2]);
						
						init_monster[0] = jogada_m.x;
						init_monster[1] = jogada_m.y;

						printf("Monster move to x-%d y-%d\n", jogada_m.x, jogada_m.y);
						send(sock_fd, &jogada_m, sizeof(jogada_m), 0);

					}
					if (event.key.keysym.sym == SDLK_DOWN ){

						x_new_m = init_monster[0];
						y_new_m = init_monster[1]+1;

						play jogada_m = check_new_pos(x_new_m, y_new_m, init_monster[0],
																 init_monster[1], new_board);
						jogada_m.character = 'M';
						//the old place is cleared
						clear_place(init_monster[0], init_monster[1]);

						paint_monster(jogada_m.x, jogada_m.y, rgb[0], rgb[1], rgb[2]);
						
						init_monster[0] = jogada_m.x;
						init_monster[1] = jogada_m.y;

						printf("Monster move to x-%d y-%d\n", jogada_m.x, jogada_m.y);
						send(sock_fd, &jogada_m, sizeof(jogada_m), 0);
					}

			}*/

		}
	}
	printf("fim\n");
	close_board_windows();
	exit(0);
}





void * sync_receiver(){

	

	pos_board msg1;
	int err_rcv;

	while(err_rcv = recv(sock_fd, &msg1, sizeof(pos_board), 0)>0){
    	if (msg1.object == 'P'){
    		printf("recebeu pacman ou caraças\n");
			paint_pacman(msg1.x_next, msg1.y_next, msg1.r, msg1.g, msg1.b);
			if(msg1.x != -1) clear_place(msg1.x, msg1.y);
		}

		if (msg1.object == 'M'){
    		printf("recebeu monster ou pipi\n");
			paint_monster(msg1.x_next, msg1.y_next, msg1.r, msg1.g, msg1.b);
			if(msg1.x != -1) clear_place(msg1.x, msg1.y);
		}

	}



	return (NULL);
}


/*
play check_new_pos(int x_next, int y_next, int x, int y,  board_info new_board){

	play next_move;
	next_move.x = x;
	next_move.y = y;

	//proxima posicao NAO pertence a board
	if ( (x_next > new_board.lines-1) || (y_next > new_board.cols-1) || (x_next < 0) || (y_next < 0) ){
		//e pode recuar
		next_move.x = 2*x - x_next;
		next_move.y = 2*y - y_next;
		if (new_board.board[next_move.x][next_move.y] == ' '){
			return(next_move);
		}
		//esta preso
		else{	
			next_move.x = x;
			next_move.y = y;
			return(next_move);
		}
	}

	//proxima posicao e um Brick
	if (new_board.board[x_next][y_next] == 'B')
	{
		//e pode recuar
		next_move.x = 2*x - x_next;
		next_move.y = 2*y - y_next;

		if ( (next_move.x < new_board.lines) && (next_move.y < new_board.cols) && (next_move.x >= 0) && (next_move.y >= 0) ){
		
			if (new_board.board[next_move.x][next_move.y] == ' '){
				return(next_move);
			}
			//esta preso
			else{	
				next_move.x = x;
				next_move.y = y;
				return(next_move);
			}
		}

		else{	
				next_move.x = x;
				next_move.y = y;
				return(next_move);
			}

	}
	//Pode avancar
	if (new_board.board[x_next][y_next] == ' '){
		next_move.x = x_next;
		next_move.y = y_next;
		return(next_move);
	}


	//printf("AQUI");
}
*/