#ifndef FS_H
#define FS_H
#include "state.h"
#include <pthread.h>

void init_fs();
void destroy_fs();
int is_dir_empty(DirEntry *dirEntries);
int create(char *name, type nodeType);
int delete(char *name);
int lookup2(char *name);
int lookup2_deslockread(char *name);
void deslockar_vetor(int vetor[]);
void print_tecnicofs_tree(FILE *fp);
int lookup(char *name, int vetor[]);
int move(char *path, char *finalPath);

pthread_rwlock_t lock;

#endif /* FS_H */
