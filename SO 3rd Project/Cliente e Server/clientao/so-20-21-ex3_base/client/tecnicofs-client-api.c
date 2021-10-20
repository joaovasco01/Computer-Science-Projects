
#include "tecnicofs-client-api.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <sys/stat.h>

#define MAX_INPUT_SIZE 100
char template[]= "/tmp/fileXXXXXX"; /* ficheiro temporario atribuido a cada cliente */
int sockfd;
struct sockaddr_un client_addr, serv_addr;
socklen_t clilen, servlen;

int setSockAddrUn(char *path, struct sockaddr_un *addr) {

  if (addr == NULL)
    return 0;

  bzero((char *)addr, sizeof(struct sockaddr_un));
  addr->sun_family = AF_UNIX;
  strcpy(addr->sun_path, path);

  return SUN_LEN(addr);
}



int tfsCreate(char *filename, char nodeType) {

  char buffer[MAX_INPUT_SIZE];
  int i,c;

    /* juntar novamente linha input para a funcao applyCommands do main.c a poder processar */
    c = sprintf(buffer,"c %s %c",filename, nodeType);


    /*Comunicacoes com o servidor*/
    if(sendto(sockfd, buffer , c + 1, 0, (struct sockaddr *) &serv_addr, servlen) <0){
      perror("client: sendto error \n");
      return -1;
    }

    if(recvfrom(sockfd,(int*) &i, sizeof(i), 0, 0, 0) < 0){
      if(i == -1){
        return -1;
      }
    }

  return 0;
}

int tfsDelete(char *path) {

  char buffer[MAX_INPUT_SIZE] = "d ";

  /* juntar novamente linha input para a funcao applyCommands do main.c a poder processar */
  strcat(buffer, path);
  int i;

/*Comunicacoes com o servidor*/
  if(sendto(sockfd, buffer , sizeof(buffer) + 1, 0, (struct sockaddr *) &serv_addr, servlen) <0){
    perror("client: sendto error \n");
    return -1;
    }
  if(recvfrom(sockfd, (int*) &i, sizeof(i), 0, 0, 0) < 0){
    if(i == -1){
      return -1;
    }
  }
  return 0;
}

int tfsMove(char *from, char *to) {

  char buffer[MAX_INPUT_SIZE];
  int i,c;

    /* juntar novamente linha input para a funcao applyCommands do main.c a poder processar */
    c = sprintf(buffer,"m %s %s",from, to);


    /*Comunicacoes com o servidor*/
    if(sendto(sockfd, buffer , c + 1, 0, (struct sockaddr *) &serv_addr, servlen) <0){
      perror("client: sendto error \n");
      return -1;
    }

    if(recvfrom(sockfd,(int*) &i, sizeof(i), 0, 0, 0) < 0){
      if(i == -1){
        return -1;
      }
    }

  return 0;
}



int tfsLookup(char *path) {

  char buffer[MAX_INPUT_SIZE] = "l ";

  /* juntar novamente linha input para a funcao applyCommands do main.c a poder processar */
  strcat(buffer, path);
  int i;
/*Comunicacoes com o servidor*/
  if(sendto(sockfd, buffer , sizeof(buffer) + 1, 0,(struct sockaddr *) &serv_addr, servlen) <0){
    perror("client: sendto error \n");
    return -1;
  }
  if(recvfrom(sockfd,(int*) &i, sizeof(i), 0, 0, 0) < 0){
    if(i == -1){
      return -1;
    }
  }
  return 0;
}

int tfsPrint(char *File){
  char buffer[MAX_INPUT_SIZE]="p ";

  /* juntar novamente linha input para a funcao applyCommands do main.c a poder processar */
  strcat(buffer, File);
  int i;
/*Comunicacoes com o servidor*/
  if(sendto(sockfd, buffer , sizeof(buffer) + 1, 0,(struct sockaddr *) &serv_addr, servlen) <0){
    perror("client: sendto error \n");
    return -1;
  }
  if(recvfrom(sockfd,(int*) &i, sizeof(i), 0, 0, 0) < 0){
    if(i != 0){
      return -1;
    }
  }
  return 0;
}



int tfsMount(char * sockPath) {


  /* inicializacao/criacao do socket do cliente */
  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0) ) < 0) {
    perror("client: can't open socket");
    exit(EXIT_FAILURE);
  }

  /* atribuicao de um endereco (diferente) temporario a cada cliente */
  mkstemp(template);

  /* verificar se 'template' nao existe, se existir, ficheiro com aquele nome e' apagado */
  unlink(template);

  /* clilen e' o tamanho do endereco do socket do cliente */
  clilen = setSockAddrUn(template, &client_addr);

  /* associacao do nome sockfd ao socket criado em cima */
  if (bind(sockfd, (struct sockaddr *) &client_addr, clilen) < 0) {
    perror("client: bind error");
    exit(EXIT_FAILURE);
  }

  /*servlen e' o tamanho do endereco do socket do servidor */
  servlen = setSockAddrUn(sockPath, &serv_addr);
  return 0;
}



int tfsUnmount() {

  /* termina o socket do cliente */
  close(sockfd);

  /* apaga o ficheiro do cliente */
  unlink(template);

  return -1;
}
