/* Joao Vasco 95611
Maria Almeida 95628 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include "fs/operations.h"
#include <sys/time.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <strings.h>
#include <sys/uio.h>
#include <sys/stat.h>


#define MAX_COMMANDS 10
#define MAX_INPUT_SIZE 100

int sockfd;
struct sockaddr_un server_addr, client_addr;
socklen_t servlen, clilen;


int numberThreads = 0;


/* variaveis globais */
pthread_mutex_t mutex;
pthread_rwlock_t  rwlock;



void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}



void* applyCommands(void* arg){
    int valor;
    FILE *file;


    while (1){

        char token;
        char name[MAX_INPUT_SIZE],type[MAX_INPUT_SIZE];


        struct sockaddr_un client_addr;
        char in_buffer[MAX_INPUT_SIZE];
        int c;

        clilen=sizeof(struct sockaddr_un);
        c = recvfrom(sockfd, in_buffer, sizeof(in_buffer)-1, 0, (struct sockaddr *)&client_addr, &clilen);
        if (c <= 0) continue;

        //Preventivo, caso o cliente nao tenha terminado a mensagem em '\0',
        in_buffer[c]='\0';



        int numTokens = sscanf(in_buffer, "%c %s %s", &token, name, type);

        if (numTokens < 2) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }



        int searchResult;
        switch (token) {
            case 'c':
                switch (type[0]) {
                    case 'f':

                        printf("Create file: %s\n", name);
                        valor = create(name, T_FILE);
                        sendto(sockfd, (int*) &valor, sizeof(valor)+1, 0, (struct sockaddr *)&client_addr, clilen);

                        break;

                    case 'd':

                        printf("Create directory: %s\n", name);
                        valor = create(name, T_DIRECTORY);
                        sendto(sockfd,(int*) & valor,sizeof(valor)+1, 0, (struct sockaddr *)&client_addr, clilen);


                        break;

                    default:

                        fprintf(stderr, "Error: invalid node type\n");
                        exit(EXIT_FAILURE);

                }
            case 'l':

                searchResult = lookup2(name);
                sendto(sockfd,(int*) & searchResult,sizeof(searchResult)+1, 0, (struct sockaddr *)&client_addr, clilen);

                if (searchResult >= 0)
                    printf("Search: %s found\n", name);
                else
                    printf("Search: %s not found\n", name);



             break;

             case'm':
             searchResult = 1;
             sendto(sockfd,(int*) & searchResult,sizeof(searchResult)+1, 0, (struct sockaddr *)&client_addr, clilen);

             if (searchResult >= 0)
                 printf("A nao execucao do move nao permite esta nao permitindo mover de %s para %s\n", name,type);
             else
                 printf("MOVE ERRO\n");

              break;

            case 'd':

                printf("Delete: %s\n", name);
                valor = delete(name);
                sendto(sockfd, (int*) &valor, sizeof(valor)+1, 0, (struct sockaddr *)&client_addr, clilen);
                break;


            case 'p':

                /* fazer wrlock na raiz dos inodes para que nao seja executada mais nenhuma tarefa enquanto 'p' se estiver a executar */
                lockando_operations_write(0);

                file=fopen(name,"w");

                if (file == NULL){
                    printf("File a Nulo\n" );
                    exit(EXIT_FAILURE);
                }

                print_tecnicofs_tree(file);

                /* o servidor envia '0' ao cliente se a funcao print_tecnicofs_tree() for bem sucedida */
                sendto(sockfd, 0, sizeof(0)+1, 0, (struct sockaddr *)&client_addr, clilen);

                fclose(file);

                deslockando_state(0);

                break;

            default: { /* error */
                fprintf(stderr, "Error: command to apply\n");
                break;
            }
        }

    }
    return NULL;
}


/* funcao auxiliar para verificar numero argumentos e
verificar se os argumentos nao sao NULL. Verificar tb
se argv[4] nao e diferente de rwlock mutex ou nosync */
void funcao_ver_argc(int argc, char* argv[]) {

    int t=0;


    if (argc!=3) {
      printf("Programa nao funciona. Nao tem argc == 3 \n");
      exit(EXIT_FAILURE);
    }

    while (t<3) {
      if (argv[t]==NULL) {
        printf("Programa nao funciona. Argv[%d] nao existe \n", t);
        exit(EXIT_FAILURE);
      }
      t++;
    }
}


/* funcao auxiliar para criar tarefas */
void fazedor_de_threads() {

    int i;

    pthread_t tid[numberThreads]/*, insere_comando*/;


    /* criacao da thread */
    for (i=0; i<numberThreads; i++) {
        if (pthread_create (&tid[i], NULL, applyCommands, NULL) != 0){
        printf("Erro ao criar tarefa.\n");
        exit(EXIT_FAILURE);
        }
    }



    for (i=0; i<numberThreads; i++) {
        if (pthread_join (tid[i], NULL) != 0) {
        printf("Erro ao esperar por tarefa.\n");
        exit(EXIT_FAILURE);
        }
    }
}


/* funcao usada para atribuir o path e a familia do socket */
int setSockAddrUn(char *path, struct sockaddr_un *addr) {

  if (addr == NULL)
    return 0;

  bzero((char *)addr, sizeof(struct sockaddr_un));
  addr->sun_family = AF_UNIX;
  strcpy(addr->sun_path, path);

  return SUN_LEN(addr);
}


int main(int argc, char* argv[]) {

    struct timeval inicio, fim;
    float time1, time2, time3, time4;


    /* inicializacao mutex e rwlock */
    pthread_mutex_init(&mutex, NULL);

    funcao_ver_argc(argc,argv);


    /* inicializacao/criacao do socket do servidor */
    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
    perror("server: can't open socket");
    exit(EXIT_FAILURE);
    }

    /* verificar se argv[2] (socket do servidor) nao existe, se existir, ficheiro com esse nome e' apagado */
    unlink(argv[2]);

    /* servlen e' o tamanho do endereco do socket do servidor. */
    servlen = setSockAddrUn (argv[2], &server_addr);


    /* associacao do nome sockfd ao socket criado em cima */
    if (bind(sockfd, (struct sockaddr *) &server_addr, servlen) < 0) {
    perror("server: bind error");
    exit(EXIT_FAILURE);
    }
    /* init filesystem */
    init_fs();


    /* transformar argv[3] em int */
    numberThreads=atoi(argv[1]);


    /* process input and print tree */
    gettimeofday(&inicio,NULL);
    fazedor_de_threads();



    /* release allocated memory */
    destroy_fs();

    gettimeofday(&fim,NULL);

    /* funcionamento do relogio */
    time1 = (fim.tv_usec - inicio.tv_usec) ;
    time2 = time1/1000000;
    time3 = (fim.tv_sec - inicio.tv_sec) ;
    time4=time3+time2;
    printf("TecnicoFS completed in [%.4f] seconds.\n",time4 );

    close(sockfd);
    unlink(argv[2]);

    exit(EXIT_SUCCESS);
}
