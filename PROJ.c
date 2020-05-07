
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
	board = malloc(sizeof(char *) * line);
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




player_id* new_player(pid_t npid, int client_sock, player_id* head, int n_player, int dim[], board_info new_board){

	if (head == NULL) {	  			//lista vazia - primeiro jogador
		head = malloc(sizeof(player_id));

		if (head == NULL){
			perror("malloc");
			exit(-1);
		}

		head -> player_pid = npid; 
		head -> player_n = n_player;				//arranjar uma funcao que receba o nº do jogador e retorne um rgb[3] decente
		
		head -> pos_pacman[0]=rand()%dim[0];
		head -> pos_pacman[1]=rand()%dim[1];
		while (new_board.board[head->pos_pacman[0]][head->pos_pacman[1]] != ' '){ 
			head -> pos_pacman[0]=rand()%dim[0];
			head -> pos_pacman[1]=rand()%dim[1];
		}

		head -> pos_monster[0]=rand()%dim[0];
		head -> pos_monster[1]=rand()%dim[1];
		while (new_board.board[head->pos_monster[0]][head->pos_monster[1]] != ' '){ 
			head -> pos_monster[0]=rand()%dim[0];
			head -> pos_monster[1]=rand()%dim[1];
		}

		head -> rgb[0] = rand()%255;
		head -> rgb[1] = rand()%255;
		head -> rgb[2] = rand()%255;
		
		head -> sock_id = client_sock;
		head -> next = NULL;

		return(head);
		}

		//verificar que e um novo jogador
		player_id* run = head;

		while (run -> player_pid != npid){		//TALVEZ TENHA DE TROCAR PARA =! NULL

			if(run -> next == NULL){
														//se sim adicionalo a lista, preenche-lo e retornalo
				//n_player_aux++;
				run -> next = malloc(sizeof(player_id));
				
				run -> next -> player_pid = npid;
				run -> next -> player_n = n_player;

			run -> next -> pos_pacman[0]=rand()%dim[0];
			run -> next -> pos_pacman[1]=rand()%dim[1];
			while (new_board.board[head->pos_pacman[0]][head->pos_pacman[1]] != ' '){ 
				run -> next -> pos_pacman[0]=rand()%dim[0];
				run -> next -> pos_pacman[1]=rand()%dim[1];
			}

			run -> next -> pos_monster[0]=rand()%dim[0];
			run -> next -> pos_monster[1]=rand()%dim[1];
			while (new_board.board[head->pos_monster[0]][head->pos_monster[1]] != ' '){ 
				run -> next -> pos_monster[0]=rand()%dim[0];
				run -> next -> pos_monster[1]=rand()%dim[1];
			}

				run -> next -> rgb[0] = rand()%255;
				run -> next -> rgb[1] = rand()%255;
				run -> next -> rgb[2] = rand()%255;

				run -> next -> sock_id = client_sock;			
				run -> next -> next = NULL;

				return (head);
			}	
			run = run->next;	

						
		}
		return(head);		//retorna a estrutura correspondente ao npid se este ja existir na lista

}

/*
board_info serialize(board_info new_board){

	int size = 2 + new_board.cols*new_board.lines;
	int dim[size];
	int v = 2;

	//printf("\n\nColunas: %d, Linhas: %d, Size: %d\n", new_board.cols,new_board.lines,size );
	
	new_board.dim[0] = new_board.lines;
	new_board.dim[1] = new_board.cols;

	for (int i = 0; i < new_board.lines; ++i)
	{
		for (int j = 0; j < new_board.cols; ++j)
		{			
			if(new_board.board[i][j] == 'B') new_board.dim[v] = 1;
			else new_board.dim[v] = 0;

			v++;
		}		
	}

	return new_board;
}*/

/*void display_(board_info new_board){
	
	printf("\n");

	for( int a = 2; a < 2+new_board.cols*new_board.lines; a++){
		
		if(a-2 == 10 || a-2 == 20 || a-2 == 30 || a-2 == 40 || a-2 == 50
			|| a-2 == 60 || a-2 == 70 || a-2 == 80 || a-2 == 90) 
			printf("\n");

		printf("%d ", new_board.dim[a] );
	}

	printf("\n");
}*/
	
