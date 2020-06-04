#include "Proj1.h"

int sock_fd;
int server_socket;


int dim[2];// 0 -colunas  1-linhas
//x - colunas y-linhas
int n_bricks;

int comms[50]={0};  //array de socket_ids

int n_player = 0;
int total_p = -1;

int countp [MAX_SIZE]={0};

int countm [MAX_SIZE]={0};

int done [MAX_SIZE] = {0};

int fruit [MAX_SIZE] = {0};

int edies [MAX_SIZE] = {0};








int main(int argc, char* argv[]){

	
	//Leitura do ficheiro BOARD.TXT
	pos_board ** new_board;
	new_board = board_read();

	//teste board_read()
	int i, j; 
	  for ( i = 0 ; i < dim[1]; i++){
	    for (j = 0; j < dim[0]; j++){
	      printf("%c", new_board[i][j].object);
	    }
	    printf("\n");
	  }


	//Criação da Socket
	struct sockaddr_in local_addr = make_socket(&sock_fd);
	local_addr.sin_addr.s_addr= INADDR_ANY;
	sock_fd = Do_Bind(sock_fd, local_addr);
	Do_Listen(sock_fd);

	printf("Waiting for players...\n");

	struct sockaddr_in client;
	socklen_t size = sizeof (struct sockaddr_in);


	int client_sock;

	printf("board %d * %d \n", dim[0], dim[1]);
	
	pid_t npid;

	int err_rcv;

	pthread_t player_thread;
	colour c_colour;


	//player_id * head = NULL;

	while(1){ 
		
		client_sock = accept(sock_fd, (struct sockaddr*)&client, &size);

		if( client_sock == -1 ){
			perror("accept");
			exit(-1);
		}

		n_player++;
		total_p++;

		printf("\n-- Entrou um jogador (Total: %d) --\n", n_player);

		//RECEBE COR
		if ( (err_rcv = recv(client_sock, &c_colour, sizeof(colour), 0)) < 0 ) {
			printf("ERRO a receber cor");
			exit(-1);
		}
		printf("COR - %d, %d, %d\n",c_colour.r, c_colour.g, c_colour.b);

		//ENVIAR A BOARD INICIAL lida do ficheiro
		send_board(client_sock, new_board);

		send(client_sock, &client_sock, sizeof(int), 0);


		//RECEBE O PID
		if ( (err_rcv = recv(client_sock, &npid, sizeof(npid), 0)) > 0 ) {
			//printf("recebeu o pid:  %d , do jogador %d \n", npid, n_player);
		}

		//VERIFICAR SE HA ESPACO PARA MAIS 1 PLAYER
		if (dim[0]*dim[1] - n_bricks < n_player*2) 
			printf("CAN'T FIT ANOTHER ONE -- DJ KHALED  \n");
		else{ 
			//Novo jogador
			player_id  * new_player = NULL;
			new_player = init_player(new_player, new_board, c_colour, client_sock, player_thread); 
			
			board_update (new_board, new_player);
			
			//Ligar jogador a lista de Jogadores
			//head = list_player(new_player, head);

			//atualizar o array de sock_ids
			comms[n_player-1] = new_player-> sock_id;

			//instancia o nº de frutas correspondente ao n_player
			spawn_fruits(n_player, new_board);


			//enviar posiçao inicial a todos os jogadores
			for (int i = 0; i < n_player-1; ++i)
			{
				send(comms[i], &new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]], sizeof (pos_board), 0);
				send(comms[i], &new_board[new_player -> pos_monster[0]][new_player->pos_monster[1]], sizeof (pos_board), 0);
			}

			pthread_create(&player_thread, NULL, comms_Thread, (void *)new_board);
/*

			//Verificar se a lista esta a ficar feita
			player_id * aux = head;
			while(aux){
				printf("O jogador %d esta na lista\n", aux->player_n);
				printf("com sock id - %d \n", aux->sock_id);
				aux = aux -> next;
			}*/
		}
	}
	
	return (0);	
}
 	


