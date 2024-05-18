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

//This function will be used in ex5 to read the size of each string (student's name and text)
int read_bytes_from_string(char *string, int string_length) {
  int count;
  for (int i = 0; i < string_length; i++) {
    if (string[i] == '\n') {
      count = i;
      break;
    }
  }
  return count;
}

int main (int argc, const char** argv) {

  printf("1221380 - %s\n", __FILE__);
  
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
    printf("Were written %d characters into console, including the new line!\n", buffer_stdin_size);
  }
  
  //Removing \n
  sprintf(msg1.text, "%d\n%s", buffer_stdin_size, buffer_stdin);

  printf("Connecting to server....\n\n");
  sleep(2);

  //Connect to server
  int socket_descriptor = my_connect(argv[1], argv[2]);

  ///Write to socket descriptor
  write(socket_descriptor, &msg1, sizeof(msg1_t));

  //Read response, same variable type as it was sent
  char buffer1[2000], buffer2[1000];
  read(socket_descriptor, &msg2, sizeof(msg2_t));

  //For manipulation, store it in a temporary buffer
  strcpy(buffer1, msg2.text);
  strcpy(buffer2, msg2.student_name);

  //How many bytes represent the lenght of the text sent?
  int num = read_bytes_from_string(buffer1, strlen(buffer1));

  //Transform it into an integer
  char text_recv_size_c[4];
  memcpy(text_recv_size_c, buffer1, num);
  int text_recv_size = atoi(text_recv_size_c);

  //Copy the buffer portion for a pretty formatted variable
  char text_recv[2000];
  memcpy(text_recv, buffer1 + num + 1, text_recv_size);
  text_recv[text_recv_size] = '\0';

  //How many bytes represent the lenght of the student's name sent?
  num = read_bytes_from_string(buffer2, strlen(buffer2));

  //Transform it into an integer
  char student_name_recv_size_c[4];
  memcpy(student_name_recv_size_c, buffer2, num);
  int student_name_recv_size = atoi(student_name_recv_size_c);

  //Copy the buffer portion for a pretty formatted variable
  char student_name_recv[2000];
  memcpy(student_name_recv, buffer2 + num + 1, student_name_recv_size);
  student_name_recv[student_name_recv_size] = '\0';
  
  //Address it again to msg2 variable type, now only with the correct formatted string
  strcpy(msg2.student_name, student_name_recv);
  strcpy(msg2.text, text_recv);

  printf("Mensagem: %s\n""Nome: %s\n""Total: %d\n", msg2.text, msg2.student_name, (student_name_recv_size + text_recv_size));

  close(socket_descriptor);
  return 0;
}