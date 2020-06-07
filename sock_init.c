#include "sock_init.h"

//----------------------------SERVER----------------------------
  struct sockaddr_in make_socket (int *sock_fd){

    struct sockaddr_in addr;

    *sock_fd= socket(AF_INET, SOCK_STREAM, 0);
    if (*sock_fd == -1){
      perror("socket: ");
      exit(-1);
     }
    addr.sin_family = AF_INET;
    addr.sin_port= htons(Mem_Port);
    return addr;
  }
//BIND
  int Do_Bind(int sock_fd , struct sockaddr_in local_addr ){

    if(-1 == bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr))) {
     perror("bind");
     exit(-1);
    }
    return sock_fd;
  }
//LISTEN
  void Do_Listen(int sock_fd){
    listen(sock_fd, 5);
  }



//----------------------------CLIENT----------------------------
//CONNECT
int connect_server (int sock_fd, struct sockaddr_in server_addr){
  if( -1 == connect(sock_fd,(const struct sockaddr *) &server_addr, sizeof(server_addr))){
      printf("Error connecting\n");
      exit(-1);}
  return sock_fd;
}

