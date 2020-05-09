//gcc teste.c UI_library.c -o teste-UI -lSDL2 -lSDL2_image
#include <SDL2/SDL.h>

//to use internet domain sockets:
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include "UI_library.h"
#include "Proj1.h"
#include "sock_init.h"

#include <pthread.h>




int sock_fd;


int server_socket;

int main(int argc, char* argv[]){

	
	//Leitura do ficheiro BOARD.TXT
	board_info new_board;
	new_board = board_read();


	//Criação da Socket
	struct sockaddr_in local_addr = make_socket(&sock_fd);
	local_addr.sin_addr.s_addr= INADDR_ANY;
	sock_fd = Do_Bind(sock_fd, local_addr);
	Do_Listen(sock_fd);

	printf("Waiting for players...\n");

	struct sockaddr_in client;
	socklen_t size = sizeof (struct sockaddr_in);

	int n_player=0;
	int client_sock;

	int dim[2];
	dim[0] = new_board.cols;
	dim[1] = new_board.lines;
	printf("board %d * %d \n", dim[0], dim[1]);


	//INIT estrutura da lista de jogadores
	player_id * head = NULL;
	
	pid_t npid;

	int err_rcv;
	int first_pos[2];

	int pos1[2];
	int pos2[2];

	int rgb[3];

	pthread_t player_thread;



	while(1){ 

    	client_sock = accept(sock_fd, (struct sockaddr*)&client, &size);
		if( client_sock == -1 ){
			perror("accept");
			exit(-1);
		}

		n_player++;
		printf("\n-- Temos %d jogadores --\n", n_player);

		/*ENVIAR A BOARD INICIAL lida do ficheiro*/
		send_board(client_sock, new_board);

		//RECEBE O PID
		if ( (err_rcv = recv(client_sock, &npid, sizeof(npid), 0)) > 0 )
			printf("recebeu o pid:  %d , do jogador %d \n", npid, n_player);

		//RECEBE O THREAD_ID
		//if ( (err_rcv = recv(client_sock, &player_thread, sizeof(player_thread), 0)) > 0 ){
		//printf("recebeu o thread_id\n");

		/*VERIFICAR SE HA ESPACO PARA MAIS 1 PLAYER*/
		if (new_board.cols*new_board.lines - new_board.bricks < n_player*2){
			printf("CAN'T FIT ANOTHER ONE -- DJ KHALED  \n");
			}

		else{ 
			/*Novo jogador*/
			player_id  * new_player = NULL;
			new_player = init_player(new_player, new_board, npid, n_player, client_sock, player_thread); 
			
			//Posicao do Pacman
			first_pos[0] = new_player->pos_pacman[0];
			first_pos[1] = new_player->pos_pacman[1];

			board_update ('P', new_board, first_pos);
			
			//Posicao do Monster
			first_pos[0] = new_player->pos_monster[0];
			first_pos[1] = new_player->pos_monster[1];

			board_update ('M', new_board, first_pos);

			
			/*Ligar jogador a lista de Jogadores*/
			head = list_player(new_player, head);
			  

			/*retorna ponteiro para a estrutura do player que queremos*/
			player_id * player = find_player(head, npid);

			/*mandar informacoes para o novo jogador*/
			rgb[0] = player -> rgb[0];
			rgb[1] = player -> rgb[1];
			rgb[2] = player -> rgb[2];
			printf("rgb - %d %d %d \n", rgb[0], rgb[1], rgb[2]);
			send(client_sock, &rgb, sizeof(rgb), 0);

			pos1[0] = player -> pos_pacman[0];
			pos1[1] = player -> pos_pacman[1];
			send(client_sock, &pos1, sizeof(pos1), 0);
		
			pos2[0] = player -> pos_monster[0];
			pos2[1] = player -> pos_monster[1];
			send(client_sock, &pos2, sizeof(pos2), 0);

			/*mandar para os que ja la estavam*/
			//send_spawn(player, head);

			


			//Verificar se a lista esta a ficar feita
			/*player_id * aux = head;
			while(aux){
				printf("O jogador %d esta na lista\n", aux->player_n);
				aux = aux -> next;
			}*/
		}
	}
	return (0);	
 }
 	



