
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
#include "PROJ.h"
#include "sock_init.h"

#include <pthread.h>



#define MaxPlayers 500


Uint32 Event_ShowCharacter;
int sock_fd;


int server_socket;

/*void * serverThread(void* argc){

	ex4_message msg;
	ex4_message *event_data;

	int err_rcv; 
	struct sockaddr_in client_addr;
	socklen_t size_addr = sizeof(client_addr);

	SDL_Event new_event;

	while(1){ 
		printf("waiting for connections\n");
		sock_fd = accept(server_socket, (struct sockaddr*) &client_addr, &size_addr);
		if (sock_fd==-1)
		{
			perror("accept");
			exit(-1);
		}
		printf("accepted connection\n");

		while((err_rcv = recv(sock_fd, &msg, sizeof(msg), 0))>0){

			printf("received %d bytes %d %d %d \n", err_rcv,  msg.character, msg.x, msg.y);
			event_data = malloc(sizeof(ex4_message));
			*event_data = msg;

			SDL_zero(new_event);
			new_event.type = Event_ShowCharacter;
			new_event.user.data1 = event_data;

			SDL_PushEvent(&new_event);

		}
		
 	}
	return(NULL);
}*/


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
   /*int board[line][col];
   memset( board, 0 ,line*col*sizeof(int));*/

   char letter;

   	char ** board;
	int i, j;
	board = malloc(sizeof(char *) * line);           /*MUDEI PARA line+1*/
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

   	int v = 2;

   fclose(fp);

   // Teste
  /* int row, columns;
   for (row=0; row<line; row++)
   {
       for(columns=0; columns<col; columns++)
       {
            printf("%d ", game_board.board[row][columns]);
       }
       printf("\n");
   } */

   return game_board;
}



/*adiciona novo jogador ao fim da lista e preenche-o*/
/*
player_id* new_player(pid_t npid, int client_sock, player_id* head, int n_player, int dim[], board_info new_board){

	int aux_monster[2];
	int aux_pacman[2];


	
	if (head == NULL) {	  						
		head = malloc(sizeof(player_id));

		if (head == NULL){
			perror("malloc");
			exit(-1);
		}
		printf("malloc\n");

		head -> player_pid = npid; 
		head -> player_n = n_player;	


		//	DEFINIR PRIMEIRA POSICAO DO PACMAN

		aux_pacman[0] = rand()%(dim[0]-1);
		aux_pacman[1] = rand()%(dim[1]-1);
		while (new_board.board[aux_pacman[0]][aux_pacman[1]] != ' '){ 
			aux_pacman[0]=rand()%(dim[0]-1);
			aux_pacman[1]=rand()%(dim[1]-1);
		}
		head -> pos_pacman[0]=aux_pacman[0] ;
		head -> pos_pacman[1]=aux_pacman[1];


		//DEFINIR PRIMEIRA POSICAO DO MONSTER

		aux_monster[0] = rand()%(dim[0]-1);
		aux_monster[1] = rand()%(dim[1]-1);
		while (new_board.board[aux_monster[0]][aux_monster[1]] != ' ' && ((aux_pacman[0]==aux_monster[0]) && (aux_pacman[1]==aux_monster[1]))){ 
			aux_monster[0]=rand()%(dim[0]-1);
			aux_monster[1]=rand()%(dim[1]-1);
		}
		head -> pos_monster[0]=aux_monster[0];
		head -> pos_monster[1]=aux_monster[1];


		head -> rgb[0] = rand()%255;
		head -> rgb[1] = rand()%255;
		head -> rgb[2] = rand()%255;
		
		head -> sock_id = client_sock;
		head -> next = NULL;

		return(head);
	}

	//nao é o primeiro
	 
	printf("jogador %d entra\n", n_player);
	player_id* run = head;

	while (run -> next != NULL)
		run = run->next;		
			
	run -> next = malloc(sizeof(player_id));
	
	run -> next -> player_pid = npid;
	run -> next -> player_n = n_player;

	
	//	DEFINIR PRIMEIRA POSICAO DO PACMAN
	printf("antes primeiro while\n");

	aux_pacman[0] = rand()%(dim[0]-1);
	aux_pacman[1] = rand()%(dim[1]-1);
	while (new_board.board[aux_pacman[0]][aux_pacman[1]] != ' '){ 
		aux_pacman[0]=rand()%(dim[0]-1);
		aux_pacman[1]=rand()%(dim[1]-1);
		printf("dentro primeiro while\n");
	}
	run -> next -> pos_pacman[0]=aux_pacman[0];
	run -> next -> pos_pacman[1]=aux_pacman[1];


	//DEFINIR PRIMEIRA POSICAO DO MONSTER
	printf("antes segundo while\n");

	aux_monster[0] = rand()%(dim[0]-1);
	aux_monster[1] = rand()%(dim[1]-1);
	while (new_board.board[aux_monster[0]][aux_monster[1]] != ' ' && ((aux_pacman[0]==aux_monster[0]) && (aux_pacman[1]==aux_monster[1]))){ 
		aux_monster[0]=rand()%(dim[0]-1);
		aux_monster[1]=rand()%(dim[1]-1);
	}
	run -> next -> pos_monster[0]=aux_monster[0];
	run -> next -> pos_monster[1]=aux_monster[1];


	printf("depois segundo while\n");

	run -> next -> rgb[0] = rand()%255;
	run -> next -> rgb[1] = rand()%255;
	run -> next -> rgb[2] = rand()%255;

	run -> next -> sock_id = client_sock;			
	run -> next -> next = NULL;

	printf("fim new_player\n");

	return (head);
}	
*/


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