/*

	//creates a windows and a board with 50x20 cases
	create_board_window(new_board.cols, new_board.lines);

	//monster and packman position
	int x = 0;
	int y = 0;

	int x_other =0;
	int y_other =0;



	while (!done){
		while (SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) {
					done = SDL_TRUE;
			}

			if(event.type == Event_ShowCharacter) {

				ex4_message * data_ptr;
				data_ptr = event.user.data1;

				clear_place(x_other,y_other);

				x_other = data_ptr->x;
				y_other = data_ptr->y;

				if (is_server)
				{
					paint_monster(x_other, y_other, 7, 200, 100);
				}else{
					paint_pacman(x_other, y_other, 7, 100, 200);
				}

				printf("new event received\n");
			}



			//when the mouse mooves the monster also moves
			if(event.type == SDL_MOUSEMOTION){
				int x_new, y_new;

				//this fucntion return the place cwher the mouse cursor is
				get_board_place(event.motion .x, event.motion .y,
												&x_new, &y_new);
				//if the mluse moved toi anothe place
				if((x_new != x) || (y_new != y)){
					//the old place is cleared
					clear_place(x, y);
					x = x_new;
					y=y_new;
					//decide what color to paint the monster
					//and paint it
					ex4_message msg;

					if(is_server==1){
						paint_pacman(x, y , 200, 100, 7);
						msg.character = 2;
					}else{
						paint_monster(x, y , 7, 200, 100);
						msg.character = 2;
					}
					printf("move x-%d y-%d\n", x,y);
					msg.x=x;
					msg.y=y;
					send(sock_fd, &msg, sizeof(msg), 0);
				}
			}
		}
	}
	
	printf("fim\n");
	close_board_windows();
	exit(0);
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
	printf("%d\n", dim[0]);
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
	player_id * head;
	pid_t npid;

	//pthread_t thread_id;
	int err_rcv;






	while(1){ 

    	client_sock = accept(sock_fd, (struct sockaddr*)&client, &size);
		if( client_sock == -1 ){
			perror("accept");
			exit(-1);
		}

		n_player++;
		printf("\nTEMOS %d CRL\n", n_player);

		/*SERIALIZE STRUCTURE*/

		//display_(new_board);
	
		//send(client_sock, &new_board.dim, sizeof(new_board.dim), 0);

		send_board(client_sock, new_board);
		

		if ( (err_rcv = recv(client_sock, &npid, sizeof(npid), 0)) > 0 ){
			printf("recebeu o pid -  %d \n", npid);

			/*VERIFICAR SE HA ESPACO PARA MAIS 1 PLAYER*/
			if (new_board.cols*new_board.lines - new_board.bricks < n_player*2){
				printf("CAN'T FIT ANOTHER ONE -- DJ KHALED  \n");
			}
			else{ 

				head = new_player(npid, client_sock, head, n_player, dim, new_board);   //ADD PLAYER TO LIST OF player_id
				
				int pos[2];
				int rgb[3];
				pos[0] = head -> pos_pacman[0];
				pos[1] = head -> pos_pacman[1];
				rgb[0] = head -> rgb[0];
				rgb[1] = head -> rgb[1];
				rgb[2] = head -> rgb[2];

				send(client_sock, &rgb, sizeof(rgb), 0);

				new_board = board_update ('P', new_board, pos);
				send(client_sock, &pos, sizeof(pos), 0);
			
				pos[0] = head -> pos_monster[0];
				pos[1] = head -> pos_monster[1];
				new_board = board_update ('M', new_board, pos);
				send(client_sock, &pos, sizeof(pos), 0);




				//Verificar se a lista esta a ficar feita
				/*player_id * aux = head;
				while(aux){
					printf("O jogador %d esta na lista\n", aux->player_n);
					aux = aux -> next;
				}*/
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
