# System Programming Final Review

## Summary:
This write up will summarize and categorize important knowledge about system programming from the text book *Advanced Unix Programming 2nd Edition* and the notes from the class CS450 offered by Emory Computer Science Department, taught by Professor Ken Mandelberg, who constantly sighed during the lectures. Note this is for my own study and usage. 

## Final Exam Range:
-  Ch1. 1-1.4,1.7
-  Ch2. ALL
-  CH3. 3.9.1 only in 3.9, and everything else
-  Ch5. Thru 5.3
-  Ch6. 6.1-6.4 (easy on 6.3)
-  Ch7. 7.1-7.2, 7.4, 7.5.1, 7.5.2, 7.8, 7.9, 7.12, 7.13.1, 7.13.2
-  Ch9. 9.1 - 9.1.6, 9.1.9, 9.1.10, 9.4, 9.6, 9.7.1, 9.7.2 

## Content:
- [C and UNIX](#c-and-unix)
- [File I/O](#file-io)
- [Process and Threads](#process-and-threads)
- [Interprocess Comunication](#interprocess-communication)

## Codes:
- [Goldbach test with Sieve Method](https://github.com/gabrielwry/System-Programming/tree/master/HW1)
- [Archive program to do similar `ar` as system call](https://github.com/gabrielwry/System-Programming/tree/master/HW2)
- [uniqify and sort words from `stdin`](https://github.com/gabrielwry/System-Programming/tree/master/HW3)
- [Interprocess communication programs to compute perfect numbers](https://github.com/gabrielwry/System-Programming/tree/master/HW4)

### <span name = "c-and-unix"> C and Unix</span>
 - UNIX versions and standards:
	- UNIX was first developed as a research project in AT&T Bell Lab
	- Licenses of UNIX make their own modification. **BSD**, developed by Berkeley University supports **sockets** and **virtual memory.**
	- UNIX world divides to **BSD** side and **System V side**.
	- **POSIX** is a standard for UNIX system call and commands from IEEE.
	- If an application conforms the **standard** and the **language** (`C, C++`) it is written in, the application's source code is portable and can be compiled and run with no changes.
	- Three principal strains of UNIX:
		- Closed commercial systems;
		- BSD-based open source systems;
		- Linux;
 - System Calls
	- C and C++ Bindings:
	  - A system call execute some special codes that transfers control from user to kernel and back, and hence it is **expensive**
	  - Every system call is defined inside the **header** file, which can have conflicts due the implementation of the OS version, so sometimes a header file needs to be adjusted to work properly. 
	  - Other languages also have standard facilities to support some POSIX facilities. 
	  - Several tips for system call usages:
		  - include header (废话);
		  - do error handling, maybe use `perror` really convenient;
		  - `cast` is not very safe, it will suppress compiler warning if the types don't match;  
		  - check if call is thread safe;
		  - write standardized, so your program is portable;
		- Read the man page for function syntax and synopses.
		
	- Error Handling :
		- always check for error, most likely the call will return `-1` for error, and it will set the `errno`;
		- There are system calls's errors depends on changing `errno`
		- There is a `strerror()` call inside `<error.h>` that will translate the `errno` to understandable string.
		- error macro is a gadget of codes that handle error checking for us, like `EC_CLEANUP_BGN` and `EC_CLEANUP_END`
		
 - Date and Times
	 - Calendar time: used to log information about access, modification or status change times of a file. 
		 - Four types: , 
			 1. arithmetic type *time_t* seconds since epoch, midnight , Jan 1st, 1970, always implemented as a *long 32-bit signed int*;
			 2. struct type *timeval* time in seconds and millisec;
			 3. struct type *tm*, breaks down to year, moth, day, hour, minute, sec
			 4. string, like *Tue Jul 23 09:44:17 2002*;
	 - Execution time: used for tracking process execution. 
		 - Main types:
			 -  long type *clock_t*, time interval in units of **CLOCKS_PER_SEC**
			 -  struct type *timeval* holds an interval in seconds and microseconds
			 - struct type *timespec* holds interval in seconds and nanoseconds.



### <span name = "file-io">File I/O</span>
 - File types:
	 - Ordinary File: segment of bytes
		 - The structure of a disk:
		 ```
			 |   Boot Block  |    
			 ===============
			 |   Super Block |  <- Stores parameters of size
			 ===============
			 |   I-nodes     | <- Meta data for a file (length, permission, group owner, time stamp, type, block list
			 ===============
			 |               |
			 ===============
			 |       DATA    |
		```
		
		 - Directories: has an i-node itself, using a 2-column to keep name -> i-node
			 - Relative path name starts with the current dir, absolute path starts with the root dir (usually i-node 2, stored when kernel is first constructed)
			 - 

 - File Descriptor and Open File Description
	 - File Descriptor:
		 - total: 1 - N, 0 is reserved for `stdin`, 1 is for `stdout`, 2 is for `stderr`
		 - UNIX filter read from `STDIN_FILENO` and write to `STDOUT_FILENO` so the shell can use it in pipeline. 
		 - can be used for any types of file
		 - `open()` for files with regular file, special files, and FIFO, `pipe()` to open un-named pipes;
	 - Open File Description: 
		 - file descriptor is just a pre-process table entry which will point to  a file-table entry, Open File Description and then point to the data via **i-node**
		 - multiple file descriptors can point to the same file description
		 - each `open` and `pipe` call creates a new **file descriptor** and a **file description**
		 - duplication of file descriptor will point to the same file description. (`dup,dup2,fork` system calls)
	
	
	 - Permission: 
		 - 9 permissions in total: read,write,execute for owner, group and others
		 - Use bit operation `|` to set permission mask
		 

 - `open` and `create` system call
	 - `open` open an existing file, or create one (can only be regular file), and return a file descriptor which can be later used.
	  - Existing File: specified by `path`, use flags `O_RDONLY O_WRONLY O_RDWR`, `PERM` arguments are always omitted for existing files;
		  - file offsets is where `read` and `write` will occur and is placed at the first byte of the file
		  - files descriptor returned is the lowest number available. 
	- Creating File: need O_CREAT flag, `PERM` is used to set the file's permission
			- O_TRUNC can make a fresh clean file with no data in it, if the process have right permission
			- new file will need write permission in parent directory, existing files only care about permission on the file itself
			- `create` system call to take care of the combination of flags
			- Ownership: 
				- owner set to effective user-ID of the process
				- group is set to either groupID of the parent directory or the effective group-ID of the process
			- O_EXCL returns error if exists.
		- Using file as a lock:
			- For exclusive access to resource: try open a file with O_EXCL when trying to access resources, only one process will succeed, when that one process finished, it will unlink the file and other process can succeed on access theh resource. 

 - `Unmask` and `unlink` 
	 - `unmask()` is used to change the permission group 9-bit, it will return the old mask
	 - `unlink()` reduces the link number in i-node by one, if the count reach 0, the file system will discard the file. 
		 - Note: if a process that has this file open is not closed yet, the actual file data will remain on the disk but the file name will be removed once the count reached 0.
		 - Any kind of file can be unlinked,  but only superuser can unlink a dir, always use rmdir a directory. 

 - Temporary Files: mkstemp guarantees to create a file with unique name, need to arrange `unlink()`

 - File offset and O_APPEND:
	 - A file offset is a position in regular file where next `read` or `write` will occur
	 - Independent offset each time a file is opened, as the file description is unique.
	 - Without the O_APPEND, the offset starts at zero, and unless specified, the `read` and `write` are sequential. 
	 - O_APPEND will prevent from overwriting data, good for logging (setting offset of write to EOF automatically)

 - `write()`:
	 - writes nbyte to the current position pointed by offset, and increment the offset by the number of bytes written;
	 - `write()` doesn't really write data to the disk, it actually first transfers data to a buffer and then return; If UNIX crashes: 
		 - real data won't be written on disk
		 - process won't be notified about the error, partial write won't trigger the errno
		 - the order of the physical write can not be controlled

 - `read()`: read n-bytes from the current position specified by the offset, won't be affected by the O_APPEND, partial read won't trigger the errno.
 - `close()` only mark the file descriptor as reusable, when the last file descriptor pointed to a open file description is closed, the open file description closed as well ( the file description keeps the count of the file descriptors pointed to it, so it knows when the last one was deleted), then if the last file description pointed to an in memory i node is closed, the in memory i node is closed. Doesn't have to `close` actually, it will automatically close once the process terminated. 
 - User Buffering:  Not really much to say, BUFIO will accelerate the provess. 

 - `lseek()`: set the file offset, return the result offset
	 - whence:  SEEK_SET -> set to pos,  SEEK_CUR -> set to current + pos (can be positive, 0, or negative), SEEK_END -> set to file size + pos
	 - resulting offset must be non-negative

 - `pread() pwrite()`: it is the `read` and `write` ignoring the offset, don use and don't set . 
	 - will solve the problem of another process changing the offset between `lseek()` and `read, write`

 - `readv() writev()`:

 

		 

 
 