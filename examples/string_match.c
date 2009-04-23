#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include "athread.h"

#define STRING_FILENAME "string_data.txt"
#define CHAR_SIZE_TO_READ (file_info.st_size/split_factor)
#define DEFAULT_SPLIT_FACTOR 10 
//#define TEST_MODE

/* GLOBALS */
struct stat file_info;
char term_to_find[100];
int split_factor;
char *content_string;

/* END-GLOBALS */

int get_file_info() {
	if (stat(STRING_FILENAME, &file_info)) 
		return 0;
	else
		return 1;
}

char *open_file_and_read_from_byte(int byte) {
	char *read_string;

	if ((read_string = malloc(sizeof(char) * CHAR_SIZE_TO_READ)) == NULL) {
		fprintf(stderr, "Erro na alocacao de memoria para buffer. Abortando\n");
		exit(1);
	}

	memcpy(read_string, &content_string[byte], CHAR_SIZE_TO_READ);
	return read_string;
}

void *split_func(void *in, int split_factor, size_t total_size, int rank) {
	int start_at = (rank) * file_info.st_size / split_factor;
	
	return (void *) open_file_and_read_from_byte(start_at);
}


void *merge_func(void *ret_th, int split_factor, int rank, void *output) {
	int *value = malloc(sizeof(int));

	if (*(int*)ret_th == 1) 
		printf("Termo encontrado... (rank => %d)\n", rank);

	return (void *) value;
}

void *thread_func(void *in) {
	char *string_content = (char *) in;
	int *result = malloc(sizeof(int));
	
	if (strstr(string_content, term_to_find) == NULL) {
		*result = 0;
	} else {
		*result = 1;
	}

	return (void *) result;
}

int main(int argc, char *argv[]) {
	if (!get_file_info()) {
		fprintf(stderr, "Erro durante tentativa de obter informacoes do arquivo. Abortando\n");
		exit(1);
	} else {
		if (argc < 2) {
			fprintf(stderr, "Erro no numero de parametros. Uso: %s termo_para_buscar [split_factor]\n", argv[0]);
			exit(1);
		}

		split_factor = DEFAULT_SPLIT_FACTOR;
		if (argc == 3) {
			split_factor = (atoi(argv[2]) > 0) ? (atoi(argv[2])) : (DEFAULT_SPLIT_FACTOR);
		}
		
		aInit(&argc, &argv);
		strcpy(term_to_find, argv[1]);
		printf("\n*** Iniciando busca ***\n");
		printf("Termo: %s\n", term_to_find);
		printf("Tamanho do arquivo: %d bytes(~%d Kb)\n", (int) file_info.st_size, (int) file_info.st_size/1024);
		printf("Split factor: %d\n", split_factor);
	} 

	#ifdef TEST_MODE
	printf("\n\n*** Running in test mode ***\n\n");
	#endif

	/*
	 * Abre o arquivo e libera globalmente
	 */
	FILE *fp;
	if ((fp = fopen(STRING_FILENAME, "r")) == NULL ) {
		fprintf(stderr, "Erro na abertura do arquivo. Abortando\n");
		exit(1);
	}

	// le conteudo do arquivo
	content_string = malloc(file_info.st_size);

	if (!content_string) {
		fprintf(stderr, "Erro na alocao de memoria para arquivo. Abortando\n");
		exit(1);
	}
	
	fread(content_string, sizeof(char), file_info.st_size, fp);

	athread_t th;
	athread_attr_t attr;
	int *found_at = malloc(sizeof(int));

  athread_attr_init(&attr);
  athread_attr_set_inputsize(&attr, CHAR_SIZE_TO_READ * sizeof(char));
  athread_attr_set_returnsize(&attr, sizeof(int));
  athread_attr_set_splitfactor(&attr, split_factor);
  athread_attr_set_splitfunction(&attr, split_func);
  athread_attr_set_mergefunction(&attr, merge_func);
  athread_create(&th, &attr, thread_func, (void *) content_string);

	athread_join(th, (void *)found_at);
	
	aTerminate();
	return 0;
}
