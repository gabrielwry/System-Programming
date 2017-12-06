/* THIS CODE IS MY OWN WORK. IT WAS WRITTEN WITHOUT CONSULTING
 * A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Runye Wang */

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>

#define MQ_KEY 23561
#define SM_KEY 23561
#define PERM 0666

#define INIT_FLAG  1
#define KILL_FLAG  2
#define FOUND_FLAG 3
#define INIT_SUCCESS_FLAG 4

#define RANGE 33554432
#define SEG_NUM 32
#define INT_PER_SEG 32768
// INT_PER_SEG * SEG_NUM * 8 * sizeof(int) = RANGE

#define MAX_PERFECT 20
#define MAX_PROCESS 20

typedef struct {
	int pid;		
	int found;		
	int tested;
	int skipped;
} stats;

typedef struct {
	int bitMap[SEG_NUM][INT_PER_SEG];	// bitmap array to hold at most 2^25 bits
	int result[MAX_PERFECT];
	stats process[MAX_PROCESS]; //process stats
} segment;

typedef struct {
	long flag;		// message flag
	int data;		// data(pid or perfect number)
} msg;

extern int errno;