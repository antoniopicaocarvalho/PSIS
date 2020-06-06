#include "Proj1.h"

int sock_fd;
int server_socket;

int dim[2];// 0 -> colunas | x  1 -> linhas | y

int n_bricks;

int comms[50]={0};  //array de socket_ids

int n_player ,total_p;

int countp [MAX_SIZE] = {0};
int countm [MAX_SIZE] = {0};

int done [MAX_SIZE] = {0};

int fruit [MAX_SIZE] = {0};
int edies [MAX_SIZE] = {0};

int player_m[MAX_SIZE] = {0};

int begin_time, actual_time;

pthread_mutex_t board_mutex;

int main(int argc, char* argv[]){

	
	//READ BOARD.TXT
	pos_board ** new_board;
	new_board = board_read();

	//-----PRINT BOARD-------
	int i, j; 
	for ( i = 0 ; i < dim[1]; i++){
	    for (j = 0; j < dim[0]; j++) printf("%c", new_board[i][j].object);
	    printf("\n");
	}

	//CREATE MUTEX 
	if (pthread_mutex_init(&board_mutex, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }


	//CREATE SOCKET
	struct sockaddr_in local_addr = make_socket(&sock_fd);
	local_addr.sin_addr.s_addr= INADDR_ANY;
	sock_fd = Do_Bind(sock_fd, local_addr);
	Do_Listen(sock_fd);

	printf("Waiting for players...\n");

	struct sockaddr_in client;
	socklen_t size = sizeof (struct sockaddr_in);

	int client_sock;

	printf("board %d * %d \n", dim[0], dim[1]);
	
	int err_rcv;

	pthread_t player_thread;
	colour c_colour;

	//TIME THREAD
	pthread_t clock_thread;
	pthread_create(&clock_thread, NULL, clock_time, NULL);

	while(1){ 
		
		client_sock = accept(sock_fd, (struct sockaddr*)&client, &size);

		if( client_sock == -1 ){
			perror("accept");
			exit(-1);
		}

		n_player++;

		//RECV PLAYER'S COLOUR 
		if ( (err_rcv = recv(client_sock, &c_colour, sizeof(colour), 0)) < 0 ) {
			printf("ERRO a receber cor");
			exit(-1);
		}
		
		//SEND BOARD
		send_board(client_sock, new_board);
		//SEND PLAYER ID
		send(client_sock, &client_sock, sizeof(int), 0);


		//CHECK 
		if (dim[0]*dim[1] - n_bricks < (n_player-1)*4+2){
			printf("NOT ENOUGH SPACE FOR NEW PLAYER\n");
			n_player--;
			pos_board quit;
			quit.object='Y';
			send(client_sock, &quit, sizeof(pos_board), 0);

			close(client_sock);
		}
		else{ 
			printf("\n-- Entrou um jogador (Total: %d) --\n", n_player);

			//NEW PLAYER
			player_id  * new_player = NULL;

			pthread_mutex_lock(&board_mutex);
			new_player = init_player(new_player, new_board, c_colour, client_sock, player_thread); 
			board_update (new_board, new_player);
			pthread_mutex_unlock(&board_mutex);

			//UPDATE ARRAY OF SOCKETS
			comms[n_player-1] = new_player-> sock_id;

			pthread_mutex_lock(&board_mutex);
			//CREATE FRUITS
			spawn_fruits(n_player, new_board);
			//SEND NEW PLAYER'S FIRST POS TO THE OTHER PLAYERS
			for (int i = 0; i < n_player-1; ++i)
			{
				send(comms[i], &new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]], sizeof (pos_board), 0);
				send(comms[i], &new_board[new_player -> pos_monster[0]][new_player->pos_monster[1]], sizeof (pos_board), 0);
			}
			pthread_mutex_unlock(&board_mutex);

			//PLAYER HANDLING THREAD
			pthread_create(&player_thread, NULL, comms_Thread, (void *)new_board);

			total_p++;

		}
	}
	return (0);	
}

void * clock_time(void * input){

	struct tm * local0;
	time_t t0 = time(NULL);
	local0 = localtime(&t0);
	begin_time = (local0 -> tm_min)*60 + local0->tm_sec;

	while(1){

		struct tm * local00;
		time_t t00 = time(NULL);
		local00 = localtime(&t00);
		actual_time = (local00 -> tm_min)*60 + local00->tm_sec;

	}
}

