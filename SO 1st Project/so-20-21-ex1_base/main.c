/* Joao Vasco 95611
Maria Almeida 95628 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include "fs/operations.h"
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100

int numberThreads = 0;


char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

/* variaveis globais */
pthread_mutex_t mutex;
pthread_rwlock_t  rwlock;



int insertCommand(char* data) {
    if(numberCommands != MAX_COMMANDS) {
        strcpy(inputCommands[numberCommands++], data);
        return 1;
    }
    return 0;
}

char* removeCommand() {
    if(numberCommands > 0){
        numberCommands--;
        return inputCommands[headQueue++];
    }
    return NULL;
}

void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}

void processInput(FILE *file){
    char line[MAX_INPUT_SIZE];

    /* break loop with ^Z or ^D */
    while (fgets(line, sizeof(line)/sizeof(char), file)) {

        char token, type;
        char name[MAX_INPUT_SIZE];

        int numTokens = sscanf(line, "%c %s %c", &token, name, &type);


        /* perform minimal validation */
        if (numTokens < 1) {
            continue;
        }
        switch (token) {
            case 'c':
                if(numTokens != 3)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;

            case 'l':
                if(numTokens != 2)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;

            case 'd':
                if(numTokens != 2)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;

            case '#':
                break;

            default: { /* error */
                errorParse();
            }
        }

    }

}


/* funcao de lock (write) */
void lockando_wrlock(char* mode) {
  if(!strcmp(mode, "mutex")){ pthread_mutex_lock(&mutex);}
  else if(!strcmp(mode, "rwlock")){ pthread_rwlock_wrlock(&rwlock);}
}

/* funcao de lock (read) */
void lockando_rdlock(char* mode) {
  if(!strcmp(mode, "mutex")){ pthread_mutex_lock(&mutex);}

  else if(!strcmp(mode, "rwlock")){ pthread_rwlock_rdlock(&rwlock);}
}

/* funcao unlock */
void deslockando_wrlock(char* mode) {
  if(!strcmp(mode, "mutex")){ pthread_mutex_unlock(&mutex);}

  else if(!strcmp(mode, "rwlock")){ pthread_rwlock_unlock(&rwlock);}
}





void* applyCommands(void* arg){
    while (1){

        pthread_mutex_lock(&mutex);

        if (numberCommands<=0) {

            /* dar unlock do mutex porque numberCommands <= 0 */
            pthread_mutex_unlock(&mutex);

            return NULL;
        }

        char* mode = (char*) arg;

        const char* command = removeCommand();

        pthread_mutex_unlock(&mutex);

        if (command == NULL){
            continue;
        }


        char token, type;
        char name[MAX_INPUT_SIZE];

        int numTokens = sscanf(command, "%c %s %c", &token, name, &type);
        if (numTokens < 2) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }




        int searchResult;
        switch (token) {
            case 'c':
                switch (type) {
                    case 'f':

                        lockando_wrlock(mode);

                        printf("Create file: %s\n", name);
                        create(name, T_FILE);

                        deslockando_wrlock(mode);

                          break;


                    case 'd':

                        lockando_wrlock(mode);


                        printf("Create directory: %s\n", name);
                        create(name, T_DIRECTORY);


                        deslockando_wrlock(mode);

                        break;

                    default:

                        fprintf(stderr, "Error: invalid node type\n");
                        exit(EXIT_FAILURE);

                }
                break;
            case 'l':
                lockando_rdlock(mode);

                searchResult = lookup(name);

                if (searchResult >= 0)
                    printf("Search: %s found\n", name);
                else
                    printf("Search: %s not found\n", name);


                deslockando_wrlock(mode);

                break;


            case 'd':

                lockando_wrlock(mode);

                printf("Delete: %s\n", name);

                delete(name);

                deslockando_wrlock(mode);

                  break;


            default: { /* error */
                fprintf(stderr, "Error: command to apply\n");
                exit(EXIT_FAILURE);
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

    if ((strcmp(argv[4],"rwlock")!=0) && (strcmp(argv[4],"mutex")!=0) && (strcmp(argv[4],"nosync")!=0)){
      printf("Ultimo argumento != de rwlock, mutex ou nosync.\n" );
      exit(EXIT_FAILURE);
    }

    if (argc!=5) {
      printf("Programa nao funciona. Nao tem argc== 5 \n");
      exit(EXIT_FAILURE);
    }

    while (t<5) {
      if (argv[t]==NULL) {
        printf("Programa nao funciona. Argv[%d] nao existe \n", t);
        exit(EXIT_FAILURE);
      }
      t++;
    }
}


/* funcao auxiliar para criar tarefas */
void fazedor_de_threads(char* argv4) {

    int i;

    pthread_t tid[numberThreads];


    /* verificar input se 'nosync' corresponde a numberThreads == 1 */
    if(numberThreads != 1 && !strcmp(argv4, "nosync")){
            fprintf(stderr, "Erro: sem sincronizacao. numThreads != 1\n");
            exit(EXIT_FAILURE);
    }

    /* criacao da thread */
    for (i=0; i<numberThreads; i++) {
        if (pthread_create (&tid[i], NULL, applyCommands, (void*)argv4) != 0){
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

int main(int argc, char* argv[]) {

    FILE *file;
    struct timeval inicio, fim;
    float time1, time2, time3, time4;


    /* inicializacao mutex e rwlock */
    pthread_mutex_init(&mutex, NULL);
    pthread_rwlock_init(&rwlock, NULL);


    funcao_ver_argc(argc,argv);

    /* init filesystem */
    init_fs();


    /* ler ficheiro input */
    file=fopen(argv[1],"r");

    if (file == NULL){
      printf("File a Nulo\n" );
      exit(EXIT_FAILURE); }

    /* process input and print tree */
    processInput(file);
    fclose(file);



    gettimeofday(&inicio,NULL);


    /* iniciar escrita no ficheiro output*/
    file=fopen(argv[2],"w");

    if (file == NULL){
      printf("File a Nulo\n" );
      exit(EXIT_FAILURE); }


    /* transformar argv[3] em int */
    numberThreads=atoi(argv[3]);

    fazedor_de_threads(argv[4]);

    print_tecnicofs_tree(file);

    fclose(file);

    /* release allocated memory */
    destroy_fs();

    gettimeofday(&fim,NULL);

    /* funcionamento do relogio */
    time1 = (fim.tv_usec - inicio.tv_usec) ;
    time2 = time1/1000000;
    time3 = (fim.tv_sec - inicio.tv_sec) ;
    time4=time3+time2;
    printf("TecnicoFS completed in [%.4f] seconds.\n",time4 );


    exit(EXIT_SUCCESS);
}