board_info board_read() {

	int col;
	int line;

	board_info game_board;

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

   game_board.cols = col;
   game_board.lines = line;
   game_board.bricks = 0;

   char letter;

   	char ** board;
	int i, j;
	board = malloc(sizeof(char *) * (line+1));           
	for ( i = 0 ; i < line; i++){
		board[i] = malloc (sizeof(char) * (col+1));
		for (j = 0; j < col; j++){
			board[i][j] = ' ';
		}
		board[i][j] = '\0';
	}

	j = 0;
    i = -1;

    while( (letter = getc(fp)) != EOF){    
		//Block
		if(letter == 'B') {
			board[i][j] = 'B';
			game_board.bricks++;
		}
		//New Column
		j++;
		//New Line
		if (letter == '\n'){
			i++;
			j = 0;
      	} 
    }
    game_board.board = board;

    fclose(fp);

    return game_board;
}

void send_board(int client_sock, board_info new_board){
	//dim 0 -> line, dim 1 -> col
	int dim[2];	
	dim[1] = new_board.lines;
	dim[0] = new_board.cols;

	//Envia Lines and cols 
	send(client_sock, &dim, sizeof(dim), 0);

	//Envia o numero de bricks
	dim[0] = new_board.bricks;
	send(client_sock, &dim, sizeof(dim), 0);

	//Envia as coordenadas do Brick
	for (int i = 0; i < new_board.lines; ++i)
	{
		for (int j = 0; j < new_board.cols; ++j)
		{			
			if(new_board.board[i][j] == 'B'){
				dim[0] = j; //Colunas
				dim[1] = i; //Linhas
				send(client_sock, &dim, sizeof(dim), 0);
			}		
		}		
	}
}


player_id * init_player (player_id * new_player, board_info new_board, pid_t npid, int n_player, int client_sock, pthread_t player_thread){

		new_player = malloc(sizeof(player_id));  

		new_player->player_pid = npid; 
		new_player->player_n = n_player;
		new_player->thread_id = player_thread;	

		int c = new_board.cols; 
		int l = new_board.lines;
		int aux_monster[2];
		int aux_pacman[2];


		/*	DEFINIR PRIMEIRA POSICAO DO PACMAN*/
		while (1){ 
			aux_pacman[0]=rand()%(l-1);
			aux_pacman[1]=rand()%(c-1);
			//printf("Posicao possivel: %d %d\n", aux_pacman[0], aux_pacman[1] );
			if(new_board.board[aux_pacman[0]][aux_pacman[1]] == ' ') break;
		}
		new_player->pos_pacman[0]=aux_pacman[0] ;
		new_player->pos_pacman[1]=aux_pacman[1];

		/*DEFINIR PRIMEIRA POSICAO DO MONSTER*/
		while(1){
			aux_monster[0]=rand()%(l-1);
			aux_monster[1]=rand()%(c-1);
			printf("Posicao possivel: %d %d\n", aux_monster[0], aux_monster[1] );
			if( new_board.board[aux_monster[0]][aux_monster[1]] == ' ' && ((aux_pacman[0]!=aux_monster[0]) && (aux_pacman[1]!=aux_monster[1])) )
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

		return(new_player);
}

board_info board_update (char item, board_info board, int pos[2]){

	/*PRIMEIRAS POSIÇOES*/
	board.board[pos[0]][pos[1]] = item;
	return(board);
}

player_id * list_player(player_id * new_player, player_id* head){   

	if (head == NULL) {	  						
			head = new_player;
			return(head);
		}

	player_id * run = head;

	while (run -> next != NULL)
		run = run -> next;
			
	run -> next = malloc(sizeof(player_id));	/*fazer malloc no init_player???*/
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