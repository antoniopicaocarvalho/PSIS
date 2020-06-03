#include "UI_library.h"
#include "sock_init.h"
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "Proj1.h"

Uint32 Event_ShowCharacter;

pos_board pac;
pos_board mon;

int sock_fd, sock_id, flag1, flag2;


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
	int dim[2];
	int bricks;

	pid_t npid = getpid();
	printf("o meu pid é %d \n", npid);
	
	char ** board;

	int i, j;
	int err_rcv;
	//Recebe linhas e colunas
	if(err_rcv = recv(sock_fd, &msg, sizeof(msg), 0)>0){
		
		dim[0] = msg[0];
		dim[1] = msg[1];
		create_board_window(msg[0],msg[1]);
		printf("Board criada!!!\n"); 
	}
	
	//Criacao da matriz window
	board = malloc(sizeof(char *) * (msg[0]));           
	for ( i = 0 ; i < msg[0]; i++){
		board[i] = malloc (sizeof(char) * (msg[1]));
		for (j = 0; j < msg[1]; j++){
			board[i][j] = ' ';
		}
		board[i][j] = '\0';
	}

	//Recebe numero de Bricks
	if(err_rcv = recv(sock_fd, &bricks, sizeof(int), 0)>0) printf("Há %d Bricks!!!\n",bricks); 
	

	//Pinta os Bricks
	for(int i = 0; i < bricks ; i++){
		if(err_rcv = recv(sock_fd, &msg, sizeof(msg), 0)>0){
			paint_brick(msg[0],msg[1]);
			board[msg[0]][msg[1]] = 'B';
		}
	}

	printf("Board Concluida\n\n");

	if(err_rcv = recv(sock_fd, &sock_id, sizeof(int), 0)>0) {};

	send(sock_fd, &npid, sizeof(npid), 0);

	pthread_t thread_id;
	pthread_create(&thread_id, NULL, sync_receiver, NULL);
	//send(sock_fd, &thread_id, sizeof(thread_id), 0);

	pos_board jogada;

	flag1 = 0;
    flag2 = 0;

	//waiting for window to be created
	while(flag1+flag2<2){ }


