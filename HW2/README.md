# C Program to Simulate POSIX `ar` command

This is the second home work of Emory CS 450 **System Programming** 2017 Fall, taught by Professor Ken Mandelberg.
Please note that this program is subject to **Honor Code of Emory University**.

## Note

The POSIX `ar` command is a command to control archive type files. The [man page](https://www.unix.com/man-page/posix/1posix/ar/)
provides a detailed description. This program simulates this command's `-t -v -q -x -A` options.

## Code Structure

The supported options and their behaviors are described below: <br>
`-t` : print out the name of files in archive file <br>
`-v` : print out verbose table (file name, type, mode, user ID, group ID) of files in archive file <br>
`-q`: quick append a file in the current `dir` to the archive file, if the archive file doesn't exist, it will automatically create one <br>
`-x` : extract a file from archive file <br>
`-A` : append all regular files in the current `dir` into the archive file which will be created if it doesn't exist <br>

A `hdr` structure is defined to store information retrived from the archive file header. It is defined as:
```C
typedef struct _hdr { // ar header data structure to store file infomration
	char identifier[16]; // length 16 identifier
	time_t timestamp; // timestamp
	int uid; // owner id
	int gid;// group id
	int mode; // file mode
	int size; // size already
	int bits;// actual bits read by read()
	}hdr;
 ```
 Helper functions for reading data from file header, printing info, and writting file header are defined:
 ```C
void print_mode(int mode);
void print_time(time_t timestamp);
int is_archive(int fildes); // check if file is archive file
int write_data(int in_fildes,int out_fildes,hdr* file_hdr);
int find_header(int fildes, char* filename); 
hdr* read_header(int fildes);
hdr* write_header(int out_fildes,char* filename);
```

## Summary
This programming assignment turned out to be chanllenging. The first chanllenge is the usage of `file descriptor` and read the first 
60 bytes of data as file header into the `hdr`. Remeber the specfic information about the file is stored as null-terminated string. 
Second, to extract data back from archive file requires using `lseek()` to jump the right amount of bytes and reach the real data of the files.
Third, the verbose table take some peculiar format, especially for mode and time, you need to understand the permission mask and use the 
correct `time` function to restore those information. And last, you need to know how to use `stat` to read in file info into the `file_stat` structure. 
