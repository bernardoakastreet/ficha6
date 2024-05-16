#define BUFFERSIZE 4096

#include <unistd.h> 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

typedef struct {
  char student_id[7];
  char text[2000]; //should be ‘\0’ terminated
} msg1_t;
typedef struct {
  char text[2000]; //should be ‘\0’ terminated
  char student_name[100]; //should be ‘\0’ terminated
} msg2_t;

void print_address(const struct sockaddr * clt_addr, socklen_t addrlen) 
{
  char hostname[256];  
  char port[6];

  int n = getnameinfo(clt_addr, addrlen, hostname, sizeof(hostname), 
    port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV); 
  if(n != 0)
    printf("%s\n", gai_strerror(n));
  else
    printf("Connection from %s:%s\n", hostname, port);
}

int my_create_server_socket(char *port)
{
  int s, r;
  struct addrinfo hints, *a;

  memset (&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_flags = AI_PASSIVE;
  r = getaddrinfo(NULL, port, &hints, &a);
  if (r != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
    exit(1);
  } 


  s = socket(PF_INET, SOCK_STREAM, 0);
  if(s == -1){
    perror("socket");
    exit(1);
  }


  //avoid bind errors if the port is still being used by previous connections
  int so_reuseaddr = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr));

  
  r = bind(s, a->ai_addr, a->ai_addrlen);
  if(r == -1) {
    perror("bind");
    exit(1);
  }    
  
  r = listen(s, 5); 
  if(r == -1) {
    perror("listen");
    exit(1);
  }   
  
  freeaddrinfo(a);
  return s;
}

int main(int argc, char *argv[]){
  int new_socket_descriptor, socket_descriptor; 
  int n, fdd;
  struct sockaddr clt_addr;
  socklen_t addrlen;

  msg1_t msg1;
  msg2_t msg2;

  char *student_name_temp = (char *)malloc(33 * sizeof(char));
  char student_name[32];
  const int student_name_size = 32;
  strcpy(student_name_temp, "Bernardo Augusto Silva Teixeira\n");
  memcpy(student_name, student_name_temp, student_name_size);
  free(student_name_temp);
  
  //SIGPIPE is sent to the process if a write is made to a closed connection.
  //By default, SIGPIPE terminates the process. This makes the process to ignore the signal.
  signal(SIGPIPE, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);

  if(argc!=2) {
    printf("Usage: %s port_number\n", argv[0]);
    exit(1); 
  }

  socket_descriptor = my_create_server_socket(argv[1]);

  while(1) {
    
    printf("Waiting connection\n");  
    addrlen = sizeof(clt_addr);

    new_socket_descriptor = accept(socket_descriptor, &clt_addr, &addrlen); 
    if(new_socket_descriptor < 0) {
      perror("accept");
      sleep(1);
      continue;
    }
    print_address(&clt_addr, addrlen);
    
    int f = fork();
    if(!f){
      printf("Handling client in child process no. %d\n", getpid());
      close(socket_descriptor);

      //Set struct for timeout interval
      struct timeval timeout;      
      timeout.tv_sec = 5;
      timeout.tv_usec = 0;

      //Close client socket if no data sent within 5s
      if (setsockopt(new_socket_descriptor, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0) perror("setsockopt failed\n");
      
      //Handle client
      size_t read_bytes = read(new_socket_descriptor, &msg1, sizeof(msg1_t));
      int text_size = (int)strlen(msg1.text);
      
      for(int i = 0; i < text_size; i++) msg1.text[i] = toupper(msg1.text[i]);
      //Convert message to upper case
      strcpy(msg2.text, msg1.text);
      //Fill msg2.student_name variable
      strcpy(msg2.student_name, "Bernardo Augusto Silva Teixeira");

      //Write to socket the msg2 struct all built
      write(new_socket_descriptor, &msg2, sizeof(msg2_t));

      close(new_socket_descriptor);
      return 0;
    }
  close(new_socket_descriptor);
  }
  return 0;
}