/*	play *event_data;
	SDL_Event new_event;
	
	event_data = malloc(sizeof(play));
	*event_data = jogada;

	SDL_zero(new_event);
	new_event.type = Event_ShowCharacter;
	new_event.user.data1 = event_data;

	SDL_PushEvent(&new_event);
*/

	int done = 0;
	int npos[4];

	pac.x = pac.x_next;
	pac.y = pac.y_next;
	mon.x = mon.x_next;
	mon.y = mon.y_next;

	//printf("pos %d - %d \n", pac.x, pac.y);

	int x_new, y_new, x_pac, y_pac, x_new_m, y_new_m, x_mon, y_mon;
	

	while (!done){
		while (SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) {
				mon.object = 'q';
				pac.object = 'Q';
				send(sock_fd, &mon, sizeof(pos_board), 0);
				send(sock_fd, &pac, sizeof(pos_board), 0);

				done = SDL_TRUE;
			}

			//when the mouse mooves the pacman also moves
			if(event.type == SDL_MOUSEMOTION){

				pos_board jogada_p;

				//prev pos
				x_pac = pac.x_next;
				y_pac = pac.y_next;

				//this function return the place where the mouse cursor is
				get_board_place(event.motion.x, event.motion.y, &x_new, &y_new);

				//Rato tem de ser adjacente a posicao anterior
				if( ((x_new - x_pac == 1) && (y_new - y_pac == 0)) || 
					((x_new - x_pac == -1) && (y_new - y_pac == 0)) ||
					((x_new - x_pac == 0) && (y_new - y_pac == 1)) || 
					((x_new - x_pac == 0) && (y_new - y_pac == -1)) ){

					//pos_board jogada_p = check_new_pos(x_new, y_new, x_pac, y_pac, board, dim);
					
					jogada_p.object = pac.object;
					jogada_p.sock_id = pac.sock_id;

					jogada_p.r = pac.r;
					jogada_p.g = pac.g;
					jogada_p.b = pac.b;

					jogada_p.x = x_pac;
					jogada_p.y = y_pac;
					jogada_p.x_next = x_new;
					jogada_p.y_next = y_new;

					send(sock_fd, &jogada_p, sizeof(pos_board), 0);
				}
			}
			//Setas
			if(event.type == SDL_KEYDOWN){

				pos_board jogada_m;

				if (event.key.keysym.sym == SDLK_LEFT ){

					x_new_m = mon.x_next-1;
					y_new_m = mon.y_next;
					x_mon = mon.x_next;
					y_mon = mon.y_next;

					//pos_board jogada_m = check_new_pos(x_new_m, y_new_m, x_mon, y_mon, board, dim);

					jogada_m.object = mon.object;
					jogada_m.sock_id = mon.sock_id;

					jogada_m.r = mon.r;
					jogada_m.g = mon.g;
					jogada_m.b = mon.b;

					jogada_p.x = x_mon;
					jogada_p.y = y_mon;
					jogada_p.x_next = x_new_m;
					jogada_p.y_next = y_new_m;

					send(sock_fd, &jogada_m, sizeof(jogada_m), 0);
				}
				if (event.key.keysym.sym == SDLK_RIGHT ){

					x_new_m = mon.x_next+1;
					y_new_m = mon.y_next;
					x_mon = mon.x_next;
					y_mon = mon.y_next;

					//pos_board jogada_m = check_new_pos(x_new_m, y_new_m, x_mon, y_mon, board, dim);

					jogada_m.object = mon.object;
					jogada_m.sock_id = mon.sock_id;

					jogada_m.r = mon.r;
					jogada_m.g = mon.g;
					jogada_m.b = mon.b;

					jogada_p.x = x_mon;
					jogada_p.y = y_mon;
					jogada_p.x_next = x_new_m;
					jogada_p.y_next = y_new_m;

					send(sock_fd, &jogada_m, sizeof(jogada_m), 0);
				}
				if (event.key.keysym.sym == SDLK_UP ){

					x_new_m = mon.x_next;
					y_new_m = mon.y_next-1;
					x_mon = mon.x_next;
					y_mon = mon.y_next;

					//pos_board jogada_m = check_new_pos(x_new_m, y_new_m, x_mon, y_mon, board, dim);

					jogada_m.object = mon.object;
					jogada_m.sock_id = mon.sock_id;

					jogada_m.r = mon.r;
					jogada_m.g = mon.g;
					jogada_m.b = mon.b;

					jogada_p.x = x_mon;
					jogada_p.y = y_mon;
					jogada_p.x_next = x_new_m;
					jogada_p.y_next = y_new_m;

					send(sock_fd, &jogada_m, sizeof(jogada_m), 0);
				}
				if (event.key.keysym.sym == SDLK_DOWN ){

					x_new_m = mon.x_next;
					y_new_m = mon.y_next+1;
					x_mon = mon.x_next;
					y_mon = mon.y_next;

					//pos_board jogada_m = check_new_pos(x_new_m, y_new_m, x_mon, y_mon, board, dim);

					jogada_m.object = mon.object;
					jogada_m.sock_id = mon.sock_id;

					jogada_m.r = mon.r;
					jogada_m.g = mon.g;
					jogada_m.b = mon.b;

					jogada_p.x = x_mon;
					jogada_p.y = y_mon;
					jogada_p.x_next = x_new_m;
					jogada_p.y_next = y_new_m;

					send(sock_fd, &jogada_m, sizeof(jogada_m), 0);
				}
			}

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
    	if (msg1.object == 'P' || msg1.object == 'S'){
			
    		if(msg1.x != -1) clear_place(msg1.x, msg1.y);
			
			if(msg1.object == 'P') paint_pacman(msg1.x_next, msg1.y_next, msg1.r, msg1.g, msg1.b);
			if(msg1.object == 'S') paint_powerpacman(msg1.x_next, msg1.y_next, msg1.r, msg1.g, msg1.b);

			//guardar o pos_board deste jogador - pacman
			if(msg1.sock_id == sock_id){  
				pac = msg1;
				flag1 = 1;
			}
		}

		if (msg1.object == 'M'){
    		if(msg1.x != -1) clear_place(msg1.x, msg1.y);
			paint_monster(msg1.x_next, msg1.y_next, msg1.r, msg1.g, msg1.b);

			//guardar o pos_board deste jogador - monster
			if(msg1.sock_id == sock_id) {
				mon = msg1;
				flag2 = 1;
			}
		}
		if (msg1.object == 'q' || msg1.object == 'Q')
		{
			clear_place(msg1.x_next, msg1.y_next);
		}

		if (msg1.object == 'L'){
			paint_lemon(msg1.x, msg1.y);
		}
		if (msg1.object == 'C'){
			paint_cherry(msg1.x, msg1.y);
		}

	}



	return (NULL);
}


/*
pos_board check_new_pos(int x_next, int y_next, int x, int y, char ** board, int dim[2]){

	pos_board next_move;

	//Esperado
	next_move.x_next = x_next;
	next_move.y_next = y_next;
	next_move.x = x;
	next_move.y = y;

	//proxima posicao NAO pertence a board ou e um Brick
	if((x_next > dim[0]-1) || (y_next > dim[1]-1) || (x_next < 0) || (y_next < 0) || board[x_next][y_next] == 'B') {
		//Bounce back
		next_move.x_next = 2*x - x_next;
		next_move.y_next = 2*y - y_next;
		//Caso o Bounce back seja para dentro da board
		if ( (next_move.x_next < dim[0]) && (next_move.y_next < dim[1]) && (next_move.x_next >= 0) && (next_move.y_next >= 0) ){
			//pode fazer o bounce back
			if (board[next_move.x_next][next_move.y_next] == ' '){
				return(next_move);
			}
			//esta preso
			else{	
				next_move.x_next = x;
				next_move.y_next = y;
				return(next_move);
			}
		}
		else{	
				next_move.x_next = x;
				next_move.y_next = y;
				return(next_move);
			}

	}
	//Pode avancar
	if (board[x_next][y_next] == ' '){
		next_move.x_next = x_next;
		next_move.y_next = y_next;
		return(next_move);
	}


	//printf("AQUI");
}
*/
