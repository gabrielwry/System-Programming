# Multi-Procee Computing Perfect Number
This is the third home work of Emory CS 450 **System Programming** 2017 Fall, taught by Professor Ken Mandelberg.
Please note that this program is subject to **Honor Code of Emory University**.

## Note
Perfect number is a number that is equal to the sum of his divisors, fo example 6 is a perfect number because 6 = 1 + 2 + 3.
A more detailed description can be found [here](https://en.wikipedia.org/wiki/Perfect_number).

## Code Structure
This is a very tedious program with tons of detail. All header file and global constants are defined in the `hw4.h` header file which 
should be included in the three individual files `compute.c`, `manage.c`, and `report.c`.<br>
Three structures are defined:
``` C
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
```
`stats` includes the process information of one `compute` process. .<br>
`segment` is the structure for shared memory, it also includes a `bitMap` to test up to 2^25 integers for perfect number, there are also
helper functions similar to the first Goldbach.c part that can be found [here](https://github.com/gabrielwry/System-Programming/blob/master/HW3/README.md).<br>
`msg` is a user-define `msg_buf` structure but modified to only hold integer as message, the usage should be sufficient.<br>
There can be multiplle `compute` processes running and testing from different integer up to the limit, so the bitMap is used for marking the 
tested integer and one integer should never be tested twice. <br>
The inter-process communication is available via `shared memory` and `message queue`. There are four types of messages defined:
``` C
#define INIT_FLAG  1
#define KILL_FLAG  2
#define FOUND_FLAG 3
#define INIT_SUCCESS_FLAG 4
```
The complete process goes as this:<br>
First, a `compute` process sends a `INIT_FLAG` message with its pid to the `manage` process and `manage` process initates its stat and store in the 
stats array, once the initiation succeeds, the `manage` send a `INIT_SUCCESS_FLAG` with the array index back to the `compute` process so it is informed of his 
unique index in that array. And when the `compute` process is shut down, it sends a `KILL_FLAG` message with its own index so it can be cleared out from the array<br>
Second, the `compute` process starts testing perfect number, once it finds a perfect number, it sends a `FOUND_FLAG` message with the found 
perfect number to the manager, and manager add it to the result array.<br>
Thrid, there are two usage of the `report` process. One, whenever it is called, it reads from the shared memory and reports the found 
perfect number, each running `compute` process's stat. Two, when it is called with the `-k` option, it kills all running `compute` processes.<br>

## Summary
The challenging part of this assignment is 1st. understanding the usage of `msgsnd` and `msgrcv`; 2nd. remeber to send message back to 
computing process once the `manage` finished the initialization process; 3rd, a clean exit of the `computing` and `managing` process, which
requires hook the newly defined `sigaction` to `SIG_INT SIG_QUIT SIG_HUN`. Other than that, this program should be reallu intuitive. 
