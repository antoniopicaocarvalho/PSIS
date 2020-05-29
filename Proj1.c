//gcc teste.c UI_library.c -o teste-UI -lSDL2 -lSDL2_image
#include <SDL2/SDL.h>

//to use internet domain sockets:
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "UI_library.h"
#include "Proj1.h"
#include "sock_init.h"

#include <pthread.h>


int sock_fd;
int server_socket;


int dim[2];// 0 -colunas  1-linhas
//x - colunas y-linhas
int n_bricks;

int comms[50]={0};  //array de socket_ids

int n_player = 0;



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
	int first_pos[2];

	pthread_t player_thread;


	player_id * head = NULL;

	while(1){ 
		
		client_sock = accept(sock_fd, (struct sockaddr*)&client, &size);

		if( client_sock == -1 ){
			perror("accept");
			exit(-1);
		}

		n_player++;
		printf("\n-- Entrou um jogador (Total: %d) --\n", n_player);

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
			new_player = init_player(new_player, new_board, npid, client_sock, player_thread); 
			
			board_update (new_board, new_player);
			
			//Ligar jogador a lista de Jogadores
			//head = list_player(new_player, head);

			//atualizar o array de sock_ids
			comms[n_player-1] = new_player-> sock_id;


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
	clean.object = ' ';
	int sock = comms[n_player-1];
	int k = 0;

	pthread_mutex_t board_mutex;

	clock_t start, aux, t_play, tt;
    double cpu_time_used;
    int count = 0;

	

	for (int i = 0; i < dim[1]; ++i)
	{
		for (int j = 0; j < dim[0]; ++j)
		{
			if(((pos_board**)input)[i][j].object=='M' || ((pos_board**)input)[i][j].object=='P'){

					send(sock, &((pos_board**)input)[i][j], sizeof (pos_board), 0);
			}
		}
	}
	start = clock();
	double aux_t = 0;
		while((err_rcv = recv(sock, &play , sizeof(pos_board), 0)) >0 ){
			tt = clock();
			cpu_time_used = (double)(tt - start) / CLOCKS_PER_SEC;
			printf("cpu time - %f\n", cpu_time_used);
			if (floor(cpu_time_used) - aux_t > 0){
				count = 0;
				printf("count zerou\n");
				aux_t = floor(cpu_time_used);
			} 
	    	if (play.object == 'q'){

	    		pthread_mutex_lock(&board_mutex);
	    		((pos_board**)input)[play.y_next][play.x_next] = clean;
	    		pthread_mutex_unlock(&board_mutex);

	    		 for (int i = 0; i < n_player; ++i) if (comms[i]!=sock) send(comms[i], &play, sizeof (pos_board), 0);  
	    	}
	    	else if (play.object == 'Q')
	    	{
	    		pthread_mutex_lock(&board_mutex);
	    		((pos_board**)input)[play.y_next][play.x_next] = clean;
	    		pthread_mutex_unlock(&board_mutex);

	    		n_player --;
	    		k=0;
	    		while (comms[k] != play.sock_id) k++;
	    		if (k!=n_player) comms[k] = comms[n_player];
	    		for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
	    		printf("\n-- Saiu um jogador (Total: %d) --\n", n_player);
	    	}
	    	else{  
	    		t_play = clock();
	    		count ++;

	    		if (count <= 2){
	    			pos_board aux = ((pos_board**)input)[play.y_next][play.x_next];
	    			if((aux.object == 'P' && play.object == 'P') ||  
	    			(aux.object == 'M' && play.object == 'M') || (aux.r == play.r && aux.g == play.g && aux.b == play.b)){

	    			//start time
	    			//init play_count
	    			//play_count++;
	    			//if(count == 2 && time)

	    			aux.x_next = play.x;
					aux.y_next = play.y;
					aux.x = play.x_next;
					aux.y = play.y_next;

					pthread_mutex_lock(&board_mutex);
	    			((pos_board**)input)[play.y][play.x] = aux;
	    			pthread_mutex_unlock(&board_mutex);

					for (int i = 0; i < n_player; ++i) send(comms[i], &aux, sizeof (pos_board), 0);
					play.x = play.x_next;
					play.y = play.y_next;

			    	pthread_mutex_lock(&board_mutex);
	    			((pos_board**)input)[play.y_next][play.x_next] = play;
	    			pthread_mutex_unlock(&board_mutex);

			    	for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
					//Jogada Realizada
					printf("%c - next:  %d %d,  prev:  %d %d\n",play.object, play.x_next, play.y_next, play.x, play.y);

		    		}
			    	else{

				    	pthread_mutex_lock(&board_mutex);
		    			((pos_board**)input)[play.y_next][play.x_next] = play;
		    			pthread_mutex_unlock(&board_mutex);

				    	pthread_mutex_lock(&board_mutex);
		    			((pos_board**)input)[play.y][play.x] = clean;
		    			pthread_mutex_unlock(&board_mutex);

				    	for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
				    	//Jogada Realizada
						printf("%c - next:  %d %d,  prev:  %d %d\n",play.object, play.x_next, play.y_next, play.x, play.y);
			    	}
	    			
	    		}
	    	}
		}
	
	return (NULL);
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


player_id * init_player (player_id * new_player, pos_board ** new_board, pid_t npid, int client_sock, pthread_t player_thread){

		new_player = malloc(sizeof(player_id));  

		new_player->player_pid = npid; 
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
		new_player->rgb[0] = rand()%255;
		new_player->rgb[1] = rand()%255;
		new_player->rgb[2] = rand()%255;
		
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

/*
player_id * find_player (player_id * head, pid_t npid){

	player_id * aux = head;
	while(aux){
		if (aux -> player_pid == npid) return(aux);

		aux = aux->next;
	}

	printf("NAO ENCONTROU NA LISTA TA FDD\n");
	exit(-1);
}
/*

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