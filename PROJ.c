
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

	int n_lines, n_cols;
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

   int board[line][col];
   memset( board, 0 ,line*col*sizeof(int));

   char letter;

   int j = 0;
   int i = -1;

   while( (letter = getc(fp)) != EOF){
      
      //Block
      if(letter == 'B') game_board.board[i][j] = 1;

      //New Column
      j++;

      //New Line
      if (letter == '\n'){
         i++;
         j = 0;
      } 
   }

   fclose(fp);

   // Teste
   int row, columns;
   for (row=0; row<line; row++)
   {
       for(columns=0; columns<col; columns++)
       {
            printf("%d ", game_board.board[row][columns]);
       }
       printf("\n");
   } 

   return game_board;
}





int main(int argc, char* argv[]){

	SDL_Event event;
	int done = 0;
	int is_server = 1;

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


	//INIT estrutura estrutura da lista de jogadores
	player_id * head;
	pid_t npid;

	pthread_t thread_id;
	int err_rcv;


	
	while(1){ 

    	client_sock = accept(sock_fd, (struct sockaddr*)&client, &size);
		if( client_sock == -1 ){
			perror("accept");
			exit(-1);
		}

		n_player++;
		printf("\nTEMOS %d CRL\n", n_player);
		send(client_sock, &dim, sizeof(dim), 0);

		if (err_rcv = recv(client_sock, &npid, sizeof(npid), 0)>0){
			printf("recebeu o pid -  %d \n", npid);

			head = new_player(npid, client_sock, head, n_player, dim);   /*ADD PLAYER TO LIST OF player_id*/

			/*Verificar se a lista esta a ficar feita*/
			player_id * aux = head;
			while(aux){
				printf("O jogador %d esta na lista\n", aux->player_n);
				aux = aux -> next;
			}

		}

		

 	}
 	return (0);	
}



player_id* new_player(pid_t npid, int client_sock, player_id* head, int n_player, int dim[]){

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

		head -> pos_monster[0]=rand()%dim[0];
		head -> pos_monster[1]=rand()%dim[1];


		head -> rgb[0] = rand()%255;
		head -> rgb[1] = rand()%255;
		head -> rgb[2] = rand()%255;
		
		head -> sock_id = client_sock;
		head -> next = NULL;

		return(head);
		}

		//verificar que e um novo jogador
		player_id* run = head;

		while (run -> player_pid != npid){					//nao é o primeiro

			if(run -> next == NULL){
														//se sim adicionalo a lista, preenche-lo e retornalo
				//n_player_aux++;
				run -> next = malloc(sizeof(player_id));
				
				run -> next -> player_pid = npid;
				run -> next -> player_n = n_player;

			//FALTA FUNCAO QUE CORRE BOARD E VERIFICA POSICOES DISPONIVEIS 

				run -> next -> pos_pacman[0]=rand()%dim[0];
				run -> next -> pos_pacman[1]=rand()%dim[1];

				run -> next -> pos_monster[0]=rand()%dim[0];
				run -> next -> pos_monster[1]=rand()%dim[1];
				
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