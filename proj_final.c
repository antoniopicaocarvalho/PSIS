#include "proj_final.h"


int sock_fd;
int server_socket;
//BOARD DIMENSIONS 
int dim[2];
// 0 -> columns | x  
// 1 -> lines   | y

int n_bricks;

//SCOREBOARD
int points [MAX_SIZE] = {0};

//ARRAY OF SOCKET IDs
int comms[50]={0}; 

//number of current player
//total number of player that were added
int n_player ,total_p;

//COUNTERS FOR 2 PLAYS A SEC
int countp [MAX_SIZE] = {0};
int countm [MAX_SIZE] = {0};

//TIME MANAGEMENT THREAD's FLAG
int done [MAX_SIZE] = {0};

//1- lemon, 2-cherry
int fruit [MAX_SIZE] = {0};

//number of eaten monsters
int edies [MAX_SIZE] = {0};

//movement flag
int player_m[MAX_SIZE] = {0};

int begin_time, actual_time;

pthread_mutex_t board_mutex;


int main(int argc, char* argv[]){

	
	//READ BOARD.TXT
	pos_board ** new_board;
	new_board = board_read();

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

	printf("\n.: WELCOME TO PACMAN:.\n");

	struct sockaddr_in client;
	socklen_t size = sizeof (struct sockaddr_in);

	int client_sock;
	
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
			printf("Error: colour wasn't received.");
			exit(-1);
		}
		
		//SEND BOARD
		send_board(client_sock, new_board);
		//SEND PLAYER ID
		send(client_sock, &client_sock, sizeof(int), 0);


		//CHECK 
		if (dim[0]*dim[1] - n_bricks < (n_player-1)*4+2){
			printf("There isn't space enought to add the new player\n");
			n_player--;
			pos_board quit;
			quit.object='Y';
			send(client_sock, &quit, sizeof(pos_board), 0);

			close(client_sock);
		}
		else{ 
			printf("\n-- New Player (Total: %d) --\n", n_player);

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

	pos_board safe;
	safe.object = 'q';

	int sock = comms[n_player-1];

	int k = total_p;

	int px, py;

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
						((pos_board**)input)[i][j].k = k;
						monster = ((pos_board**)input)[i][j];
					}
					if (((pos_board**)input)[i][j].object=='P'){
						((pos_board**)input)[i][j].time = actual_time;
						((pos_board**)input)[i][j].k = k;
					 	pacman = ((pos_board**)input)[i][j];
					}
				}	
			}
		}
	}

	//TIME MANAGEMENT THREAD
	pthread_t time_thread;
	pthread_create(&time_thread, NULL, thirty_reset, (void *)((pos_board**)input));

	//flag movement
	int m = 0;

	//RECV POSSIBLE PLAY
	while((err_rcv = recv(sock, &play , sizeof(pos_board), 0)) >0 ){

    	if (play.object == 'q'){
    		//clean monster position
    		((pos_board**)input)[play.y_next][play.x_next] = clean;
    		for (int i = 0; i < n_player; ++i) if (comms[i]!=sock) send(comms[i], &play, sizeof (pos_board), 0);  
    	}
	   	else if (play.object == 'Q'){
	   		//clean pacman position and update variables
			((pos_board**)input)[play.y_next][play.x_next] = clean;

			n_player --;

			//update comms array
			int w = 0;
			while (comms[w] != play.sock_id) w++;
			if (w != n_player) comms[w] = comms[n_player];

			close(play.sock_id);

			for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);

			//clean fruit when player exits
			if (n_player > 0){

				pos_board cfruit;
				cfruit.object = 'Q';

				int d, auxf;

				while(!d){ 
	    			for (int i = 0; i < dim[1]; ++i){
						for (int j = 0; j < dim[0]; ++j){
							if (((pos_board**)input)[i][j].object=='L' || ((pos_board**)input)[i][j].object=='C'){
								if(auxf < 2){
									((pos_board**)input)[i][j] = clean;
									cfruit.x_next = j;
									cfruit.y_next = i;
									auxf++;
									for (int n = 0; n < n_player; ++n) send(comms[n], &cfruit, sizeof (pos_board), 0); 
								}
							}
							if(auxf == 2) d = 1;
						}
	    			}
				}
			}	

			//stop time management thread
			done[k] = 1;

			printf("\n-- Player Left (Total: %d) --\n", n_player);
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
					if (((pos_board**)input)[play.y_next][play.x_next].object == 'B'){
						//BOUNCE POS
						int n_x = 2*play.x - play.x_next;
						int n_y = 2*play.y - play.y_next;
						//BOUNCE BACK - INSIDE BOARD
						if ( (n_x < dim[0]) && (n_y < dim[1]) && (n_x >= 0) && (n_y >= 0) ){
							//STUCK
							if (((pos_board**)input)[n_y][n_x].object == 'B'){
								
								play.x_next = play.x;
								play.y_next = play.y;

								//move done!
								m = 1;

								for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
							}
							//CAN BOUNCE BACK
							else{

								play.x_next = n_x;
								play.y_next = n_y;
							}
						}//BOUNCE BACK - OUTSIDE BOARD
						else{	
							play.x_next = play.x;
							play.y_next = play.y;

							//move done!
							m = 1;

							for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
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
						if (((pos_board**)input)[n_y][n_x].object == 'B'){
							
							play.x_next = play.x;
							play.y_next = play.y;

							//move done!
							m = 1;

							for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
						}
						//CAN BOUNCE BACK
						else{

							play.x_next = n_x;
							play.y_next = n_y;
						}
					}//BOUNCE BACK - OUTSIDE BOARD
					else{	
						play.x_next = play.x;
						play.y_next = play.y;

						//move done!
						m = 1;

						for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
					}
		    	}

		    	//CONTINUE TO CHECK PLAY
		    	if(m==0){

		    		pos_board aux = ((pos_board**)input)[play.y_next][play.x_next];

			    	//SWITCH POS
	    			if((aux.object == 'P' && play.object == 'P') || (aux.object == 'S' && play.object == 'P') ||
	    			   (aux.object == 'P' && play.object == 'S') || (aux.object == 'S' && play.object == 'S') ||
	    			   (aux.object == 'M' && play.object == 'M') || (aux.r == play.r && aux.g == play.g && aux.b == play.b)){
	    				
		    			aux.x_next = play.x;
						aux.y_next = play.y;
						aux.x = play.x_next;
						aux.y = play.y_next;

						((pos_board**)input)[aux.y_next][aux.x_next] = aux;

						play.x = play.x_next;
						play.y = play.y_next;
						play.k = k;
		    			((pos_board**)input)[play.y_next][play.x_next] = play;

						for (int i = 0; i < n_player; ++i){
							send(comms[i], &aux, sizeof (pos_board), 0);
				    		send(comms[i], &play, sizeof (pos_board), 0);
				    		}
			    	}
			    	//FRUIT - REST  
			    	else if((aux.object == 'L' && play.object == 'P') || (aux.object == 'L' && play.object == 'S') ||
	    			(aux.object == 'C' && play.object == 'P') || (aux.object == 'C' && play.object == 'S')||
	    			(aux.object == 'L' && play.object == 'M') || (aux.object == 'C' && play.object == 'M')){
			    		points[k]=points[k]+1;
			    		
			    		if(play.object == 'P') play.object = 'S';
				    	else if(play.object == 'S') edies[k]=0;
				    	if (aux.object == 'L') fruit[k] = 1;
		    			if (aux.object == 'C') fruit[k] = 2;

		    			play.k = k;

				    	((pos_board**)input)[play.y][play.x] = clean;
		    			((pos_board**)input)[play.y_next][play.x_next] = play;
	
				    	for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
			    	}
			    	//MONSTRO - PACMAN
			    	else if((aux.object == 'M' && play.object == 'P') || (aux.object == 'P' && play.object == 'M')){

			    		while (1){ 
							py=rand()%(dim[1]);
							px=rand()%(dim[0]);
							if(((pos_board**)input)[py][px].object == ' ') break;								
						}

						// PACMAN EATS MONSTER
						if (play.object == 'P'){
							points[aux.k]=points[aux.k]+1;

							play.x_next = px;
							play.y_next = py;
							play.k = k;

							((pos_board**)input)[aux.y_next][aux.x_next] = aux;
			    			((pos_board**)input)[play.y_next][play.x_next] = play;
			    			((pos_board**)input)[play.y][play.x] = clean;
			    			
							for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
						}
						// MONSTER EATS PACMAN
						else{
							points[k]=points[k]+1;

							aux.x = aux.x_next;
							aux.y = aux.y_next;
							aux.x_next = px;
							aux.y_next = py;

							play.k = k;

			    			((pos_board**)input)[aux.y_next][aux.x_next] = aux;
			    			((pos_board**)input)[play.y][play.x] = clean;
			    			((pos_board**)input)[play.y_next][play.x_next] = play;

							for (int i = 0; i < n_player; ++i) {
								send(comms[i], &aux, sizeof (pos_board), 0);
								send(comms[i], &play, sizeof (pos_board), 0);
							}
						}
			    	}
			    	//MONSTER - SUPERPACMAN
			    	else if((aux.object == 'M' && play.object == 'S') || (aux.object == 'S' && play.object == 'M')){

			    		edies[k]++;
			    		//if superpacman has eaten 2 monsters
			    		if (edies[k]==2){
							if(play.object == 'S') play.object='P';
							else if(aux.object == 'S') aux.object='P';
							edies[k]=0;
						}

			    		while(1){ 
							py=rand()%(dim[1]);
							px=rand()%(dim[0]);
							if(((pos_board**)input)[py][px].object == ' ') break;								
						}

						// MONSTER CLASHES INTO SUPERPACMAN
						if (play.object == 'M'){
							points[aux.k]=points[aux.k]+1;

							play.x_next = px;
							play.y_next = py;
							play.k = k;


							((pos_board**)input)[play.y][play.x] = clean;
							((pos_board**)input)[aux.y_next][aux.x_next] = aux;
			    			((pos_board**)input)[play.y_next][play.x_next] = play;
			    			

							for (int i = 0; i < n_player; ++i) {
								send(comms[i], &aux, sizeof (pos_board), 0);
								send(comms[i], &play, sizeof (pos_board), 0);
							}
						}

						// SUPERPACMAN EATS MONSTER
						else{
							points[k]=points[k]+1;

							aux.x = aux.x_next;
							aux.y = aux.y_next;
							aux.x_next = px;
							aux.y_next = py;

							play.k = k;

							((pos_board**)input)[play.y][play.x] = clean;
			    			((pos_board**)input)[aux.y_next][aux.x_next] = aux;  			
			    			((pos_board**)input)[play.y_next][play.x_next] = play;

							for (int i = 0; i < n_player; ++i) {
								send(comms[i], &aux, sizeof (pos_board), 0);
								send(comms[i], &play, sizeof (pos_board), 0);
							}
						}
					}
					//CAN MOVE
					else{ 	 
						play.k = k;

						((pos_board**)input)[play.y][play.x] = clean;
						((pos_board**)input)[play.y_next][play.x_next] = play;
	    				
	    				for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
					}
				}

				m = 0;

				//CLEAN DUPLICATES
				for (int i = 0; i < dim[1]; ++i)
				{
					for (int j = 0; j < dim[0]; ++j)
					{
					
						if(((pos_board**)input)[i][j].object == play.object && 
						   ((pos_board**)input)[i][j].sock_id == play.sock_id &&
						   ((pos_board**)input)[i][j].x_next != play.x_next &&
						   ((pos_board**)input)[i][j].y_next != play.y_next){
							((pos_board**)input)[i][j] = clean;
							safe.x_next = j;
							safe.y_next = i;
							for (int i = 0; i < n_player; ++i) send(comms[i], &safe, sizeof (pos_board), 0);
						}
							
					}
				}

    		}
    		//DOESN'T MOVE
    		else{
    			play.x_next = play.x;
				play.y_next = play.y;

				for (int i = 0; i < n_player; ++i) send(comms[i], &play, sizeof (pos_board), 0);
    		}
    	}

    	pthread_mutex_unlock(&board_mutex);
	}
	return (NULL);
}

