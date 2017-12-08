# System Programming Final Review

## Summary:
This write up will summarize and categorize important knowledge about system programming from the text book *Advanced Unix Programming 2nd Edition* and the notes from the class CS450 offered by Emory Computer Science Department, taught by Professor Ken Mandelberg, who constantly sighed during the lectures. Note this is for my own study and usage. 

## Final Exam Range:
-  Ch1. 1-1.4,1.7
-  Ch2. ALL
-  CH3. 3.9.1 only in 3.9, and everything else
-  Ch5. Thru 5.13
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

 - File System:
	 - The block I/O system is accessed via a block special file, or block device that interfaces directly with the disk;
		 - Disk can be dived to different volume, partition, or file system.
		 - Buffer cache can be performed on block Disk Device but not Raw Disk Device; dir, files, and i-nodes are not supported for either device. 
	
	 - `statvfs() and fstatvfs()`: get file system information by path or by fd
	 - Mounting and unmounting file system: connecting to a disk device to the hierarchy tree started with root is called mounting, and disconnecting is called unmounting. 
		 - need device name and the intended directory to mount a file system, original content of the directory is now hided, will reappear when the device is unmounted
	
	 - Hard link and Symbolic link:
		 - hard link: an entry in a directory with a name and an i-number 
			 - create: can get a hard link when any types of file is created (including dir), and can get additional hard link to nondirectories with `link()` with oldpath and newpath, the two pathes are equivalent in anyway. The second new link must not be existed already, or otherwise need to unlink first. 
			 - rename: use `mv` command, but won't deal with link to another file system
				 - `rename()` system call: if new path exists, remove it with unlink or rmdir, link oldpath with new path even if the old path is a dir, remove the oldpath with unlink or rmdir
				 - Note that, new path and old path have to be either both dir or both files, if new path is dir, it has to be empty, you need write permission in new path's parent, if rename fails, everything left unchanged

		 - symbolic link:  symbolic links put path of file to be linked to in the data part of an actual file, no read or write but use system calls, symbolic link will recursively point to next link until something that is not a symbolic link is reached.
			 - create: `symlink()` system call to make symbolic link, works like `link()`, but make a hard link to a symbolic link file that contain the string given by new path
			 - read: `readlink()` 

	 - Accessing and Displaying File Metadata:
		 - `stat() lstat() fstat()`: `stat` get file info by path, `lstat` get file info by path without following symbolic link, `fstat` get file info by file descriptor; `lstat` will display the metadata file of the symbolic link, if reached one.
	

	 - Directory: UNIX implemented directory as a regular file, except a special bit is set in i-node and kernel does not permit writing on it. 
		 - Read: `opendir()` return dir pointer followed by the path `closedir()` close the dir pointed by the dirp pointer; `readdir()` read the dir pointed by the pointer returned from `opendir()` to the structure `dirent` which has i-number and name. `rewinddir()` can read it again without `close` and `reopen` it. Use `readdir_r()` to supply a memory to read the info into and supports multi threads. 
		 - `chdir() fchdir()` system call behind `cd` command, change directory by path or by file descriptor. 
		 - `mkdir() rmdir()`
			 - `mkdir()` automatically creates the . and .. dir
			 - `rmdir()` has to remove non-empty dir, or multiple unlink are needed first. 
	

	 - Changin I-Node:
		 - `chmod() fchmod()`: can change a file's S_ISUID, S_ISGID, S_ISVTX and permission
		 - `chown() fchown() lchown()`: change owner and group of a file by path or by fd, of regular file or symbolic link file. 
		 - `utime()` set file access time with struct `utimbuf`

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

 - `readv() writev()`: can write contagious data to the file in different places at once, need to set up the `*iov` to make it contain a pointer to data and the size of the data. Save some time by avoiding multiple system calls.

 - Synchronized I/O:
	 - Synchronized vs. Synchronous:  Normally UNIX is un-synchronized and synchronous
		 - Synchronized means the `write()` doesn't return until the actual data is written on the disk; 
		 - Synchronous means the `read()` doesn't return until data is available and `write()` has at least put the data in kernel buffer, and actually writes to disk if it is also Synchronized.
	 - Buffer-flushing System Calls:
		 - `sync()` tell the kernel to schedule flushing all buffer written so far, but it returns once it is scheduled. Used when kernel is shut down or a portable device is unmounted.
		 - `fsync()`flushes buffer specified by `fd` and didn't return until buffer is indeed flushed, if the System is set to be synchronized. 
		 - `fdatasync()`is faster than `fsync()` as it only flushes the real data, not the controlling data like modification time. 
		 - open flags for synchronization: 
			 - O_SYNC: an implicit `fsync()` for every write;
			 - O_DSYNC: an implicit `fdatasync()` for every write
			 - O_RSYNC: update the access time in a synchronized manner.
	

 - `truncate()` and `ftruncate()`: `truncate()` with path name, and `ftruncate()` with fd to shrink the file. 


 ### <span name = "process-and-threads">Process and Threads</span>
 - Environment:
	 - when a UNIX program runs, it receives two groups of data, the arguments and the environment, which are both array of character pointer to NUL-terminated strings. 
	 - global variable `environ` has the form name = value; can use `getenv()` to get the corresponding envrion; use `putenv()` `setenv()` and `unsetenv()` to adjust environment variables 

 - `exec()` system call:`exec()` system call reinitialized a process from a designated program, always used with `fork()`
	 - Program, processes, and threads:
		 - Program, collection of instruction and data that is kept in a regular file on disk, marked executable in i-node; contents obey rules established by the kernel; program into text-file -> object file with machine-language translation; linker is used to bind object file with libraries
		 - Process, is created to run a program; contains three segments: instruction seg, user data seg, system data seg (include current dic, open file des, accumulated CPU time); 
		 - Threads, are tracked by kernel: separate flow of control through instructions, start with one thread, unless execute special system call to create another;
		 - There is no functional relationship between processes initialized from the same program ( process can not detect shared instruction seg); strong functional replationship between threads in the same process
		 -  Process has inheritance, threads are equal ( all threads have equal access to all data and resources, not copy)

 - `fork()` create a new process by copying the system-data segment from the old process, child receives a 0 on success return and parent receives the process-ID of the child. Then the child usually does an exec and the parent either waits or do something else.
	 - Several things that are not inherited by the child process:
		 - pid (废话）
		 - if the parent is multiprocess, only the one calling fork() exists on child
		 - same file description but different file descriptor
		 - execution time
	

 - `exit()`: same as returning a value from main and call `exit()` on that argument
	 - _exit and _Exit one is from UNIX, the other from standard C
	 - `exit()` a higher level than `_exit` as it does some sort of clean up, call function registered atexit, and do a flush of I/O buffer.
	 - usually call `_exit` in a child process that hasn't done an `exec`
	 - upon termination: all open file descriptors are closed, all child processes now have a special system process as parent

 - `wait(),waitpid(),waitid()`: waits for a child process
	 - `waitpid()`: 
		 - pid  argument:
			 - > 0: wait for the specific child process with pid
			 - -1: wait for any child process
			 - 0: wait for any child process in the same process group as the calling process
			 - < -1: wait for any child process in the process group whose process-group id is -pid
		 - only direct child created by `fork()` can be waited for, normally a process should wait for every child it created, or the terminated child process may exist in system as zombie until parents terminated
		 - a child changes status is waitable, can cause at most one return from waitpid
	

	 - `wait()` is a short hand for `waitpid` with pid = -1
	 
	 - `waitid()` get the status of a process back and keep it waitable

 - Setting User and Group IDs and Process IDs: Kernel keeps the saved id that were set by last `exec()`, no ordinary process can explicitly change real user ID or saved ID, except `exec` can change saved id; ordinary process can change effective ID to real or saved ID, superuser can change real and effective ID to any value, saved ID changes with the real ID if a superuser changes it. 
	 - use `seteuid, setegid` superuser can also use `setuid, setgid`
	 - `getpid()` and `getppid()` can get process ID or process ID of parent.

 ###  <span name = "interprocess-communication"> Interprocess Communication</span>
 