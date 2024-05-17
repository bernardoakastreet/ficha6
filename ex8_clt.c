#define BUFFERSIZE 2000

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
  int sd, n;
  struct addrinfo hints, *a;
  char buffer[BUFFERSIZE];

  if (argc != 2) {
    printf("Utilização: %s server_ip_address\n", argv[0]);
    exit(1);
  }

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  getaddrinfo(argv[1], "3000", &hints, &a);

  sd = socket(AF_INET, SOCK_DGRAM, 0);

  printf("\nInsira o texto a enviar para o servidor:\n");

  fgets(buffer, BUFFERSIZE , stdin);
  //Removing \n
  buffer[(int)strlen(buffer) - 1] = '\0';

  printf("\nA enviar para o servidor...\n\n");
  sleep(2);
  if (sendto(sd, buffer, strlen(buffer), 0, a->ai_addr, a->ai_addrlen) == -1) {
    perror("Erro ao enviar socket\n");
    exit(1);
  }
  printf("Socket enviado para o servidor com sucesso!\n");
  return 0;
}

//Dos testes realizados, foi possível enviar 999 bytes de dados, ao que, após esse valor, o packet foi dividido e enviado o restante noutro packet