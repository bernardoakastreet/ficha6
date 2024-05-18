#define BUFFERSIZE 4096
#define STUDENT_NAME_SIZE 33

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

int myReadLine1(int s, char *buf, int count) {
  int r, n = 0;
  if (count <= 0)
    return 0;
  else if (count == 1) {
    buf[0] = 0;
    return 0;
  } else
    --count; //leave space for '\0'
  do {
    r = read(s, buf + n, 1);
    if (r == 1)
      ++n;
  } while ( r == 1 && n < count && buf[n - 1] != '\n');

  buf[n] = '\0';

  return n;
}

int myReadBlock(int s, void *buf, int count)
{
  int r, nread = 0;

  while( nread < count ) {
    r = read(s, buf+nread, count-nread);
    if(r <= 0)
      break;
    else
      nread += r;
  }
  
  return nread;  
}

int main(int argc, char *argv[]){

  printf("1221380 - %s\n", __FILE__);
  
  int new_socket_descriptor, socket_descriptor; 
  int n, fdd;
  struct sockaddr clt_addr;
  socklen_t addrlen;


  char student_name[STUDENT_NAME_SIZE];
  strcpy(student_name, "Bernardo Augusto Silva Teixeira\n");
  
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
      char *buffer_first_string_temp = (char *)malloc(2000*sizeof(char));

      //Read student id and message size from sd
      int first_string_read_size = myReadLine1(new_socket_descriptor, buffer_first_string_temp, 20);

      //Alocate space for message size value, not expecting more than 4 digit (<9999)
      char *message_size_c = (char*)malloc(4*sizeof(char));
      
      //Separate message size from student id
      memcpy(message_size_c, buffer_first_string_temp+7, first_string_read_size-8);
      //Buffer not needed
      free(buffer_first_string_temp);

      //Convert to numeric
      int message_size = atoi(message_size_c);

      //Alocate space for message
      char *message_recv = (char *)malloc(2000*sizeof(char));
      
      //Read message from bytes given
      myReadBlock(new_socket_descriptor, message_recv, message_size);
      message_recv[message_size] = 0;
      printf("Received the following message with %d bytes: ", message_size);
      for(int i = 0; i < message_size; i++) printf("%c", message_recv[i]);

       //Convert message to upper case
      for(int i = 0; i < message_size; i++) message_recv[i] = toupper(message_recv[i]);

      //Fill msg2 struct with data
      msg2_t msg2;
      for(int i = 0; i < 2000; i++) msg2.text[i] = '\0';

      char mensagem[1000], mensagem1[1000], mensagem2[1000];
      int n = 32;
      sprintf(mensagem1, "%d\n%s", message_size, message_recv);
      sprintf(mensagem2, "%d\n%s", n, student_name);
      
      strcpy(msg2.student_name, mensagem2);
      strcpy(msg2.text, mensagem1);

      //Write to socket the msg2 struct all built
      write(new_socket_descriptor, &msg2, sizeof(msg2_t));

      //Getting ready to close client process
      free(message_recv);
      close(new_socket_descriptor);
      return 0;
    }
  close(new_socket_descriptor);
  }
  return 0;
}