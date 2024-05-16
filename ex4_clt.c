#define BUFFER_STDIN_SIZE 2000

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

typedef struct {
  char student_id[7];
  char text[2000]; //should be ‘\0’ terminated
} msg1_t;
typedef struct {
  char text[2000]; //should be ‘\0’ terminated
  char student_name[100]; //should be ‘\0’ terminated
} msg2_t;


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
  
  msg1_t msg1;
  msg2_t msg2;

  if(argc!=3) {
    printf("Utilização: %s server_ip_address port_number\n",argv[0]);
    exit(1); 
  }

  msg1.student_id[0] = '1';
  msg1.student_id[1] = '2';
  msg1.student_id[2] = '2';
  msg1.student_id[3] = '1';
  msg1.student_id[4] = '3';
  msg1.student_id[5] = '8';
  msg1.student_id[6] = '0';

  char *buffer_stdin = (char *)malloc(BUFFER_STDIN_SIZE*sizeof(char));

  printf("Insira a mensagem a enviar:\n");
  
  fgets(buffer_stdin, BUFFER_STDIN_SIZE, stdin);
  int buffer_stdin_size = (int)strlen(buffer_stdin);
  if(!buffer_stdin_size){
    printf("Nothing written on console, exiting...");
    exit(1);
  }else{
    printf("Were written %d characters into console, including the end of string!\n", buffer_stdin_size);
  }
  
  //Removing \n
  memcpy(msg1.text, buffer_stdin, buffer_stdin_size-1);
  msg1.text[buffer_stdin_size] = '\0';
  free(buffer_stdin);

  printf("Connecting to server....\n\n");
  sleep(2);

  //Connect to server
  int socket_descriptor = my_connect(argv[1], argv[2]);

  //Write to socket descriptor
  write(socket_descriptor, &msg1, sizeof(msg1_t));

  //Read response, if available
  size_t size_buffer = read(socket_descriptor, &msg2, sizeof(msg2_t));

  if(!(int)size_buffer){
    printf("Connection timeout with server, exiting....\n");
    exit(1);
  }else{
    printf("Receiving from server...\n");
    sleep(1);
    printf("Mensagem: %s\n""Nome: %s\n""Total: %ld\n", msg2.text, msg2.student_name, size_buffer);
  }

  close(socket_descriptor);
  return 0;
}