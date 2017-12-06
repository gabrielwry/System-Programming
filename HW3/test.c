/*
 * uniqify.c
 * Copyright (C) 2012 Tudor Marcu. All rights reserved.
 */

#define _POSIX_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define BUF_SIZE 100

char prev_word[BUF_SIZE] = "";
int word_freq = 1;

void fork_sorters(int num_children, int **parse_fd, int **sort_fd, int *pid)
{
	int i, spid;

	for (i = 0; i < num_children; i++) {
		if (pipe(parse_fd[i]) < 0 || pipe(sort_fd[i]) < 0) {
			fprintf(stderr,"Failed to get pipes\n");
			exit(EXIT_FAILURE);
		}

		spid = fork();
		switch(spid) {
		case -1:
			fprintf(stderr, "Could not fork process\n");
			exit(EXIT_FAILURE);
			break;
		/* Child closes write from parser and closes read from sort/merge */
		case 0:
			printf("Sorting\n");
			dup2(parse_fd[i][0], STDIN_FILENO);
			dup2(sort_fd[i][1], STDOUT_FILENO);
			close(parse_fd[i][0]);

			close(parse_fd[i][1]);
			/* Handle sort end */
			close(sort_fd[i][0]);
			close(sort_fd[i][1]);
		        execlp("sort", "sort", (char *)NULL);
			_exit(EXIT_FAILURE);
			break;
		default:
			close(parse_fd[i][0]);
			close(sort_fd[i][1]);
			pid[i] = spid; 
			break;
		}
	}
}

void parse_input(int num_children, int **parse_fd)
{
	char buf[BUF_SIZE];
	char *word=malloc(sizeof(char) * BUF_SIZE); 
	char *temp=malloc(sizeof(char) * BUF_SIZE);
	int i;
	FILE **parser_out = malloc(num_children * sizeof(FILE *));

	for (i = 0; i < num_children; i++) {
		parser_out[i] = fdopen(parse_fd[i][1], "w");
		if (parser_out[i] == NULL) {
			fprintf(stderr, "Couldn't open write pipe[%d]\n", i);
			exit(EXIT_FAILURE);
		}
	}

	while(fgets(buf, BUF_SIZE, stdin) != NULL) {
		for (i = 0; i < strlen(buf); i++) {
			if (isalpha(buf[i]) == 0) {
				buf[i] = ' ';
			} else {
				buf[i] = tolower(buf[i]);
			}
		}
		i = 0;
		temp  = strtok(buf, " ");
		if (temp != NULL)
			sprintf(word, "%s\n", temp);
		while (temp != NULL) {
			fputs(word, parser_out[i]);
			temp = strtok(NULL, " ");
			if (temp != NULL)
				sprintf(word, "%s\n", temp);

			if (i == (num_children - 1))
				i = 0;
			else
				i++;
		}
	}

	for (i = 0; i < num_children; i++) {
		fclose(parser_out[i]);
	}
	printf("Parsing Finished\n");
	free(temp);
	free(word);
}

int mergeWords(int num_children, char **word)
{
	int i, alpha = 0;
	while (word[alpha] == NULL) {
		if (alpha == (num_children - 1))
			return -1;
		alpha++;
	}

	for (i = alpha + 1; i < num_children; i++) {
		if (word[i] != NULL) {
			if (strcmp(word[i], word[alpha]) < 0)
				alpha = i;
		}
	}
		
	if (strcmp(prev_word, word[alpha]) != 0) {
		if (strcmp(prev_word, "") != 0 && prev_word != NULL)
			printf("%d - %s\n", word_freq, prev_word);
		strcpy(prev_word, word[alpha]);
		word_freq = 1;
	} else {
			word_freq++;
	}

	return alpha;
}

void merger(int num_children, int **sort_fd)
{
	int i, alpha = 0;
	char **word = malloc(num_children * sizeof(char *));
	FILE **input = malloc(num_children * sizeof(FILE *));

	for (i = 0; i < num_children; i++) {
		input[i] = fdopen(sort_fd[i][0], "r");
		word[i] = malloc(BUF_SIZE * sizeof(char));
		if (fgets(word[i], BUF_SIZE, input[i]) == NULL)
			word[i] = NULL;
	}

	while ((alpha = mergeWords(num_children, word)) + 1) {
		if (fgets(word[alpha], BUF_SIZE, input[alpha]) == NULL)
			word[alpha] = NULL;		
	}

	for (i = 0; i < num_children; i++) {
		fclose(input[i]);
		free(word[i]);
	}
	free(word);
	free(input);
}

void fork_merger(int numProcesses, int **fd)
{
        int cpid;

        switch(cpid = fork()){
                case -1:
                        fprintf(stderr, "Could not print\n");
                        exit(EXIT_FAILURE);
                case 0:
                		printf("Merging\n");v
                        merger(numProcesses, fd);
                        _exit(EXIT_FAILURE);
                        break;
                default:
                		printf("waiting for sort\n");
                        waitpid(cpid, NULL, 0);
                        break; 
        }

}

int main(int argc, char *argv[])
{
	int i;
	int num = atoi(argv[1]);
	int **parse_fd = malloc(num * sizeof(int *));
	int **sort_fd = malloc(num * sizeof(int *));
	pid_t *pid = malloc(num * sizeof(int));

	for (i = 0; i < num; i++) {
		parse_fd[i] = malloc(2 * sizeof(int));
		sort_fd[i] = malloc(2 * sizeof(int));
	}

	fork_sorters(num, parse_fd, sort_fd, pid);

	parse_input(num, parse_fd);
	
	fork_merger(num, sort_fd);

	for (i = 0; i < num; i++) {
		close(parse_fd[i][1]);
		free(parse_fd[i]);
		free(sort_fd[i]);
	}

	free(parse_fd);
	free(sort_fd);
	free(pid);

	for (i = 0; i < num; i++) {
			wait(NULL);
	}

	return 0;
}