void * comms_Thread(void * input){	

	int err_rcv;

	pos_board play;
	pos_board clean;
	pos_board monster;
	pos_board pacman;
	pos_board aux2;
	clean.object = ' ';

	int sock = comms[n_player-1];

	int k = total_p;

	int px, py;

	pthread_mutex_t board_mutex;

	int auxf;


	//registar o instante em que foi criado o jogador
	struct tm * local;
	time_t t = time(NULL);
	local = localtime(&t);
	//int time_aux, time1;
	int time_init = (local -> tm_min)*60 + local->tm_sec;	

	//manda a board para o jogador novo e guarda as estruturas pos_board das posiçoes do respetivo
	for (int i = 0; i < dim[1]; ++i){
		for (int j = 0; j < dim[0]; ++j)
		{
			if(((pos_board**)input)[i][j].object=='M' || ((pos_board**)input)[i][j].object=='P' || ((pos_board**)input)[i][j].object=='L' || ((pos_board**)input)[i][j].object=='C'){

				send(sock, &((pos_board**)input)[i][j], sizeof (pos_board), 0);

				if (((pos_board**)input)[i][j].sock_id == sock)
				{
					if (((pos_board**)input)[i][j].object=='M'){
						((pos_board**)input)[i][j].time = time_init;
						monster = ((pos_board**)input)[i][j];
					}
					if (((pos_board**)input)[i][j].object=='P'){
						((pos_board**)input)[i][j].time = time_init;
					 	pacman = ((pos_board**)input)[i][j];
					}
				}	
			}
		}
	}

	//printf("inicio - %d\n", time_init);

	//thread dos 30s
	pthread_t time_thread;
	pthread_create(&time_thread, NULL, thirty_reset, (void *)((pos_board**)input));


	//-------------------------RECEBE JOGADA---------------------------------------
	while((err_rcv = recv(sock, &play , sizeof(pos_board), 0)) >0 ){

		pthread_mutex_lock(&board_mutex);

		struct tm * local1;
		time_t t = time(NULL);
		local1 = localtime(&t);
		int time_aux = (local1 -> tm_min)*60 + local1->tm_sec;	

    	if (play.object == 'q'){

    		//pthread_mutex_lock(&board_mutex);
    		((pos_board**)input)[play.y_next][play.x_next] = clean;

    		for (int i = 0; i < n_player; ++i) if (comms[i]!=sock) send(comms[i], &play, sizeof (pos_board), 0);  
    		pthread_mutex_unlock(&board_mutex);
    	}
	   	else if (play.object == 'Q'){
			pos_board cfruit;
			cfruit.object = 'Q';
			//pthread_mutex_lock(&board_mutex);
			((pos_board**)input)[play.y_next][play.x_next] = clean;
			//pthread_mutex_unlock(&board_mutex);
			done[k] = 1;
			n_player --;
			k=0;

			//update comms array
			while (comms[k] != play.sock_id) k++;
			if (k!=n_player) comms[k] = comms[n_player];

			for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);

			//clean fruit
			if (n_player >= 1){
				int d=1;
				while(d){ 
	    			for (int i = 0; i < dim[1]; ++i){
						for (int j = 0; j < dim[0]; ++j){
							if (((pos_board**)input)[i][j].object=='L' || ((pos_board**)input)[i][j].object=='C'){
								if(auxf <2){
									((pos_board**)input)[i][j] = clean;
									cfruit.x_next = j;
									cfruit.y_next = i;
									auxf++;
									for (int n = 0; n < n_player; ++n) send(comms[n], &cfruit, sizeof (pos_board), 0);  
								}
								else if(auxf == 2) d=0;
							}
						}
	    			}
				}
			}	

			pthread_mutex_unlock(&board_mutex);
			printf("\n-- Saiu um jogador (Total: %d) --\n", n_player);
    	}
    	else{  
    		if ((countp[k] < 2 && play.object == 'P') || (countm[k] < 2 && play.object == 'M')||(countp[k] < 2 && play.object == 'S')){

    			play.time = time_aux;

    			if(play.object == 'P' || play.object == 'S'){
    				pacman = play;
    				countp[k]++;
    			}

    			if(play.object == 'M'){
    				monster = play;
    				countm[k] ++;
    			}

    			if(play.x_next < dim[0] && play.y_next < dim[1] && play.x_next >= 0 && play.y_next >= 0 ){
	    			pos_board aux = ((pos_board**)input)[play.y_next][play.x_next];
	    			//TROCA DE POSICOES
	    			if((aux.object == 'P' && play.object == 'P') || (aux.object == 'S' && play.object == 'P') ||
	    			   (aux.object == 'P' && play.object == 'S') || (aux.object == 'S' && play.object == 'S') ||
	    			   (aux.object == 'M' && play.object == 'M') || (aux.r == play.r && aux.g == play.g && aux.b == play.b)){
	    				
	    				printf(" - troca de pos -\n");
	    				
	    				//pthread_mutex_lock(&board_mutex);
		    			
		    			aux.x_next = play.x;
						aux.y_next = play.y;
						aux.x = play.x_next;
						aux.y = play.y_next;
						
		    			((pos_board**)input)[play.y][play.x] = aux;
		    			
						for (int i = 0; i < n_player; ++i) send(comms[i], &aux, sizeof (pos_board), 0);

						play.x = play.x_next;
						play.y = play.y_next;

		    			((pos_board**)input)[play.y_next][play.x_next] = play;

				    	for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
						pthread_mutex_unlock(&board_mutex);	
			    	}
			    	//FRUTA - RESTO  
			    	else if((aux.object == 'L' && play.object == 'P') || (aux.object == 'L' && play.object == 'S') ||
	    			(aux.object == 'C' && play.object == 'P') || (aux.object == 'C' && play.object == 'S')||
	    			(aux.object == 'L' && play.object == 'M') || (aux.object == 'C' && play.object == 'M')){

			    		//pthread_mutex_lock(&board_mutex);
			    		if(play.object == 'P') play.object = 'S';
				    	else if(play.object == 'S') edies[k]=0;
				    	
				    	if (aux.object == 'L') fruit[k] = 1;
		    			if (aux.object == 'C') fruit[k] = 2;
				    	
		    			((pos_board**)input)[play.y_next][play.x_next] = play;
		    			((pos_board**)input)[play.y][play.x] = clean;

		    			
				    	for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
						pthread_mutex_unlock(&board_mutex);
						printf(" - Fruta comida -\n");
			    	}
			    	//MONSTRO - PACMAN
			    	else if((aux.object == 'M' && play.object == 'P') || (aux.object == 'P' && play.object == 'M')){

			    		//pthread_mutex_lock(&board_mutex);
			    		while (1){ 
							py=rand()%(dim[1]-1);
							px=rand()%(dim[0]-1);
							if(((pos_board**)input)[py][px].object == ' ') break;
						}

						// PACMAN é comido pelo Monster
						if (play.object == 'P')
						{
							play.x_next = px;
							play.y_next = py;

			    			((pos_board**)input)[play.y_next][play.x_next] = play;
			    			((pos_board**)input)[play.y][play.x] = clean;
			    			
							for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
							pthread_mutex_unlock(&board_mutex);
						}

						// Monster come PACMAN
						else{
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
			    	//MONSTRO - SUPERPACMAN
			    	else if((aux.object == 'M' && play.object == 'S') || (aux.object == 'S' && play.object == 'M')){

			    		//pthread_mutex_lock(&board_mutex);

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
							if(((pos_board**)input)[py][px].object == ' ') break;
						}

						// Monstro é comido pelo SUPERPACMAN
						if (play.object == 'M'){
							play.x_next = px;
							play.y_next = py;

			    			((pos_board**)input)[play.y_next][play.x_next] = play;
			    			((pos_board**)input)[play.y][play.x] = clean;

							for (int i = 0; i < n_player; ++i) {
								send(comms[i], &aux, sizeof (pos_board), 0);
								send(comms[i], &play, sizeof (pos_board), 0);
							}
							pthread_mutex_unlock(&board_mutex);
						}
						// SUPERPACMAN come Monstro
						else{

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
					//Brick
					else if (aux.object == 'B'){
						//Bounce back
						int n_x = 2*play.x - play.x_next;
						int n_y = 2*play.y - play.y_next;
						//Caso o Bounce back seja para dentro da board
						if ( (n_x < dim[0]) && (n_y < dim[1]) && (n_x >= 0) && (n_y >= 0) ){
							//pode fazer o bounce back
							
							if (((pos_board**)input)[n_y][n_x].object == ' '){
								
								play.x_next = n_x;
								play.y_next = n_y;

								((pos_board**)input)[n_y][n_x] = play;
								((pos_board**)input)[play.y][play.x] = clean;

								for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
							}
							//esta preso
							else{

								play.x_next = play.x;
								play.y_next = play.y;

								for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
							}
						}
						else{	
							play.x_next = play.x;
							play.y_next = play.y;

							for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
						}
					}
					//Pode avancar
					else{ 	
						((pos_board**)input)[play.y_next][play.x_next] = play;
	    				((pos_board**)input)[play.y][play.x] = clean;

	    				for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
					}

			    	//Jogada Realizada
					printf("move to: %d-%d\n", play.x_next, play.y_next);
					pthread_mutex_unlock(&board_mutex);
				}
				//QUANDO QUER SAIR DA BOARD
		    	else{
					//Bounce back
					int n_x = 2*play.x - play.x_next;
					int n_y = 2*play.y - play.y_next;
					//Caso o Bounce back seja para dentro da board
					if ( (n_x < dim[0]) && (n_y < dim[1]) && (n_x >= 0) && (n_y >= 0) ){
						//pode fazer o bounce back
						
						if (((pos_board**)input)[n_y][n_x].object == ' '){
							
							play.x_next = n_x;
							play.y_next = n_y;

							((pos_board**)input)[n_y][n_x] = play;
							((pos_board**)input)[play.y][play.x] = clean;

							for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
						}
						//esta preso
						else{

							play.x_next = play.x;
							play.y_next = play.y;

							for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
						}
					}
					else{	
						play.x_next = play.x;
						play.y_next = play.y;

						for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
					}

					

			    	//Jogada Realizada
					printf("move to: %d-%d\n", play.x_next, play.y_next);
					pthread_mutex_unlock(&board_mutex);
		    	}
    		}
    		//fica no mm sitio
    		else{
    			printf("NAO JOGA\n");
    			
    			/*play.x_next = play.x;
				play.y_next = play.y;

				for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
    			pthread_mutex_unlock(&board_mutex);*/
    		}
    	}

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
	return (NULL);
}


void * thirty_reset(void * input){	

	pthread_mutex_t board_mutex;
	
	pos_board pacman;
	pos_board monster;
	pos_board aux1;
	pos_board super;
	pos_board clean;
	clean.object = ' ';


	int px, py, start2, end2;
	int sock = comms[n_player-1]; 
	int k = total_p;
	int time2[500];
	int tfruit[500];

	pthread_mutex_lock(&board_mutex);
	for (int i = 0; i < dim[1]; ++i)
	{
		for (int j = 0; j < dim[0]; ++j)
		{
			if (((pos_board**)input)[i][j].sock_id == sock)
			{
				if (((pos_board**)input)[i][j].object=='M') monster = ((pos_board**)input)[i][j];
				if (((pos_board**)input)[i][j].object=='P') pacman = ((pos_board**)input)[i][j];
			}	
		}
	}
	pthread_mutex_unlock(&board_mutex);

	int time1 = pacman.time;

	while(!done[k]){ 

		pthread_mutex_lock(&board_mutex);


		for (int i = 0; i < dim[1]; ++i)
		{
			for (int j = 0; j < dim[0]; ++j)
			{
				if (((pos_board**)input)[i][j].sock_id == sock)
				{
					if (((pos_board**)input)[i][j].object=='M') monster = ((pos_board**)input)[i][j];
					if (((pos_board**)input)[i][j].object=='P') pacman = ((pos_board**)input)[i][j];
					if (((pos_board**)input)[i][j].object=='S') super = ((pos_board**)input)[i][j];
				}	
			}
		}

		
		pthread_mutex_unlock(&board_mutex);


		struct tm * local1;
		time_t t = time(NULL);
		local1 = localtime(&t);
		int time_aux = (local1 -> tm_min)*60 + local1->tm_sec;

		if (fruit[k]!=0){
			time2[end2] = time_aux+2;
			tfruit[end2] = fruit[k]; 
			fruit[k] = 0;
			end2++;
		} 

		for (int i = start2; i < end2; ++i)
		{
			if (time_aux == time2[i]){
				pthread_mutex_lock(&board_mutex);
				while (1){ 
					py=rand()%(dim[1]-1);
					px=rand()%(dim[0]-1);
					if(((pos_board**)input)[py][px].object == ' ') break;
				}
				//pos vazia
				aux1.x = px;
				aux1.y = py;
				if(tfruit[i]==1){
					aux1.object = 'L';
					((pos_board**)input)[py][px].object = 'L';
				} 
				if(tfruit[i]==2){
					aux1.object = 'C';
					((pos_board**)input)[py][px].object = 'C';
				} 
				for (int i = 0; i < n_player; ++i) send(comms[i], &aux1, sizeof (pos_board), 0);
				pthread_mutex_unlock(&board_mutex);
				start2++;
				printf(" - New fruit -\n");
			}
		}

		//passa 1s
		if (time_aux-time1 >= 1) 
		{
			printf("-----------------------------\n");
			time1 = time_aux;
			//printf("Tempo - %d\n",time_aux);
			countp[k] = 0;
			countm[k] = 0;
		
	
			//passam 30s - pacman
			if (time_aux-pacman.time >= 6) 
			{
				printf(" - Randoom pos P -\n");
				pthread_mutex_lock(&board_mutex);

				aux1 = pacman;
				while (1){ 
					py=rand()%(dim[1]-1);
					px=rand()%(dim[0]-1);
					if(((pos_board**)input)[py][px].object == ' ') break;
				}

				//pos vazia
				aux1.x_next = px;
				aux1.y_next = py;
				//pos antiga
				aux1.x = pacman.x_next;
				aux1.y = pacman.y_next;
				aux1.time = time_aux;

				
		    	((pos_board**)input)[pacman.y_next][pacman.x_next] = clean;
		    	((pos_board**)input)[aux1.y_next][aux1.x_next] = aux1;
		    	

		    	for (int i = 0; i < n_player; ++i) send(comms[i], &aux1, sizeof (pos_board), 0);
		    	pacman = aux1;
		  	    pthread_mutex_unlock(&board_mutex);



			}
			//passam 30s - monster
			if (time_aux-monster.time >= 30) 
			{
				printf(" - Randoom pos M -\n");
				pthread_mutex_lock(&board_mutex);

				aux1 = monster;
				while (1){ 
					py=rand()%(dim[1]-1);
					px=rand()%(dim[0]-1);
					if(((pos_board**)input)[py][px].object == ' ') break;
				}

				//pos vazia
				aux1.x_next = px;
				aux1.y_next = py;
				//printf("pos vazia: %d - %d  \n", px, py);
				//pos antiga
				aux1.x = monster.x_next;
				aux1.y = monster.y_next;
				aux1.time = time_aux;

		    	((pos_board**)input)[monster.y_next][monster.x_next] = clean;
		    	((pos_board**)input)[aux1.y_next][aux1.x_next] = aux1;
		    	pthread_mutex_unlock(&board_mutex);

		    	for (int i = 0; i < n_player; ++i) send(comms[i], &aux1, sizeof (pos_board), 0);
		    	monster = aux1;
		    }
		}
	}
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

void spawn_fruits (int n_player, pos_board ** new_board){
	
	int x,y;
	pos_board aux_1;
	pos_board aux_2;

	if (n_player > 1){
		while (1){ 
			y=rand()%(dim[1]-1);
			x=rand()%(dim[0]-1);
			if(new_board[y][x].object == ' ') break;
		}

		new_board[y][x].object = 'L';
		new_board[y][x].x = x;
		new_board[y][x].y = y;
		aux_1 = new_board[y][x];
		while (1){ 
			y=rand()%(dim[1]-1);
			x=rand()%(dim[0]-1);
			if(new_board[y][x].object == ' ') break;
		}
		new_board[y][x].object = 'C';
		new_board[y][x].x = x;
		new_board[y][x].y = y;
		aux_2 = new_board[y][x];

		for (int i = 0; i < n_player-1; ++i)
			{
				send(comms[i], &aux_1, sizeof (pos_board), 0);
				send(comms[i], &aux_2, sizeof (pos_board), 0);
			}
	}
}


pos_board ** board_read() {

	int col, line;

	FILE *fp = fopen("board.txt", "r");
	// test for files not existing. 
	if (fp == NULL) 
	{   
		printf("Error! Could not open file\n"); 
		exit(-1); 
	}

	//Reads first line of 'fp' file
	fscanf(fp, "%d %d", &col, &line);

	printf("Cols: %d\n", col);
	printf("Lines: %d\n", line);

    dim[0] = col;
    dim[1] = line;

    char letter;

    int i, j;

    pos_board ** board;
	board = malloc(sizeof(pos_board *) * (line));           
	for ( i = 0 ; i < line; i++){
		board[i] = malloc (sizeof(pos_board) * (col));
		for (j = 0; j < col; j++){
			board[i][j].object = ' ';
		}
	}

	j = 0;
	i = -1;

	while( (letter = getc(fp)) != EOF){    
		//Block
		if(letter == 'B') {
			board[i][j].object = 'B';
			n_bricks ++;
		}
		//New Column
		j++;
		//New Line
		if (letter == '\n'){
			i++;
			j = 0;
		} 
	}
	fclose(fp);

  	return board;
}

void send_board(int client_sock, pos_board ** new_board){

	//Envia Lines and cols 
	send(client_sock, &dim, sizeof(dim), 0);

	//Envia o numero de bricks
	send(client_sock, &n_bricks, sizeof(int), 0);

	int aux[2];

	//Envia as coordenadas do Brick
	for (int i = 0; i < dim[1]; ++i)
	{
		for (int j = 0; j < dim[0]; ++j)
		{			
			if(new_board[i][j].object == 'B'){
				aux[0] = j; //Colunas
				aux[1] = i; //Linhas
				send(client_sock, &aux, sizeof(aux), 0);
			}		
		}		
	}

	//send(sock_fd, &new_board, sizeof(new_board), 0);
	//printf("Enviou estrutura\n");
}


player_id * init_player (player_id * new_player, pos_board ** new_board, colour c_colour, int client_sock, pthread_t player_thread){

		new_player = malloc(sizeof(player_id));  

		new_player->player_n = n_player;
		new_player->thread_id = player_thread;	

		int c = dim[0]; 
		int l = dim[1];
		int aux_monster[2];
		int aux_pacman[2];


		//DEFINIR PRIMEIRA POSICAO DO PACMAN
		while (1){ 
			aux_pacman[0]=rand()%(l-1);
			aux_pacman[1]=rand()%(c-1);
			//printf("Posicao possivel: %d %d\n", aux_pacman[0], aux_pacman[1] );
			if(new_board[aux_pacman[0]][aux_pacman[1]].object == ' ') break;
		}
		new_player->pos_pacman[0]=aux_pacman[0];
		new_player->pos_pacman[1]=aux_pacman[1];

		//DEFINIR PRIMEIRA POSICAO DO MONSTER
		while(1){
			aux_monster[0]=rand()%(l-1);
			aux_monster[1]=rand()%(c-1);
			//printf("Posicao possivel: %d %d\n", aux_monster[0], aux_monster[1] );
			if( new_board[aux_monster[0]][aux_monster[1]].object == ' ' && ((aux_pacman[0]!=aux_monster[0]) && (aux_pacman[1]!=aux_monster[1])) )
				break;
		}	
		new_player->pos_monster[0]=aux_monster[0];
		new_player->pos_monster[1]=aux_monster[1];

		//Cor do Jogador
		new_player->rgb[0] = c_colour.r;
		new_player->rgb[1] = c_colour.g;
		new_player->rgb[2] = c_colour.b;
		
		new_player->sock_id = client_sock;
		new_player->next = NULL;

		//printf(" sock_id client no init - %d\n", client_sock);

		return(new_player);
}


void board_update (pos_board ** new_board, player_id  * new_player){

	//PRIMEIRAS POSIÇOES - Pacman
	new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]].object = 'P';
	new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]].sock_id = new_player-> sock_id;
	new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]].r = new_player-> rgb[0];
	new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]].g = new_player-> rgb[1];
	new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]].b = new_player-> rgb[2];
	new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]].x_next = new_player-> pos_pacman[1];
	new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]].y_next = new_player-> pos_pacman[0];

	new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]].x = -1;
	new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]].y = -1;

	//PRIMEIRAS POSIÇOES - Monster
	new_board[new_player -> pos_monster[0]][new_player->pos_monster[1]].object = 'M';
	new_board[new_player -> pos_monster[0]][new_player->pos_monster[1]].sock_id = new_player-> sock_id;
	new_board[new_player -> pos_monster[0]][new_player->pos_monster[1]].r = new_player-> rgb[0];
	new_board[new_player -> pos_monster[0]][new_player->pos_monster[1]].g = new_player-> rgb[1];
	new_board[new_player -> pos_monster[0]][new_player->pos_monster[1]].b = new_player-> rgb[2];
	new_board[new_player -> pos_monster[0]][new_player->pos_monster[1]].x_next = new_player-> pos_monster[1];
	new_board[new_player -> pos_monster[0]][new_player->pos_monster[1]].y_next = new_player-> pos_monster[0];

	new_board[new_player -> pos_monster[0]][new_player->pos_monster[1]].x = -1;
	new_board[new_player -> pos_monster[0]][new_player->pos_monster[1]].y = -1;
}



