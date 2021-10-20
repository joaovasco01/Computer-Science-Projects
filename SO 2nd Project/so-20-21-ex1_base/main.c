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

#define MAX_COMMANDS 10
#define MAX_INPUT_SIZE 100


void testar_lock(pthread_mutex_t *mutex);
void testar_cond(pthread_cond_t *cond, pthread_mutex_t *mutex);
void testar_signal(pthread_cond_t *cond);


int numberThreads = 0;
int ACABOU = 0;

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int tailQueue = 0;
int headQueue = 0;
int conta = 0;



/* variaveis globais */
pthread_mutex_t mutex;
pthread_rwlock_t  rwlock;
pthread_cond_t inserir_comando;
pthread_cond_t remove_comando;



int insertCommand(char* data) {
    if(pthread_mutex_lock(&mutex) != 0){
      printf("Erro a dar lock\n");
        exit(EXIT_FAILURE);
    }


    while(conta == MAX_COMMANDS){
        if(pthread_cond_wait(&inserir_comando, &mutex) != 0){
          printf("Erro a dar wait\n");
            exit(EXIT_FAILURE);
        }
    }

    strcpy(inputCommands[headQueue++], data);



    if(headQueue == MAX_COMMANDS){
        headQueue = 0;
    }
    conta++;
    if(pthread_cond_signal(&remove_comando) != 0){
      printf("Erro a dar signal\n");
        exit(EXIT_FAILURE);
    }

    if(pthread_mutex_unlock(&mutex) != 0){
      printf("Erro a dar unlock\n");
        exit(EXIT_FAILURE);
    }

    return 1;

}

int removeCommand(char * command) {


    if(pthread_mutex_lock(&mutex) != 0){
      printf("Erro a dar lock\n");
        exit(EXIT_FAILURE);
    }

    while(conta == 0){

        if(pthread_cond_wait(&remove_comando, &mutex) != 0){
          printf("Erro a dar wait\n");
            exit(EXIT_FAILURE);
        }

    }


    strcpy(command, inputCommands[tailQueue]);


    if (strcmp(command, "e") == 0){

        if(pthread_cond_broadcast(&remove_comando) != 0){
          printf("Erro a dar broadcast\n");
            exit(EXIT_FAILURE);
        }

        if(pthread_mutex_unlock(&mutex) != 0){
          printf("Erro a dar unlock\n");
         exit(EXIT_FAILURE);
        }
        return -1;
    }


    tailQueue++;
    if(tailQueue == MAX_COMMANDS){
        tailQueue = 0;
    }
    conta--;



    if(pthread_cond_signal(&inserir_comando) != 0){
        printf("Erro a dar signal\n");
        exit(EXIT_FAILURE);
    }
    if(pthread_mutex_unlock(&mutex) != 0){
        printf("Erro a dar unlock\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}





void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}

void * processInput(void *arg){
    char line[MAX_INPUT_SIZE];



    FILE *file = (FILE*) arg;
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
                return NULL;

            case 'l':

                if(numTokens != 2)

                    errorParse();

                if(insertCommand(line)){
                    break;
                }

                return NULL;

            case 'd':
                if(numTokens != 2)
                    errorParse();
                if(insertCommand(line))
                    break;
                return NULL;

            case 'm':
                if(numTokens != 3){
                    errorParse();
                }
                if(insertCommand(line))
                    break;
                return NULL;

            case '#':
                break;

            default: { /* error */
                errorParse();
            }
        }

    }
    /*adicionar o comando e ao buffer visto que chegou ao EOF*/

    insertCommand("e");
    return NULL;
}



void* applyCommands(void* arg){

    while (1){

        char command[MAX_INPUT_SIZE];

        if(removeCommand(command) == -1){

            return NULL;
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

                        printf("Create file: %s\n", name);
                        create(name, T_FILE);

                        break;


                    case 'd':

                        printf("Create directory: %s\n", name);
                        create(name, T_DIRECTORY);

                        break;

                    default:

                        fprintf(stderr, "Error: invalid node type\n");
                        exit(EXIT_FAILURE);

                }
                break;
            case 'l':
                searchResult = lookup2(name);

                if (searchResult >= 0)
                    printf("Search: %s found\n", name);
                else
                    printf("Search: %s not found\n", name);

                break;


            case 'd':


                printf("Delete: %s\n", name);
                delete(name);

                break;

            /*case 'e':
                insertCommand("e");
                break;*/

            /*case 'm':
                printf("Moving %s to %s", path, finalPath);
                move(path, finalPath);
                break;*/


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


    if (argc!=4) {
      printf("Programa nao funciona. Nao tem argc== 4 \n");
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
void fazedor_de_threads(FILE * file) {

    int i;

    pthread_t tid[numberThreads], insere_comando;


    if (pthread_create (&insere_comando, NULL, processInput, (void*)file) != 0){
        printf("Erro ao criar tarefa.\n");
        exit(EXIT_FAILURE);
    }


    /* criacao da thread */
    for (i=0; i<numberThreads; i++) {
        if (pthread_create (&tid[i], NULL, applyCommands, NULL) != 0){
        printf("Erro ao criar tarefa.\n");
        exit(EXIT_FAILURE);
        }
    }

    if (pthread_join (insere_comando, NULL) != 0) {
        printf("Erro ao esperar por tarefa.\n");
        exit(EXIT_FAILURE);
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

    pthread_cond_init(&inserir_comando, NULL);

    pthread_cond_init(&remove_comando, NULL);

    funcao_ver_argc(argc,argv);


    /* init filesystem */
    init_fs();


    /* transformar argv[3] em int */
    numberThreads=atoi(argv[3]);

    /* ler ficheiro input */
    file=fopen(argv[1],"r");


    if (file == NULL){
      printf("File a Nulo\n" );
      exit(EXIT_FAILURE); }



    /* process input and print tree */
    gettimeofday(&inicio,NULL);
    fazedor_de_threads(file);

    fclose(file);




    /* iniciar escrita no ficheiro output*/
    file=fopen(argv[2],"w");


    if (file == NULL){
      printf("File a Nulo\n" );
      exit(EXIT_FAILURE); }






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
