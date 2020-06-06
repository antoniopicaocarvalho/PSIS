#include "proj_final.h"

pos_board pac;
pos_board mon;

int sock_fd, sock_id, flag1, flag2;

int done = 0;

int main(int argc, char * argv[]){

    if (argc <5){
      printf("2º argument should be server address, 3º-5º should be RGB\n");
      exit(-1);}

    SDL_Event event;

	struct sockaddr_in server_addr = make_socket(&sock_fd);
	inet_aton(argv[1], &server_addr.sin_addr);
	sock_fd = connect_server (sock_fd, server_addr);

	int msg[2];

	int bricks;
	
	char ** board;

	int err_rcv;

	colour c_colour;

	c_colour.r = atoi(argv[2]);
	c_colour.g = atoi(argv[3]);
	c_colour.b = atoi(argv[4]);


	printf("COR - %d, %d, %d\n",c_colour.r, c_colour.g, c_colour.b);


	send(sock_fd, &c_colour, sizeof(colour), 0);

	//Recebe linhas e colunas
	if(err_rcv = recv(sock_fd, &msg, sizeof(msg), 0)>0){
		create_board_window(msg[0],msg[1]);
		printf("Board criada!!!\n"); 
	}

	//Recebe numero de Bricks
	if(err_rcv = recv(sock_fd, &bricks, sizeof(int), 0)>0) printf("Há %d Bricks!!!\n",bricks); 
	

	//Pinta os Bricks
	for(int i = 0; i < bricks ; i++){
		if(err_rcv = recv(sock_fd, &msg, sizeof(msg), 0)>0){
			paint_brick(msg[0],msg[1]);
		}
	}

	printf("Board Concluida\n\n");

	if(err_rcv = recv(sock_fd, &sock_id, sizeof(int), 0)>0) {
		printf("Recebe sock\n");

	}

	pthread_t thread_id;
	pthread_create(&thread_id, NULL, sync_receiver, NULL);
	//send(sock_fd, &thread_id, sizeof(thread_id), 0);

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

					jogada_m.x = x_mon;
					jogada_m.y = y_mon;
					jogada_m.x_next = x_new_m;
					jogada_m.y_next = y_new_m;

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

					jogada_m.x = x_mon;
					jogada_m.y = y_mon;
					jogada_m.x_next = x_new_m;
					jogada_m.y_next = y_new_m;

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

					jogada_m.x = x_mon;
					jogada_m.y = y_mon;
					jogada_m.x_next = x_new_m;
					jogada_m.y_next = y_new_m;
					
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

					jogada_m.x = x_mon;
					jogada_m.y = y_mon;
					jogada_m.x_next = x_new_m;
					jogada_m.y_next = y_new_m;

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

	int scoreboard[2][MAX_SIZE];
	int n_players;

	/*
	pthread_mutex_t board_mutex;
	if (pthread_mutex_init(&board_mutex, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        exit(-1);
    } */

	while(err_rcv = recv(sock_fd, &msg1, sizeof(pos_board), 0)>0){

		//pthread_mutex_lock(&board_mutex);

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
		else if (msg1.object == 'M'){
    		if(msg1.x != -1) clear_place(msg1.x, msg1.y);
			paint_monster(msg1.x_next, msg1.y_next, msg1.r, msg1.g, msg1.b);

			//guardar o pos_board deste jogador - monster
			if(msg1.sock_id == sock_id) {
				mon = msg1;
				flag2 = 1;
			}
		}
		else if (msg1.object == 'q' || msg1.object == 'Q')
		{
			clear_place(msg1.x_next, msg1.y_next);
		}

		else if (msg1.object == 'L'){
			paint_lemon(msg1.x, msg1.y);
		}
		else if (msg1.object == 'C'){
			paint_cherry(msg1.x, msg1.y);
		}
		else if(msg1.object == 'x') printf("\n***** SCORE BOARD *****\n");
		else if(msg1.object == 'X') printf("- Player %d : %d pts\n", msg1.sock_id-3, msg1.points);
		else if(msg1.object == 'Y'){
			printf("NOT ABLE TO JOIN\n");
			done = SDL_TRUE;
			flag1=1; 
			flag2=1; 
		} 
		
		//pthread_mutex_unlock(&board_mutex);

	}



	return (NULL);
}