int main(int argc, char* argv[]){

	//SDL_Event event;
	//int done = 0;
	//int is_server = 1;

	Event_ShowCharacter = SDL_RegisterEvents(1);
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

	//pthread_t thread_id;
	int err_rcv;

	int first_pos[2];

	int pos[2];
	int rgb[3];



	while(1){ 

    	client_sock = accept(sock_fd, (struct sockaddr*)&client, &size);
		if( client_sock == -1 ){
			perror("accept");
			exit(-1);
		}

		n_player++;
		printf("\nTEMOS %d CRL\n", n_player);

		/*ENVIAR A BOARD INICIAL lida do ficheiro*/
		send_board(client_sock, new_board);


		if ( (err_rcv = recv(client_sock, &npid, sizeof(npid), 0)) > 0 ){
			printf("recebeu o pid -  %d do jogador %d \n", npid, n_player);


			/*VERIFICAR SE HA ESPACO PARA MAIS 1 PLAYER*/
			if (new_board.cols*new_board.lines - new_board.bricks < n_player*2){
				printf("CAN'T FIT ANOTHER ONE -- DJ KHALED  \n");
			}


			else{ 

				player_id  * new_player;//------------------------------------------------------------------------->criar estrutura do novo jogador
				new_player = init_player(new_player, new_board, npid, n_player, client_sock); //---------------->preencher a estrutura
				first_pos[0] = new_player->pos_pacman[0];
				first_pos[1] = new_player->pos_pacman[1];
				board_update ('P', new_board, first_pos);//----------------------------------------------------->atualizar a board com o spawn do pacman

				first_pos[0] = new_player->pos_monster[0];
				first_pos[1] = new_player->pos_monster[1];
				board_update ('M', new_board, first_pos);//----------------------------------------------------->atualizar a board com o spawn do monstro

				head = list_player(new_player, head);


				//head = new_player(npid, client_sock, head, n_player, dim, new_board);   


				


				/*retorna ponteiro para a estrutura do player que queremos*/
				player_id * player = find_player(head, npid);

				printf("found it\n");

				pos[0] = player -> pos_pacman[0];
				pos[1] = player -> pos_pacman[1];
				printf("pos - %d %d \n", pos[0], pos[1]);


				rgb[0] = player -> rgb[0];
				rgb[1] = player -> rgb[1];
				rgb[2] = player -> rgb[2];
				printf("rgb - %d %d %d \n", rgb[0], rgb[1], rgb[2]);

				//send(client_sock, &rgb, sizeof(rgb), 0);

				/*new_board = board_update ('P', new_board, pos);
				send(client_sock, &pos, sizeof(pos), 0);
			
				pos[0] = head -> pos_monster[0];
				pos[1] = head -> pos_monster[1];
				new_board = board_update ('M', new_board, pos);
				send(client_sock, &pos, sizeof(pos), 0);

				*/


				//Verificar se a lista esta a ficar feita
				player_id * aux = head;
				while(aux){
					printf("O jogador %d esta na lista\n", aux->player_n);
					aux = aux -> next;
				}
			}
		}
 	}
 	return (0);	
}


board_info board_update (char item, board_info board, int pos[2]){

	/*PRIMEIRAS POSIÇOES*/
	board.board[pos[0]][pos[1]] = item;
	return(board);

}


player_id* find_player (player_id* head, pid_t npid){

	player_id * aux = head;
	while(aux){
		if (aux -> player_pid == npid) return(aux);

		aux = aux->next;
	}

	printf("NAO ENCONTROU NA LISTA TA FDD\n");
	exit(-1);

}


player_id * init_player (player_id * new_player, board_info new_board, pid_t npid, int n_player, int client_sock){

		
		new_player = malloc(sizeof(player_id));  

		new_player->player_pid = npid; 
		new_player->player_n = n_player;	

		int c = new_board.cols; 
		int l = new_board.lines;

		int aux_monster[2];
		int aux_pacman[2];



		/*	DEFINIR PRIMEIRA POSICAO DO PACMAN*/

		aux_pacman[0] = rand()%(c-1);
		aux_pacman[1] = rand()%(l-1);
		while (new_board.board[aux_pacman[0]][aux_pacman[1]] != ' '){ 
			aux_pacman[0]=rand()%(c-1);
			aux_pacman[1]=rand()%(l-1);
		}
		new_player->pos_pacman[0]=aux_pacman[0] ;
		new_player->pos_pacman[1]=aux_pacman[1];


		/*DEFINIR PRIMEIRA POSICAO DO MONSTER*/

		aux_monster[0] = rand()%(c-1);
		aux_monster[1] = rand()%(l-1);
		while (new_board.board[aux_monster[0]][aux_monster[1]] != ' ' && ((aux_pacman[0]==aux_monster[0]) && (aux_pacman[1]==aux_monster[1]))){ 
			aux_monster[0]=rand()%(c-1);
			aux_monster[1]=rand()%(l-1);
		}
		new_player->pos_monster[0]=aux_monster[0];
		new_player->pos_monster[1]=aux_monster[1];


		new_player->rgb[0] = rand()%255;
		new_player->rgb[1] = rand()%255;
		new_player->rgb[2] = rand()%255;
		
		new_player->sock_id = client_sock;
		new_player->next = NULL;


}


player_id * list_player(player_id * new_player, player_id* head){   


	if (head == NULL) {	  						
			head = new_player;
			return(head);
		}


	player_id* run = head;

	while (run -> next != NULL)
		run = run->next;
			
	run -> next = malloc(sizeof(player_id));	/*fazer malloc no init_player???*/
	run -> next = new_player;
	return (head);
}	

