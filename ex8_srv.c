#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
  int sd, n;
  struct addrinfo hints, *a;
  char buffer[1000];
  struct sockaddr_in src;
  socklen_t src_len;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  getaddrinfo(NULL, "3000", &hints, &a);

  sd = socket(AF_INET, SOCK_DGRAM, 0);
  bind(sd, a->ai_addr, a->ai_addrlen);

  while (1) {
    int src_lenght = (int)sizeof(src);
    n = recvfrom(sd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&src, (socklen_t *)&src_lenght);
    buffer[n] = '\0';
    int client_port = ntohs(src.sin_port);
    char *client_address = inet_ntoa(src.sin_addr);
    printf("Received message from %s, port %d: %s\n", client_address, client_port, buffer);
  }
}