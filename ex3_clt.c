#define BUFFERSIZE 4096

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

int my_connect(char *servername, char *port) {

  //get server address using getaddrinfo
  struct addrinfo hints;
  struct addrinfo *addrs;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  int r = getaddrinfo(servername, port, &hints, &addrs);
  if (r != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
    exit(1);
  }
  
  //create socket
  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (s == -1) {
    perror("socket");  
    exit(2);
  }
  
  //connect to server
  r = connect(s, addrs->ai_addr, addrs->ai_addrlen);
  if (r == -1) {
    perror("connect");  
    close(s);
    exit(3);
  }

  freeaddrinfo(addrs);  
  return s;
}

void print_socket_address(int sd)
{
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);

  getsockname(sd, (struct sockaddr *) &addr, &addrlen);

  char hostname[256];  
  char port[6];

  int n = getnameinfo((struct sockaddr *) &addr, addrlen, hostname, sizeof(hostname), 
    port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV); 
  if(n != 0)
    printf("%s\n", gai_strerror(n));
  else
    printf("Socket address: %s:%s\n", hostname, port);  
}

int main (int argc, const char** argv) {
  
  if(argc!=3) {
    printf("Utilização: %s server_ip_address port_number\n",argv[0]);
    exit(1); 
  }

  char numero_estudante[8];
  numero_estudante[0] = '1';
  numero_estudante[1] = '2';
  numero_estudante[2] = '2';
  numero_estudante[3] = '1';
  numero_estudante[4] = '3';
  numero_estudante[5] = '8';
  numero_estudante[6] = '0';
  numero_estudante[7] = '\n';

  char *buffer_stdin = (char *)malloc(2000*sizeof(char));

  printf("Insira a mensagem a enviar:\n");
  
  fgets(buffer_stdin, 2000, stdin);
  int tamanho_mensagem = (int)strlen(buffer_stdin);
  //printf("%d\n", tamanho_mensagem); //mensagem tem ao total 13 caracteres, tamanho_mensagem=12
  if(!tamanho_mensagem) return 1;
  char *mensagem = (char *)malloc(tamanho_mensagem*sizeof(char));

  memcpy(mensagem, buffer_stdin, tamanho_mensagem);

  int tam_payload = 8 + tamanho_mensagem;
  char *payload = malloc(tam_payload*sizeof(char));
  memcpy(payload, numero_estudante, 8);

  free(buffer_stdin);

  for(int i = 0; i < tamanho_mensagem; i++) payload[i+8] = mensagem[i];

  printf("Connecting to server....\n");
  sleep(2);

  //Connect to server
  int socket_descriptor = my_connect(argv[1], argv[2]);

  //Write to socket descriptor
  write(socket_descriptor, payload, tam_payload);
  printf("Content sent to %s!\n\n", argv[1]);
  sleep(2);

  //Read response from server using FILE
  char *buffer = (char *)malloc(BUFFERSIZE*sizeof(char));
  size_t size_buffer = read(socket_descriptor, buffer, BUFFERSIZE);

  if(!(int)size_buffer){
    printf("Connection timeout with server, exiting....\n");
    exit(1);
  }

  char *message_recv = strtok(buffer, "\n");
  char *student_name_recv = strtok(NULL, "\n");

  printf("Mensagem: %s\n""Nome: %s\n""Total: %d\n", message_recv, student_name_recv, (int)size_buffer);

  close(socket_descriptor);
  return 0;
}