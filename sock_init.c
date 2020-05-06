#include "sock_init.h"

/* -----------------Funcionamento de um server-------------------------------------------
    1º-> SOCKET => é criado o socket
    2º-> BIND   => é associado um nome ao socket criado atravez do bind()
    3º-> LISTEN => habilita que o server receba o connect do client
    4º-> ACCEPT => aceita a ligação que esta a receber do client.c
    5º-> ...
*/

/* -----------------Funcionamento de um Client-------------------------------------------
    1º-> SOCKET    => é criado o socket
    2º-> CONNECT   => é associado um nome ao socket criado atravez do bind()
    3º-> ... 
*/

// ----------------<SERVER>----------------------------------------------------------------
// -----------------Socket----------------------------------------------------------------
  struct sockaddr_in make_socket (int *sock_fd){
    // cria uma estruturas do tipo sockaddr_in (INET) [expandir] 
          /*
          struct sockaddr_in {
            short            sin_family;   // e.g. AF_INET      (endresso de familia)
            unsigned short   sin_port;     // e.g. htons(3490)  (host to network) 
                                          é definida uma variavel comum ao client e ao server 
                                          (em galo-....h) para saber qual é o 
            struct in_addr   sin_addr;     // see struct in_addr, below 
            char             sin_zero[8];  // zero this if you want to
          };
          struct in_addr {
            unsigned long s_addr;  // load with inet_aton() associa o ip do meu computador ao server e ao client 
          };
          */
    struct sockaddr_in addr;
    /* cria um socket (dominio, tipo do socket, protocolo)
      AF_INET-> two processes running on any two hosts on the Internet communicate. Each of these has its own address format
      SOCK_STREAM-> estabelece um contacto direto entre o server e o player 1, como se fosse um telefonema,(não ha perda de dados)
      0-> Estamos de acordo com os parametros prédefinidos*/
    *sock_fd= socket(AF_INET, SOCK_STREAM, 0);
    // a função socket retorna '-1' caso haja um erro, como tal damos o exit do programa
    if (*sock_fd == -1){
      perror("socket: ");
      exit(-1);
     }
    //Guarda no addr.sin_family o AF_INET (adresso de familia) 
    addr.sin_family = AF_INET;
    /*  Guarda no addr.sin_port o endereço da internet local (internet local)
       Memory_PORT é uma constante definida em "Socket_init.h"
         */
    addr.sin_port= htons(Mem_Port);
    return addr;
  }
// -----------------Bind------------------------------------------------------------------
  int Do_Bind(int sock_fd , struct sockaddr_in local_addr ){
    /* A função bind serve para ligar um nome a uma socket. 
      bind(Nome do Socket, endereço de uma estrutura de dados a qual reperesnta o nome a atribuir, tamanho do endereço);
        -> se funcionar, retorna 0
        -> se não funcionar, retorna -1. Entra no ciclo if e sai do programa
      Quando é que a função bind pode dar erro? (retornar '-1')
      Ao contrario do Client.c neste bind iremos usar o Server pois é a socket que aqui precisamos*/
    if(-1 == bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr))) {
     perror("bind");
     exit(-1);
    }
    // Informa que o socket foi creado e ligado a um nome 
    printf("\nsocket created and binded \n");  
    return sock_fd;
  }
// -----------------Listen----------------------------------------------------------------
  void Do_Listen(int sock_fd){
    /*Listen() habilita que o programa servidor receba conexões de um programa cliente.
      int listen(int socket, int backlog);
        socket => nome do socket
        backlog=> O backlog indica quantas conexões pendentes o socket pode deixar na fila 
                  para serem processadas, quando as conexões são aceitas elas são removidas
                  da fila. O mínimo para o backlog é 1 e o máximo 5*/
    listen(sock_fd, 5);
  }