void * comms_Thread(void * input){	

	int err_rcv;

	pos_board play;
	pos_board monster;
	pos_board pacman;
	pos_board aux2;
	pos_board clean;
	clean.object = ' ';

	int sock = comms[n_player-1];

	int k = total_p;

	int px, py;

	pthread_mutex_lock(&board_mutex);
	//SEND PACMAN's, MONSTER's and FRUIT's POS TO THE NEW PLAYER
	for (int i = 0; i < dim[1]; ++i){
		for (int j = 0; j < dim[0]; ++j){
			if(((pos_board**)input)[i][j].object=='M' || ((pos_board**)input)[i][j].object=='P' ||
			   ((pos_board**)input)[i][j].object=='L' || ((pos_board**)input)[i][j].object=='C'){

				send(sock, &((pos_board**)input)[i][j], sizeof (pos_board), 0);

				//IDENTIFY PLAYER's CHARACTERS
				if (((pos_board**)input)[i][j].sock_id == sock){
					if (((pos_board**)input)[i][j].object=='M'){
						((pos_board**)input)[i][j].time = actual_time;
						monster = ((pos_board**)input)[i][j];
					}
					if (((pos_board**)input)[i][j].object=='P'){
						((pos_board**)input)[i][j].time = actual_time;
					 	pacman = ((pos_board**)input)[i][j];
					}
				}	
			}
		}
	}
	pthread_mutex_unlock(&board_mutex);

	//TIME MANAGEMENT THREAD
	pthread_t time_thread;
	pthread_create(&time_thread, NULL, thirty_reset, (void *)((pos_board**)input));

	//flag movement
	int m = 0;

	//RECV POSSIBLE PLAY
	while((err_rcv = recv(sock, &play , sizeof(pos_board), 0)) >0 ){

    	if (play.object == 'q'){
    		//clean monster position
    		pthread_mutex_lock(&board_mutex);
    		((pos_board**)input)[play.y_next][play.x_next] = clean;
    		for (int i = 0; i < n_player; ++i) if (comms[i]!=sock) send(comms[i], &play, sizeof (pos_board), 0);  
    		pthread_mutex_unlock(&board_mutex);
    	}
	   	else if (play.object == 'Q'){
	   		//clean pacman position and update variables
			pthread_mutex_lock(&board_mutex);

			((pos_board**)input)[play.y_next][play.x_next] = clean;
			
			//stop time management thread
			done[k] = 1;

			n_player --;

			//update comms array
			int w = 0;
			while (comms[w] != play.sock_id) w++;
			if (w != n_player) comms[w] = comms[n_player];

			close(play.sock_id);

			for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
			pthread_mutex_unlock(&board_mutex);

			//clean fruit when player exits
			if (n_player >= 1){

				pos_board cfruit;
				cfruit.object = 'Q';

				int d, auxf;

				while(!d){ 
	    			for (int i = 0; i < dim[1]; ++i){
						for (int j = 0; j < dim[0]; ++j){
							if (((pos_board**)input)[i][j].object=='L' || ((pos_board**)input)[i][j].object=='C'){
								if(auxf < 2){
									pthread_mutex_lock(&board_mutex);
									((pos_board**)input)[i][j] = clean;
									cfruit.x_next = j;
									cfruit.y_next = i;
									auxf++;
									for (int n = 0; n < n_player; ++n) send(comms[n], &cfruit, sizeof (pos_board), 0);  
									pthread_mutex_unlock(&board_mutex);
								}
								else if(auxf == 2) d = 1;
							}
						}
	    			}
				}
			}	
			printf("\n-- Saiu um jogador (Total: %d) --\n", n_player);
    	}
    	else{  
    		if ((countp[k] < 2 && play.object == 'P') || (countm[k] < 2 && play.object == 'M')||(countp[k] < 2 && play.object == 'S')){

    			//array that informs when board values are updated
    			player_m[k]++;

    			//update the time
    			play.time = actual_time;

    			//update pacman
    			if(play.object == 'P' || play.object == 'S'){
    				pacman = play;
    				countp[k]++;
    			}
				//update monster
    			if(play.object == 'M'){
    				monster = play;
    				countm[k]++;
    			}

    			//PLAY INSIDE BOARD
    			if(play.x_next < dim[0] && play.y_next < dim[1] && play.x_next >= 0 && play.y_next >= 0 ){

	    			//BRICK
					if (((pos_board**)input)[play.y_next][play.x_next] == 'B'){
						//BOUNCE POS
						int n_x = 2*play.x - play.x_next;
						int n_y = 2*play.y - play.y_next;
						//BOUNCE BACK - INSIDE BOARD
						if ( (n_x < dim[0]) && (n_y < dim[1]) && (n_x >= 0) && (n_y >= 0) ){
							//STUCK
							pthread_mutex_lock(&board_mutex);
							if (((pos_board**)input)[n_y][n_x].object == 'B'){
								
								play.x_next = play.x;
								play.y_next = play.y;

								//move done!
								m = 1;

								for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
								pthread_mutex_unlock(&board_mutex);	
							}
							//CAN BOUNCE BACK
							else{

								play.x_next = n_x;
								play.y_next = n_y;
								pthread_mutex_unlock(&board_mutex);
							}
						}//BOUNCE BACK - OUTSIDE BOARD
						else{	
							play.x_next = play.x;
							play.y_next = play.y;

							//move done!
							m = 1;

							pthread_mutex_lock(&board_mutex);
							for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
							pthread_mutex_unlock(&board_mutex);
						}
					}
		
				}
				//TRIES TO PLAY OUTSIDE BOARD
		    	else{
					//BOUNCE POS
					int n_x = 2*play.x - play.x_next;
					int n_y = 2*play.y - play.y_next;
					//BOUNCE BACK - INSIDE BOARD
					if ( (n_x < dim[0]) && (n_y < dim[1]) && (n_x >= 0) && (n_y >= 0) ){
						//STUCK
						pthread_mutex_lock(&board_mutex);
						if (((pos_board**)input)[n_y][n_x].object == 'B'){
							
							play.x_next = play.x;
							play.y_next = play.y;

							//move done!
							m = 1;

							for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
							pthread_mutex_unlock(&board_mutex);	
						}
						//CAN BOUNCE BACK
						else{

							play.x_next = n_x;
							play.y_next = n_y;
							pthread_mutex_unlock(&board_mutex);
						}
					}//BOUNCE BACK - OUTSIDE BOARD
					else{	
						play.x_next = play.x;
						play.y_next = play.y;

						//move done!
						m = 1;

						pthread_mutex_lock(&board_mutex);
						for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
						pthread_mutex_unlock(&board_mutex);
					}
		    	}

		    	//CONTINUE TO CHECK PLAY
		    	if(m==0){

		    		pos_board aux = ((pos_board**)input)[play.y_next][play.x_next];

			    	//TROCA DE POSICOES
	    			if((aux.object == 'P' && play.object == 'P') || (aux.object == 'S' && play.object == 'P') ||
	    			   (aux.object == 'P' && play.object == 'S') || (aux.object == 'S' && play.object == 'S') ||
	    			   (aux.object == 'M' && play.object == 'M') || (aux.r == play.r && aux.g == play.g && aux.b == play.b)){
	    				
	    				printf("troca de pos %c, %c\n", play.object, aux.object);
	    				
		    			aux.x_next = play.x;
						aux.y_next = play.y;
						aux.x = play.x_next;
						aux.y = play.y_next;

						play.x = play.x_next;
						play.y = play.y_next;


						pthread_mutex_lock(&board_mutex);
		    			((pos_board**)input)[play.y][play.x] = aux;
		    			((pos_board**)input)[play.y_next][play.x_next] = play;

						for (int i = 0; i < n_player; ++i){
							send(comms[i], &aux, sizeof (pos_board), 0);
				    		send(comms[i], &play, sizeof (pos_board), 0);
				    		}
						pthread_mutex_unlock(&board_mutex);
			    	}
			    	//FRUIT - REST  
			    	else if((aux.object == 'L' && play.object == 'P') || (aux.object == 'L' && play.object == 'S') ||
	    			(aux.object == 'C' && play.object == 'P') || (aux.object == 'C' && play.object == 'S')||
	    			(aux.object == 'L' && play.object == 'M') || (aux.object == 'C' && play.object == 'M')){
			    		play.points++;
			    		
			    		if(play.object == 'P') play.object = 'S';
				    	else if(play.object == 'S') edies[k]=0;
				    	if (aux.object == 'L') fruit[k] = 1;
		    			if (aux.object == 'C') fruit[k] = 2;

				    	pthread_mutex_lock(&board_mutex);
		    			((pos_board**)input)[play.y_next][play.x_next] = play;
		    			((pos_board**)input)[play.y][play.x] = clean;
				    	for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
						pthread_mutex_unlock(&board_mutex);

						printf(" - Fruta comida -\n");
			    	}
			    	//MONSTRO - PACMAN
			    	else if((aux.object == 'M' && play.object == 'P') || (aux.object == 'P' && play.object == 'M')){

			    		while (1){ 
							py=rand()%(dim[1]-1);
							px=rand()%(dim[0]-1);
							if(((pos_board**)input)[py][px].object == ' '){
								pthread_mutex_lock(&board_mutex);
								break;								
							} 
						}

						// PACMAN EATS MONSTER
						if (play.object == 'P'){
							aux.points++;

							play.x_next = px;
							play.y_next = py;

							((pos_board**)input)[aux.y_next][aux.x_next] = aux;
			    			((pos_board**)input)[play.y_next][play.x_next] = play;
			    			((pos_board**)input)[play.y][play.x] = clean;
			    			
							for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
							pthread_mutex_unlock(&board_mutex);
						}

						// MONSTER EATS PACMAN
						else{
							play.points++;

							aux.x = aux.x_next;
							aux.y = aux.y_next;
							aux.x_next = px;
							aux.y_next = py;

			    			((pos_board**)input)[aux.y_next][aux.x_next] = aux;
			    			((pos_board**)input)[play.y][play.x] = clean;
			    			((pos_board**)input)[play.y_next][play.x_next] = play;

							for (int i = 0; i < n_player; ++i) {
								send(comms[i], &aux, sizeof (pos_board), 0);
								send(comms[i], &play, sizeof (pos_board), 0);
							}
							pthread_mutex_unlock(&board_mutex);
						}
						printf(" - Pacman comido -\n");
			    	}
			    	//MONSTER - SUPERPACMAN
			    	else if((aux.object == 'M' && play.object == 'S') || (aux.object == 'S' && play.object == 'M')){
			    		printf("FRUTA2\n");

			    		edies[k]++;
			    		printf("numero de monstros comidos - %d \n", edies[k]);

			    		if (edies[k]==2){
							if(play.object == 'S') play.object='P';
							else if(aux.object == 'S') aux.object='P';
							edies[k]=0;
							printf(" - Super->Pacman -\n");
						}

			    		while (1){ 
							py=rand()%(dim[1]-1);
							px=rand()%(dim[0]-1);
							if(((pos_board**)input)[py][px].object == ' '){
								pthread_mutex_lock(&board_mutex);
								break;								
							} 
						}

						// MONSTER CLASHES INTO SUPERPACMAN
						if (play.object == 'M'){
							aux.points++;

							play.x_next = px;
							play.y_next = py;

							((pos_board**)input)[aux.y_next][aux.x_next] = aux;
			    			((pos_board**)input)[play.y_next][play.x_next] = play;
			    			((pos_board**)input)[play.y][play.x] = clean;

							for (int i = 0; i < n_player; ++i) {
								send(comms[i], &aux, sizeof (pos_board), 0);
								send(comms[i], &play, sizeof (pos_board), 0);
							}
							pthread_mutex_unlock(&board_mutex);
						}

						// SUPERPACMAN EATS MONSTER
						else{
							play.points++;

							aux.x = aux.x_next;
							aux.y = aux.y_next;
							aux.x_next = px;
							aux.y_next = py;

			    			((pos_board**)input)[aux.y_next][aux.x_next] = aux;
			    			((pos_board**)input)[play.y][play.x] = clean;
			    			((pos_board**)input)[play.y_next][play.x_next] = play;

							for (int i = 0; i < n_player; ++i) {
								send(comms[i], &aux, sizeof (pos_board), 0);
								send(comms[i], &play, sizeof (pos_board), 0);
							}
							pthread_mutex_unlock(&board_mutex);
						}
						printf(" - Monstro comido -\n");
					}
					//CAN MOVE
					else{ 	 
						pthread_mutex_lock(&board_mutex);
						((pos_board**)input)[play.y_next][play.x_next] = play;
	    				((pos_board**)input)[play.y][play.x] = clean;

	    				for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
	    				pthread_mutex_unlock(&board_mutex);
					}
				}
				printf("move to: %d-%d\n", play.x_next, play.y_next);
				m = 0;
    		}
    		//DOESN'T MOVE
    		else{
    			printf("FRUTA4\n");
    			play.x_next = play.x;
				play.y_next = play.y;
				for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);

    		}

	    	//PRINT BOARD
			printf("--------------------------------\n");
			for (int i = 0; i < dim[1]; ++i)
			{
				for (int j = 0; j < dim[0]; ++j)
				{
					printf("%c", ((pos_board**)input)[i][j].object);
				}
				printf("\n");
			}
			printf("--------------------------------\n\n");

    	}
	}
	return (NULL);
}
