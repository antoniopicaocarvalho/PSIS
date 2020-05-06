#include "UI_library.h"
#include "sock_init.h"
#include <sys/types.h>
#include <unistd.h>
#include "PROJ.h"


/*void* paint_places (void *arg){

	int *dim_board=(int*) arg;
	create_board_window(300, 300, *dim_board);
	int done=0;
	play_response resp;
	while(!done){
		read(sock_fd, &resp, sizeof(struct play_response)); 
		printf("Resposta do programa:\nCode:%d\nCoordenadas:(%d,%d)\nConteúdo da casa: %s\n", resp.code, resp.play1[0], resp.play1[1], resp.str_play1);
		printf("Conecção com o proprio jogador[0] com os outros[1]: %d\n", resp.sock);
		if(resp.sock==0){
			switch (resp.code) {
				case 1: 
					paint_card(resp.play1[0], resp.play1[1], 212, 212, 212);
					write_card(resp.play1[0], resp.play1[1], resp.str_play1, 100, 100, 100);
					break;
				case 3:
					paint_card(resp.play1[0], resp.play1[1], resp.rgb[0], resp.rgb[1], resp.rgb[2]);
					write_card(resp.play1[0], resp.play1[1], resp.str_play1, 255, 255, 255);
						
					paint_card(resp.play2[0], resp.play2[1], resp.rgb[0], resp.rgb[1], resp.rgb[2]);
					write_card(resp.play2[0], resp.play2[1], resp.str_play2, 255, 255, 255);
				 	break;
				case 2:	
					paint_card(resp.play1[0], resp.play1[1], resp.rgb[0], resp.rgb[1], resp.rgb[2]);
					write_card(resp.play1[0], resp.play1[1], resp.str_play1, 255, 255, 255);
						
					paint_card(resp.play2[0], resp.play2[1], resp.rgb[0], resp.rgb[1], resp.rgb[2]);
					write_card(resp.play2[0], resp.play2[1], resp.str_play2, 255, 255, 255);
					break;
				case -2: 
					paint_card(resp.play1[0], resp.play1[1], 212, 212, 212);
					write_card(resp.play1[0], resp.play1[1], resp.str_play1, 100, 100, 100);
							
					paint_card(resp.play2[0], resp.play2[1], 212, 212, 212);
					write_card(resp.play2[0], resp.play2[1], resp.str_play2, 100, 100, 100);
							
					sleep(2);

					clear_card(resp.play1[0], resp.play1[1]);
					clear_card(resp.play2[0], resp.play2[1]);
					break;
			}
		}else if(resp.sock==1){
			switch (resp.code) {
				case 1:
					paint_card(resp.play1[0], resp.play1[1], 100, 100, 100);
					paint_limit_card(resp.play1[0], resp.play1[1], resp.rgb[0], resp.rgb[1], resp.rgb[2]);
					write_card(resp.play1[0], resp.play1[1], resp.str_play1, resp.rgb[0], resp.rgb[1], resp.rgb[2]);
					break;
				case 3: 
					paint_card(resp.play1[0], resp.play1[1], resp.rgb[0], resp.rgb[1], resp.rgb[2]);
					write_card(resp.play1[0], resp.play1[1], resp.str_play1, 255, 255, 255);
					
					paint_card(resp.play2[0], resp.play2[1], resp.rgb[0], resp.rgb[1], resp.rgb[2]);
					write_card(resp.play2[0], resp.play2[1], resp.str_play2, 255, 255, 255);
					break;
				case 2:	
					paint_card(resp.play1[0], resp.play1[1], resp.rgb[0], resp.rgb[1], resp.rgb[2]);
					write_card(resp.play1[0], resp.play1[1], resp.str_play1, 255, 255, 255);
					
					paint_card(resp.play2[0], resp.play2[1], resp.rgb[0], resp.rgb[1], resp.rgb[2]);
					write_card(resp.play2[0], resp.play2[1], resp.str_play2, 255, 255, 255);
					break;
				case -2: 
					paint_card(resp.play1[0], resp.play1[1] , 100, 100, 100);
					paint_limit_card(resp.play1[0], resp.play1[1], resp.rgb[0], resp.rgb[1], resp.rgb[2]);
					write_card(resp.play1[0], resp.play1[1], resp.str_play1,resp.rgb[0], resp.rgb[1], resp.rgb[2]);
					
					paint_card(resp.play2[0], resp.play2[1] , 100, 100, 100);
					paint_limit_card(resp.play2[0], resp.play2[1], resp.rgb[0], resp.rgb[1], resp.rgb[2]);
					write_card(resp.play2[0], resp.play2[1], resp.str_play2,resp.rgb[0], resp.rgb[1], resp.rgb[2]);
					
					sleep(1);
					
					clear_card(resp.play1[0], resp.play1[1]);
					clear_card(resp.play2[0], resp.play2[1]);
					break;
				case -1:
					clear_card(resp.play1[0], resp.play1[1]);
			}
		}else if(resp.sock=2){
			printf("--------<Game_Stats>-----------\n\n");
			if(resp.place==1){
				printf("You are the Winner, Congrats, mah friend!!\n");
			}
			printf("You made %d pairs, Place: %d\n\n", resp.n_pairs, resp.place);
			printf("--------<New_Game>-----------\n\n");

			for(int i=0; i<*dim_board; i++){
				for(int j=0; j<*dim_board; j++){
					clear_card(i,j);
				}
			}
		}
	}
	
	pthread_exit(NULL);
	close_board_windows();
}


void * clientThread(void* argc){

	ex4_message msg;
	ex4_message *event_data;
	SDL_Event new_event;
	int err_rcv; 

	printf("just connected to the server\n");

	while((err_rcv = recv(sock_fd, &msg, sizeof(msg), 0))>0){

		printf("received %d bytes %d %d %d \n", err_rcv,  msg.character, msg.x, msg.y);

			event_data = malloc(sizeof(ex4_message));
			*event_data = msg;

			SDL_zero(new_event);
			new_event.type = Event_ShowCharacter;
			new_event.user.data1 = event_data;

			SDL_PushEvent(&new_event);

	}

	return(NULL);
}*/


/*---------------------------------------client.c---------------------------------------------------------*/
int main(int argc, char * argv[]){



    if (argc <2){//-----------------------------------------------------------------------falta o adress do server
      printf("second argument should be server address\n");
      exit(-1);}

    SDL_Event event;

	struct sockaddr_in server_addr = make_socket(&sock_fd);

	inet_aton(argv[1], &server_addr.sin_addr);

	sock_fd = connect_server (sock_fd, server_addr);

	int dim[2];
	int err_rcv;

	pid_t npid = getpid();


	if (err_rcv = recv(sock_fd, &dim, sizeof(dim), 0)>0){
		printf("received %d bytes %d %d \n", err_rcv,  dim[0], dim[1]);
		create_board_window(dim[0], dim[1]);
	}
	send(sock_fd, &npid, sizeof(npid), 0);

	int done = 0;
	
	while (!done){
		while (SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) 
					done = SDL_TRUE;
			
		}

	}
	printf("fim\n");
	close_board_windows();
	exit(0);
}







	
	/*
	struct sockaddr_in server_addr =  make_socket (&sock_fd);//--------------------------------Socket	    
	inet_aton(argv[1], &server_addr.sin_addr);

	sock_fd = connect_server (sock_fd, server_addr);

	int dim_board;
	int done=0;
	read(sock_fd, &dim_board, sizeof(int));
	pthread_t thread_i;
	pthread_create (&thread_i, NULL, paint_places, &dim_board);	




*/
	
	
