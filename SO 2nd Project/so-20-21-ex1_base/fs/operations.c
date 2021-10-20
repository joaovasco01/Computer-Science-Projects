#include "operations.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

/* Given a path, fills pointers with strings for the parent path and child
 * file name
 * Input:
 *  - path: the path to split. ATENTION: the function may alter this parameter
 *  - parent: reference to a char*, to store parent path
 *  - child: reference to a char*, to store child file name
 */
void split_parent_child_from_path(char * path, char ** parent, char ** child) {

	int n_slashes = 0, last_slash_location = 0;
	int len = strlen(path);

	// deal with trailing slash ( a/x vs a/x/ )
	if (path[len-1] == '/') {
		path[len-1] = '\0';
	}

	for (int i=0; i < len; ++i) {
		if (path[i] == '/' && path[i+1] != '\0') {
			last_slash_location = i;
			n_slashes++;
		}
	}

	if (n_slashes == 0) { // root directory
		*parent = "";
		*child = path;
		return;
	}

	path[last_slash_location] = '\0';
	*parent = path;
	*child = path + last_slash_location + 1;

}


/*
 * Initializes tecnicofs and creates root node.
 */
void init_fs() {
	inode_table_init();
	/* create root inode */
	int root = inode_create(T_DIRECTORY);
	if (root != FS_ROOT) {
		printf("failed to create node for tecnicofs root\n");
		exit(EXIT_FAILURE);
	}
}


/*
 * Destroy tecnicofs and inode table.
 */
void destroy_fs() {
	inode_table_destroy();
}


/*
 * Checks if content of directory is not empty.
 * Input:
 *  - entries: entries of directory
 * Returns: SUCCESS or FAIL
 */

int is_dir_empty(DirEntry *dirEntries) {
	if (dirEntries == NULL) {
		return FAIL;
	}
	for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
		if (dirEntries[i].inumber != FREE_INODE) {
			return FAIL;
		}
	}
	return SUCCESS;
}


/*
 * Looks for node in directory entry from name.
 * Input:
 *  - name: path of node
 *  - entries: entries of directory
 * Returns:
 *  - inumber: found node's inumber
 *  - FAIL: if not found
 */
int lookup_sub_node(char *name, DirEntry *entries) {
	if (entries == NULL) {
		return FAIL;
	}
	for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (entries[i].inumber != FREE_INODE && strcmp(entries[i].name, name) == 0) {
            return entries[i].inumber;
        }
    }
	return FAIL;
}


/*
 * Creates a new node given a path.
 * Input:
 *  - name: path of node
 *  - nodeType: type of node
 * Returns: SUCCESS or FAIL
 */
int create(char *name, type nodeType){

	int parent_inumber, child_inumber;
	char *parent_name, *child_name, name_copy[MAX_FILE_NAME];
	/* use for copy */
	type pType;
	union Data pdata;


	int vetorzao[50];

	strcpy(name_copy, name);
	split_parent_child_from_path(name_copy, &parent_name, &child_name);

	parent_inumber = lookup(parent_name, vetorzao);

	if (parent_inumber == FAIL) {
		printf("failed to create %s, invalid parent dir %s\n",
		        name, parent_name);
		deslockar_vetor(vetorzao);
		return FAIL;
	}

	inode_get(parent_inumber, &pType, &pdata);

	if(pType != T_DIRECTORY) {
		printf("failed to create %s, parent %s is not a dir\n",
		        name, parent_name);

						deslockar_vetor(vetorzao);
		return FAIL;
	}

	if (lookup_sub_node(child_name, pdata.dirEntries) != FAIL) {
		printf("failed to create %s, already exists in dir %s\n",
		       child_name, parent_name);
		deslockar_vetor(vetorzao);
		return FAIL;
	}

	/* create node and add entry to folder that contains new node */
	child_inumber = inode_create(nodeType);
	if (child_inumber == FAIL) {
		printf("failed to create %s in  %s, couldn't allocate inode\n",
		        child_name, parent_name);
		deslockar_vetor(vetorzao);
		return FAIL;
	}

	if (dir_add_entry(parent_inumber, child_inumber, child_name) == FAIL) {
		printf("could not add entry %s in dir %s\n",
		       child_name, parent_name);
		deslockar_vetor(vetorzao);
		return FAIL;
	}
	deslockar_vetor(vetorzao);

	return SUCCESS;
}