void * thirty_reset(void * input){	

	pos_board aux1;
	pos_board aux2;
	pos_board clean;
	clean.object = ' ';

	int px, py, start2, end2;
	int sock = comms[n_player-1]; 
	int k = total_p;
	int time2[500];
	int tfruit[500];
	int min_1, min_aux, total_points;

	int time1 = actual_time;

	int move = -1;

	while(!done[k]){ 

		pos_board pacman;
		pos_board monster;

		//EATEN FRUIT
		if (fruit[k]!=0){
			time2[end2] = actual_time+2;
			tfruit[end2] = fruit[k]; 
			fruit[k] = 0;
			end2++;
		} 
		//NEW FRUIT
		for (int i = start2; i < end2; ++i){
			if (actual_time == time2[i]){
				while (1){ 
					py=rand()%(dim[1]);
					px=rand()%(dim[0]);
					if(((pos_board**)input)[py][px].object == ' '){
						pthread_mutex_lock(&board_mutex);
						break;								
					} 
				}
				//EMPTY POS FOR NEW FRUIT
				aux1.x = px;
				aux1.y = py;
				if(tfruit[i]==1){
					aux1.object = 'L';
					((pos_board**)input)[py][px].object = 'L';
				} 
				else if(tfruit[i]==2){
					aux1.object = 'C';
					((pos_board**)input)[py][px].object = 'C';
				} 
				for (int i = 0; i < n_player; ++i) send(comms[i], &aux1, sizeof (pos_board), 0);
				pthread_mutex_unlock(&board_mutex);
				start2++;
			}
		}

		//1s 
		if (actual_time-time1 >= 1){
			time1 = actual_time;
			countp[k] = 0;
			countm[k] = 0;
		
			min_aux = (actual_time - begin_time)%60;
			min_1 = (actual_time+1 - begin_time)%60;

			//SEARCH BOARD TO UPDATE MONSTER AND PACMAN AFTER PLAY
			pthread_mutex_lock(&board_mutex);
			for (int i = 0; i < dim[1]; ++i){
				for (int j = 0; j < dim[0]; ++j){
					if (((pos_board**)input)[i][j].sock_id == sock){
						if (((pos_board**)input)[i][j].object=='M') monster = ((pos_board**)input)[i][j];
						if (((pos_board**)input)[i][j].object=='P' || ((pos_board**)input)[i][j].object=='S') pacman = ((pos_board**)input)[i][j];
					}	
				}
			}		
			pthread_mutex_unlock(&board_mutex);
			
			//59s 
			if(min_1 == 0){
				aux2.object = 'x';
				send(sock, &aux2, sizeof (pos_board), 0);
				
			}

			//1 min
			if (min_aux == 0){
				total_points = points[k];	

				aux2 = pacman;
				aux2.object = 'X';
				aux2.k = total_points;
				for (int i = 0; i < n_player; ++i) send(comms[i], &aux2, sizeof (pos_board), 0);
			}

			//PACMAN 30s REFRESH
			if (actual_time-pacman.time >= 30){
				pthread_mutex_lock(&board_mutex);
				aux1 = pacman;
				while (1){ 
					py=rand()%(dim[1]);
					px=rand()%(dim[0]);
					if(((pos_board**)input)[py][px].object == ' ') break;								
				}

				//NEW POS
				aux1.x_next = px;
				aux1.y_next = py;

				//OLD POS
				aux1.x = pacman.x_next;
				aux1.y = pacman.y_next;
				aux1.time = actual_time;

		    	((pos_board**)input)[pacman.y_next][pacman.x_next] = clean;
		    	((pos_board**)input)[aux1.y_next][aux1.x_next] = aux1;
		    	
		    	for (int i = 0; i < n_player; ++i) send(comms[i], &aux1, sizeof (pos_board), 0);
		  	    pthread_mutex_unlock(&board_mutex);
			}

			//MONSTER 30s REFRESH
			if (actual_time-monster.time >= 30){
				pthread_mutex_lock(&board_mutex);

				aux1 = monster;
				while (1){ 
					py=rand()%(dim[1]);
					px=rand()%(dim[0]);
					if(((pos_board**)input)[py][px].object == ' ') break;								
				}

				//NEW POS
				aux1.x_next = px;
				aux1.y_next = py;

				//OLD POS
				aux1.x = monster.x_next;
				aux1.y = monster.y_next;
				aux1.time = actual_time;

		    	((pos_board**)input)[monster.y_next][monster.x_next] = clean;
		    	((pos_board**)input)[aux1.y_next][aux1.x_next] = aux1;
		    	for (int i = 0; i < n_player; ++i) send(comms[i], &aux1, sizeof (pos_board), 0);
		    	pthread_mutex_unlock(&board_mutex);
		    }
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

	//BOARD DIMENSIONS 
	send(client_sock, &dim, sizeof(dim), 0);

	//NUMBER OF BRICKS
	send(client_sock, &n_bricks, sizeof(int), 0);

	int aux[2];

	//BRICK'S COORDINATES
	for (int i = 0; i < dim[1]; ++i)
	{
		for (int j = 0; j < dim[0]; ++j)
		{			
			if(new_board[i][j].object == 'B'){
				aux[0] = j; //columns
				aux[1] = i; //lines
				send(client_sock, &aux, sizeof(aux), 0);
			}		
		}		
	}
}

player_id * init_player (player_id * new_player, pos_board ** new_board, colour c_colour, int client_sock, pthread_t player_thread){

		new_player = malloc(sizeof(player_id));  

		new_player->player_n = n_player;
		new_player->thread_id = player_thread;	

		int c = dim[0]; 
		int l = dim[1];
		int aux_monster[2];
		int aux_pacman[2];


		//DEFINE PACMAN's FIRST POS
		while (1){ 
			aux_pacman[0]=rand()%(l);
			aux_pacman[1]=rand()%(c);
			if(new_board[aux_pacman[0]][aux_pacman[1]].object == ' ') break;
		}
		new_player->pos_pacman[0]=aux_pacman[0];
		new_player->pos_pacman[1]=aux_pacman[1];

		//DEFINE MONSTER's FIRST POS
		while(1){
			aux_monster[0]=rand()%(l);
			aux_monster[1]=rand()%(c);
			if( new_board[aux_monster[0]][aux_monster[1]].object == ' ' && ((aux_pacman[0]!=aux_monster[0]) && (aux_pacman[1]!=aux_monster[1])) )
				break;
		}	
		new_player->pos_monster[0]=aux_monster[0];
		new_player->pos_monster[1]=aux_monster[1];

		//Cor do Jogador
		new_player->rgb[0] = c_colour.r;
 		new_player->rgb[2] = c_colour.b;
		
		new_player->sock_id = client_sock;
		new_player->next = NULL;

		return(new_player);
}

void board_update (pos_board ** new_board, player_id  * new_player){

	//FIRST POSITION - Pacman
	new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]].object = 'P';
	new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]].sock_id = new_player-> sock_id;
	new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]].r = new_player-> rgb[0];
	new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]].g = new_player-> rgb[1];
	new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]].b = new_player-> rgb[2];
	new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]].x_next = new_player-> pos_pacman[1];
	new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]].y_next = new_player-> pos_pacman[0];

	new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]].x = -1;
	new_board[new_player -> pos_pacman[0]][new_player->pos_pacman[1]].y = -1;

	//FIRST POSITION - Monster
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

