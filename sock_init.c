#include "sock_init.h"

// ----------------<SERVER>---------------------------------------------------------------
// -----------------Socket----------------------------------------------------------------
  struct sockaddr_in make_socket (int *sock_fd){

    struct sockaddr_in addr;

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

    if(-1 == bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr))) {
     perror("bind");
     exit(-1);
    }
    return sock_fd;
  }
// -----------------Listen----------------------------------------------------------------
  void Do_Listen(int sock_fd){
    listen(sock_fd, 5);
  }


// -----------------ACCEPT----------------------------------------------------------------
  int Accept_client(int sock_fd, int *player){
    //-------------Player_n---------------------------------------------------------------
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
    if( -1 == connect(sock_fd,(const struct sockaddr *) &server_addr, sizeof(server_addr))){
        printf("Error connecting\n");
        exit(-1);}
    // indica que o cliente se conectou
    printf("Client connected\n");
    return sock_fd;
  }