/*
 * Deletes a node given a path.
 * Input:
 *  - name: path of node
 * Returns: SUCCESS or FAIL
 */
int delete(char *name){

	int parent_inumber, child_inumber;
	char *parent_name, *child_name, name_copy[MAX_FILE_NAME];
	/* use for copy */
	type pType, cType;
	union Data pdata, cdata;

	int vetorzao[50];



	strcpy(name_copy, name);
	split_parent_child_from_path(name_copy, &parent_name, &child_name);

	parent_inumber = lookup(parent_name, vetorzao);

	if (parent_inumber == FAIL) {
		printf("failed to delete %s, invalid parent dir %s\n",
		        child_name, parent_name);
		deslockar_vetor(vetorzao);
		return FAIL;
	}

	inode_get(parent_inumber, &pType, &pdata);

	if(pType != T_DIRECTORY) {
		printf("failed to delete %s, parent %s is not a dir\n",
		        child_name, parent_name);
			deslockar_vetor(vetorzao);
		return FAIL;
	}

	child_inumber = lookup_sub_node(child_name, pdata.dirEntries);

	if (child_inumber == FAIL) {
		printf("could not delete %s, does not exist in dir %s\n",
		       name, parent_name);
		deslockar_vetor(vetorzao);
		return FAIL;
	}

	inode_get(child_inumber, &cType, &cdata);

	if (cType == T_DIRECTORY && is_dir_empty(cdata.dirEntries) == FAIL) {
		printf("could not delete %s: is a directory and not empty\n",
		       name);
		deslockar_vetor(vetorzao);
		return FAIL;
	}

	/* remove entry from folder that contained deleted node */
	if (dir_reset_entry(parent_inumber, child_inumber) == FAIL) {
		printf("failed to delete %s from dir %s\n",
		       child_name, parent_name);
		deslockar_vetor(vetorzao);
		return FAIL;
	}

	if (inode_delete(child_inumber) == FAIL) {
		printf("could not delete inode number %d from dir %s\n",
		       child_inumber, parent_name);
		deslockar_vetor(vetorzao);
		return FAIL;
	}
	deslockar_vetor(vetorzao);

	return SUCCESS;
}


/*Lookup (novo) que permite fazer os locks a medida q se passa pelo path*/
int lookup(char *name, int vetorzao[]) {
	char full_path[MAX_FILE_NAME];
	char delim[] = "/";

	int x = 0;


	strcpy(full_path, name);

	/* start at root node */
	int current_inumber = FS_ROOT;

	/* use for copy */
	type nType;
	union Data data;

	/* get root inode data */
	inode_get(current_inumber, &nType, &data);

	char *path = strtok(full_path, delim);

	/* search for all sub nodes */
	while (path != NULL && (current_inumber = lookup_sub_node(path, data.dirEntries)) != FAIL) {
		inode_get(current_inumber, &nType, &data);
		path = strtok(NULL, delim);
		if(path == NULL){
			lockando_operations_write(current_inumber);
		}
		else{
			lockando_operations_read(current_inumber);
		}
		vetorzao[x] = current_inumber;
		x++;
	}
	vetorzao[x]=-1;

	return current_inumber;
}

/*
 * Lookup for a given path.
 * Input:
 *  - name: path of node
 * Returns:
 *  inumber: identifier of the i-node, if found
 *     FAIL: otherwise
 */

 /*Funcao usada para dar lookup quando o main chama o lookup*/