// -----------------ACCEPT----------------------------------------------------------------
  int Accept_client(int sock_fd, int *player){
    //-------------Player_n-----------------------------------------------------------------------
      /* aceita a ligação que esta a receber do client.c
           int accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len);
            int socket                          -> o socket ao qual foi feito o listen
            struct sockaddr *restrict address   -> 'mete' o adress do client--------->*
            socklen_t *restrict address_len     -> tamanho do adress do client------->*
              * se o 2º e o 3º parametro forem NULL a comunicação pode ser estabelicida,
              * no entanto é impossivel saber a identidade do client
           retorna:(erro)    -1 em caso de erro
                  (sucesso) novo socket que ficará associado ao novo canal de comunicação */
    struct sockaddr_in client;

    socklen_t size = sizeof (struct sockaddr_in);

    if ((*player = accept(sock_fd, (struct sockaddr*)&client, &size))< 0){
      perror("accept:");
      return -1;
     }
    return 0;
  } 
// ---------------------------------------------------------------------------------------


// ----------------<Client>----------------------------------------------------------------
// -----------------Connect----------------------------------------------------------------
  int connect_server (int sock_fd, struct sockaddr_in server_addr){
    /*  Inicia a conecção num socket
        connect(int socket, const struct sockaddr *address, socklen_t address_len);
          socket                            ->  sock_fd
          const struct sockaddr *address    ->  (const struct sockaddr *) &server_addr
          socklen_t address_len             ->  sizeof(server_addr)
        Retorno: 0 se conectar, -1 se falhar a conecção (daí o if)*/
    if( -1 == connect(sock_fd,(const struct sockaddr *) &server_addr, sizeof(server_addr))){
        printf("Error connecting\n");
        exit(-1);}
    // indica que o cliente se conectou
    printf("Client connected\n");
    return sock_fd;
  }


/*

void p_color (int r, int g, int b, int **rgb){
  *rgb=malloc(3*sizeof(int));
  if(*rgb==NULL)
    return;
  (*rgb)[0]=r;
  (*rgb)[1]=g;
  (*rgb)[2]=b;
}

void player_color (int n, int *r, int *g, int *b){
  
  int *rgb;
  
  switch (n){
    case 0:{              //jogador 1 (Azul)
      p_color (0, 128, 255, &rgb);
      break;
    }
    case 1:{              //jogador 2 (Vermelho)
      p_color (255, 51, 51, &rgb);
      break;
    }
    case 2:{              //jogador 3 (Verde)
      p_color (124, 218, 89, &rgb);
      break;
    }
    case 3:{              //jogador 4 (Amarelo)
      p_color (255, 255, 51, &rgb);
      break;
    }
    case 4:{              //jogador 5 (Laranja)
      p_color (255, 153, 51, &rgb);
      break;
    }
    case 5:{              //jogador 6 (Rosa)
      p_color (255, 102, 178, &rgb);
      break;
    }
    case 6:{              //jogador 7 (Violeta)
      p_color (178, 102, 255, &rgb);
      break;
    }
    default :{              //jogador 8 ou mais 
      int rgb_code[3]={0,0,0};
      int random_n=0; 
      for(int i=0; i<3; i++){
        random_n=random()% 256;;  
        rgb_code[i]=random_n;
      }
      p_color (rgb_code[0], rgb_code[1], rgb_code[2], &rgb);
      break;
    }
  }

  *r=rgb[0];
  *g=rgb[1];
  *b=rgb[2];

  free(rgb);
}

play_response init_player (int player_num){
  
  play_response player;

  player.code=10;           //  Code do jogador
  player.first_play=-1;
  player.n=player_num;
  player.sock=-1;
  player_color(player_num, &player.rgb[0],&player.rgb[1],&player.rgb[2]);
  player.state=1;
  player.place=-1;
  player.n_pairs=0;
  return player;
}

general_info init_game (int dim){
  
  
  game.n_corrects[0]=0;     // Indica o número de cartas que já foram acertadas
  game.n_corrects[MaxPlayers]=0;  // Indica o número de cartas que já foram acertadas
  game.num_players=0;       // numero de jogadores de momento
  game.all_players=0;     // numero de todos os jogadores
  game.status=0;    
  game.dim_board=dim;
  for(int i=0;i<MaxPlayers;i++){
    game.active_players[i]=0;
  }       

  return game;
}

general_info new_player_in_game (int conection){
  
  game.players_fd[game.all_players]=conection;
  game.active_players[game.all_players]=1;
  game.num_players++;
  game.all_players++;
  printf("Player %d just connected to the game\n", game.all_players);
  if(game.num_players>=2){
    game.status=1;
  }
  return game;
}
*/