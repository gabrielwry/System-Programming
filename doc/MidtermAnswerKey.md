
# System Programming Fall 2017 Midterm
-- This answer key is for my own usage. Collaborated with Jian Chen.  
## True and Flase


| No.| Answer|        Explanation     |
|  :- |:--:  | :------------------------------------|
|  1  | F    | The implementation of a standard libc depends on version of Operating System. Refer to book **1.3 System Call** .|
|  2  |   F  | `fputc()` read from user buffer so it saved from extensive system call, but the cache is not from U-BLOCK which only stored system info.  Refer to [IBM PAGE](https://www.ibm.com/support/knowledgecenter/en/ssw_ibm_i_72/rtref/fputc.htm) and book **Session 2.12**.                |
|  3  |   T  | `getc()` outperform `fgetc()` as doesn't have to do a system call (moving the pointer) to access the stream each time reading a char. And it comes in a macro form. Refer to [IBM Page](https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.bpxbd00/getc.htm) .                 |
|  4  | F    | I am not sure about this one. But, environment variables are stored in a file before a process runs, and it then is  passed to the program and exists on the DATA segment. Refer to [ArchWiki](https://wiki.archlinux.org/index.php/environment_variables) .  |
|  5  |   F   | ELF header doesn't have those sizes. DATA TEXT and SEGMENT are for processes, they were established when the process runs. Refer to  [Stack Overflow](https://stackoverflow.com/questions/6270945/linux-stack-sizes) .           |
|  6  |   T   | Hard link uses i-number to identify objects, while symbolic link contains the text path to the objects. Making it possible to access an object on a different disk. Refer to book **Session 3.3.3** .              |
|  7  | F | When the `write()` returns, it already transferred data to the buffer, but has not written to actual file yet. Refer to book **Session 2.9** . |
|  8  |    F  | `read()` only access a data and increment file descriptor, it doesn't have disk activities. |
|  9  |   F   | Once the count reaches 0, the file name is destroyed, but the actual file may exist until the last process has it open closed. Refer to [Man Page](http://man7.org/linux/man-pages/man2/unlink.2.html) .|
|  10 |   T   | need `fcntl()` to set the non-blocking flag. Pipe clears the NON_BLOCKING flag when first initiated.       |
|  11  |    F | I guess it is the way this question is organized is really confusing. ">" and "<" redirection is implied and won't be examined in argv.|
|  12  |    T  | By definition, a super block is a record of the characteristics of the file system . Refer to [this page](http://www.linfo.org/superblock) or book **Session 3.2.2**.                |
|  13  |   F   | the & bit operation will actually test the ith bit.|
|  14  | F    | The child process created by `fork()` will have to inherit environment variables. Refer to book **Session 5.5**|
|  15  |    16  | Each indirect block stores 8K/4= 2K i-nodes which store 8K data. 2000*8000 = 16,000,000|
|  16  |   F  | break 100 will actually stop the process at line 100, but the debugger is still running.|
|  17  |   T | File descriptor will stay open after an `exec()`, need to call `close()` if want to close it. Refer to Book **Page287** |
|  18  |    F  | C doesn't have garbage system, space reserved by `mallloc()` will be marked as reusable eventually|
|  19  |   F   |`typedef` doesn't automatically allocate the memory, sometimes the needed space is unknown and need to call `malloc()` to reserve space|
|  20  |  F   | `C`'s input and output operation relies on the standard `libc`package functions. It is not built in the language|
|  21  |   F   | When the child change the `fd` with a `lseek()` parent's next `read` or `write` will be at the new location. But the descriptor is independent. Child process closes a file descriptor won't affect the parent's. Refer to booko **Page 297** .|
|  22  |   F   | There is exactly one file description, but the table can have multiple entry of descriptor pointing to the same description. Refer to book **Session 2.2.2** |
|  23  |   F   | `sizeof` operation only works data type, it should return the allocated memory space, file doesn't have a fixed one, needs to use `stat` for actual file size. Refer to [GeekForGeek](http://www.geeksforgeeks.org/sizeof-operator-c/) .|
|  24  |    F  | `fflush()` only ensure data to be written to buffer, and then a stream, this data don't have to be written onto a disk. Refer to [man page](http://man7.org/linux/man-pages/man3/fflush.3.html) .|
|  25  |    F  | Zombie process is a process that has finished but still needs to report to parent process. A process whose parent died is called an Orphan process. Refer to [GeeksforGeeks](http://www.geeksforgeeks.org/zombie-and-orphan-processes-in-c/) .|