int lookup2(char *name) {

	char full_path[MAX_FILE_NAME];
	char delim[] = "/";
	int vetoread[50];
	int x=0;

	strcpy(full_path, name);

	/* start at root node */
	int current_inumber = FS_ROOT;

	/* use for copy */
	type nType;
	union Data data;

	/* get root inode data */
	inode_get(current_inumber, &nType, &data);

	char *path = strtok(full_path, delim);

	/* search for all sub nodes */
	/*dar lock nos nodes em que passamos so para read porque esta funcao so e chamnada no main para dar lookup*/
	while (path != NULL && (current_inumber = lookup_sub_node(path, data.dirEntries)) != FAIL) {
		inode_get(current_inumber, &nType, &data);
		path = strtok(NULL, delim);
		lockando_operations_read(current_inumber);
		vetoread[x] = current_inumber;
		x++;
	}
	x--;
	while (x>=0) {
		deslockando_state(vetoread[x]);
		x--;
	}

	return current_inumber;
}



int move(char *path, char *finalPath){
	int parent_inumber, child_inumber, child_inumber2;
	char *parent_name, *child_name, name_copy[MAX_FILE_NAME], *parent_name2, *child_name2, name_copy2[MAX_FILE_NAME];
	type pType, pType2;
	union Data pdata, pdata2;

	int vetorzao[50];
	int vetorzao2[50];


	strcpy(name_copy, path);
	split_parent_child_from_path(name_copy, &parent_name, &child_name);


	parent_inumber = lookup(parent_name, vetorzao);

	/* verifica se existe path até ao ficheiro que se quer mover, exclusive */
	if (parent_inumber == FAIL) {
		printf("invalid parent dir %s\n", parent_name);
		return FAIL;
	}

	inode_get(parent_inumber, &pType, &pdata);

	/*verifica se existe o ficheiro/ diretoria que se quer mover */
	if(pType == T_DIRECTORY){
		if (lookup_sub_node(child_name, pdata.dirEntries) == FAIL) {
			printf("doesn't exist %s in dir %s\n",
				child_name, parent_name);
			return FAIL;
		}
	}
	
	/* inumber do ficheiro/diretoria que se quer mover */
	child_inumber = lookup(child_name, vetorzao);

	strcpy(name_copy2, finalPath);
	split_parent_child_from_path(name_copy2, &parent_name2, &child_name2);

	/* inumber para onde se quer mover o ficheiro/ diretoria */
	child_inumber2 = lookup(child_name2, vetorzao2);

	inode_get(child_inumber2, &pType2, &pdata2);

	/* sitio pra onde se quer mover o ficheiro/diretoria tem que ser uma diretoria */
	if(pType2 == T_DIRECTORY){
		/* remover o ficheiro/diretoria que se quer mover do sitio original */
		if(dir_reset_entry(parent_inumber, child_inumber) == FAIL){
			printf("could not remove entry %s in dir %s\n", child_name, parent_name);
			return FAIL;
		}
		/* adicionar o ficheiro/diretoria ao sitio para onde se quer mover */
		if (dir_add_entry(child_inumber2, child_inumber, child_name) == FAIL) {
		printf("could not add entry %s in dir %s\n", child_name, child_name2);
		return FAIL;
		}
		return FAIL;
	}

	else{
		printf("can't move %s because %s isn't a dir", child_name, child_name2);
		return FAIL;
	}

	return SUCCESS;

}

void deslockar_vetor(int vetor[]){
	int x=0;
	while (vetor[x]!=-1) {
		x++;
	}
	x--;
	while (x>=0) {
		deslockando_state(vetor[x]);
		x--;
	}
}

/*
 * Prints tecnicofs tree.
 * Input:
 *  - fp: pointer to output file
 */
void print_tecnicofs_tree(FILE *fp){
	inode_print_tree(fp, FS_ROOT, "");
}
