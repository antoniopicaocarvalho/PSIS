#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define Mem_Port 3000

//----------------------------SERVER----------------------------
int sock_fd;
struct sockaddr_in make_socket (int *sock_fd);
int Do_Bind(int sock_fd, struct sockaddr_in local_addr);
void Do_Listen(int sock_fd);
//----------------------------CLIENT----------------------------
int connect_server (int sock_fd, struct sockaddr_in server_addr);


  