/*
player_id * list_player(player_id * new_player, player_id* head){   

	if (head == NULL) {	  						
		head = new_player;
		return(head);
	}

	player_id * run = head;

	while (run -> next != NULL)
		run = run -> next;
			
	run -> next = malloc(sizeof(player_id));	
	run -> next = new_player;
	return (head);
}	


player_id * find_player (player_id * head, pid_t npid){

	player_id * aux = head;
	while(aux){
		if (aux -> player_pid == npid) return(aux);

		aux = aux->next;
	}

	printf("NAO ENCONTROU NA LISTA TA FDD\n");
	exit(-1);
}


void send_spawn(player_id * player, player_id * head){

	player_id * aux = head;
	player_id msg;

	int sock;
	pid_t npid = player -> player_pid;

	msg.rgb[0] = player -> rgb[0];
	msg.rgb[1] = player -> rgb[1];
	msg.rgb[2] = player -> rgb[2];

	msg.pos_pacman[0] = player -> pos_pacman[0];
	msg.pos_pacman[1] = player -> pos_pacman[1];

	msg.pos_monster[0] = player -> pos_monster[0];
	msg.pos_monster[1] = player -> pos_monster[1];

	while(aux){
		if (aux -> player_pid != npid){ 
			sock = aux -> sock_id;
			send(sock, &msg, sizeof(msg), 0);			
		}
	aux = aux->next;
	}
}


void send_board2(int client_sock, player_id* head){

	int rgb[3];
	int pos1[2];
	int pos2[2];

	player_id * aux = head;
	while(aux){

		//mandar informacoes para o novo jogador
			rgb[0] = aux -> rgb[0];
			rgb[1] = aux -> rgb[1];
			rgb[2] = aux -> rgb[2];
			//printf("rgb - %d %d %d \n", rgb[0], rgb[1], rgb[2]);
			send(client_sock, &rgb, sizeof(rgb), 0);

			pos1[0] = aux -> pos_pacman[0];
			pos1[1] = aux -> pos_pacman[1];
			send(client_sock, &pos1, sizeof(pos1), 0);
		
			pos2[0] = aux -> pos_monster[0];
			pos2[1] = aux -> pos_monster[1];
			send(client_sock, &pos2, sizeof(pos2), 0);

			aux = aux -> next;
	}
}
*/

