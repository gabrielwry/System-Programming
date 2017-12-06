#define _POSIX_SOURCE

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>

void make_pipes(int num_pipes, int arr[][2]);
void parser_process(int num_pipes, int arr0[][2], int arr1[][2], FILE* parser_to_sort_stream[]);
void child_process(int num_pipes, int arr0[][2], int arr1[][2], int child);
void merger_process(int num_pipes, int arr0[][2], int arr1[][2], FILE* sort_to_merger_stream[]);
void sort(int *read_pipe_fd, int *write_pipe_fd);

int main(int argc, char *argv[])
{
    int num_pipes = atoi(argv[1]);
    /* read/write, number of pipes*/
    int pfd0 [num_pipes][2];
    int pfd1 [num_pipes][2];
    int i;
    int j = 0;
    FILE *parser_to_sort_stream[num_pipes];
    FILE *sort_to_merger_stream[num_pipes];
    pid_t pid[num_pipes];


    if (argc < 2 || strcmp(argv[1], "--help") == 0){
        printf("not enough arguments");
    }

    make_pipes(num_pipes, pfd0);
    make_pipes(num_pipes, pfd1);

    /* fork off sorting processes */
    for (i = 0; i < num_pipes; i++){
        switch(pid[i] = fork()){
        case -1:
            strerror(errno);
        case 0:
            child_process(num_pipes, pfd0, pfd1, i);
        default:
            break;
        }
    }
    /*fork off merger process*/
    switch(fork()){
    case -1:
        strerror(errno);
    case 0:
        merger_process(num_pipes, pfd0, pfd1, sort_to_merger_stream);
    default:
        parser_process(num_pipes, pfd0, pfd1, parser_to_sort_stream);
    }

    while(wait(NULL) != -1);
    return 0;
}

void parser_process(int num_pipes, int arr0[][2], int arr1[][2], FILE* parser_to_sort_stream[]){
    int i;
    int j;
    char read_buffer[100];
    int buff_len;

    /* close all read end(s) of pipe for parent*/

    for(i = 0; i < num_pipes; i++){
        if(close (arr0[i][0]) == -1){
            fprintf(stderr, "i failed to close parser to sort read pipe # %d in parser process\n", i);
            exit(EXIT_FAILURE);
        }

        if(close (arr1[i][0]) == -1){
            fprintf(stderr, "i failed to close sort to parser read pipe # %d in parser process\n", i);
            exit(EXIT_FAILURE);
        }

        if(close (arr1[i][1]) == -1){
            fprintf(stderr, "i failed to close sort to parser write pipe # %d in parser process\n", i);
            exit(EXIT_FAILURE);
        }
        parser_to_sort_stream[i] = fdopen (arr0[i][1], "w");
    }

    /* write string to children through n pipes in round robin fashion*/
    for (i = 0; fscanf(stdin, " %99[a-zA-Z]s", read_buffer) != EOF; i++){
        /* make entire string lower case*/
        buff_len = strnlen(read_buffer, 100);
        for (j = 0; j < buff_len; j++){
            read_buffer[j] = tolower(read_buffer[j]);
        }
        fputs(read_buffer, parser_to_sort_stream[(i + num_pipes)%num_pipes]);
        fputs("\n", parser_to_sort_stream[(i + num_pipes)%num_pipes]);
        fprintf(stderr, "%s (size = %d) (stream = %d)\n", read_buffer, buff_len, ((i + num_pipes)%num_pipes));
    }

    /* parent clean-up, send EOF to child by closing stream*/
    for(i = 0; i < num_pipes; i++){
        fclose(parser_to_sort_stream[i]);
    }
}

void make_pipes(int num_pipes, int arr[][2]){
    int i;

    for(i = 0; i < num_pipes; i++){
        if(pipe(arr[i]) == -1){
            (strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
}

void child_process(int num_pipes, int arr0[][2], int arr1[][2], int child)
{
    int i;
    int buff_len;
    char child_buffer[100];

    /*close n-1 parser to sort reads, and n-1 sort to mergers writes*/
    for (i = 0; i < num_pipes; i++){
        if (i != child){
            if(close (arr0[i][0]) == -1){
                fprintf(stderr, "i failed to close read pipe # %d on child # %d\n", i, child);
                exit(EXIT_FAILURE);
            }
            if(close (arr1[i][1]) == -1){
                fprintf(stderr, "i failed to close write pipe # %d in child # %d\n", i, child);
                exit(EXIT_FAILURE);
            }
        }
    }

    /*close n parser to sort writes, and n sort to merger reads*/
    for (i = 0; i < num_pipes; i++){
        if(close (arr1[i][0]) == -1){
            fprintf(stderr, "i failed to close read pipe # %d on child # %d\n", i, child);
            exit(EXIT_FAILURE);
        }
        if(close (arr0[i][1]) == -1){
            fprintf(stderr, "i failed to close read pipe # %d on child # %d\n", i, child);
            exit(EXIT_FAILURE);
        }

    }

    /*redirect stdin and stdout for sort process*/
    sort(&arr0[child][0], &arr1[child][1]);

    /*close remaining parser to sort read, and sort to merger write*/
    if(close (arr0[child][0]) == -1){
        fprintf(stderr, "i failed to close read pipe of child %d\n", child);
        exit(EXIT_FAILURE);
    }
    if(close (arr1[child][1]) == -1){
        fprintf(stderr, "i failed to close read pipe of child %d\n", child);
        exit(EXIT_FAILURE);
    }

    /* Parent is waiting for child to exit */
    exit(EXIT_SUCCESS);
}

void merger_process(int num_pipes, int arr0[][2], int arr1[][2], FILE* sort_to_merger_stream[])
{
    int i;
    char buffer[num_pipes][100];
    int empty_flag = 0;
    char tmp_buffer[100];

    for (i = 0; i < num_pipes; i++){
        /*close n sort to parser writes, and fdopen n sort to parser reads*/
        if(close (arr1[i][1]) == -1){
            fprintf(stderr, "i failed to close write pipe # %d on merger\n", i);
            exit(EXIT_FAILURE);
        }
        /*close n parser to sort read/writes*/
        if(close (arr0[i][1]) == -1){
            fprintf(stderr, "i failed to close parser to sort write pipe # %d in merger process\n", i);
            exit(EXIT_FAILURE);
        }
        if(close (arr0[i][0]) == -1){
            fprintf(stderr, "i failed to close parser to sort read pipe # %d in merger process\n", i);
            exit(EXIT_FAILURE);
        }
        /*open sort to merger stream for read*/
        sort_to_merger_stream[i] = fdopen (arr1[i][0], "r");
    }

    while(empty_flag < num_pipes){
        for(i = 0; i < num_pipes; i++){
            if(fgets(buffer[i], 99, sort_to_merger_stream[i]) == NULL){
                empty_flag++;
                break;
            }
            fprintf(stdout, "%s", buffer[i]);
        }
    }

    for (i = 0; i < num_pipes; i++){
        if(fclose (sort_to_merger_stream[i]) == -1){
            fprintf(stderr, "i failed to close sort to merger read pipe # %d in merger process\n", i);
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
}

void sort(int *read_pipe_fd, int *write_pipe_fd){
    if(dup2(*read_pipe_fd, STDIN_FILENO) == -1){
        fprintf(stderr, "Failed to to redirect stdin\n");
        exit(EXIT_FAILURE);
    }
    if(dup2(*write_pipe_fd, STDOUT_FILENO) == -1){
        fprintf(stderr, "Failed to to redirect stdout\n");
        exit(EXIT_FAILURE);
    }
    execlp("sort", "sort", NULL);
}