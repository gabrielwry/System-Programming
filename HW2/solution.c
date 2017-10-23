#include <getopt.h>
#include <stdio.h>
#include <sys/utsname.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ar.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <utime.h>
#include <dirent.h>

#define BLOCKSIZE 16

typedef int bool;
enum {false, true};

int open_archive(char * file_name, char * prog_name);
int is_archive(char * magic_string);
int print_filename(char * header);
int cmp_filename(char * file_name, char * header);
int get_file_name(char * header, char * dest);
int parse_filesize(char * header);
int parse_file_mode(char * header);
int parse_time(char * header);
int print_ids(char * header);
int print_verbose(char * header);
void output_mode(int octal_mode);
int append_to_archive(char * archive_name, char * file_name, char * prog_name);
int find_file(char * archive_name, char * file_name, char * prog_name, int * f_size, char * header_copy);
int delete_from_archive(char * archive_name, char * file_name, char * prog_name);
int extract_from_archive(char * archive_name, char * file_name, char * prog_name);
int append_all_reg(char * archive_name, char * prog_name);

//Error message handlers
void exit_nsfile(char * prog_name, char * file_name);
void exit_formatnr(char * prog_name, char * file_name);

int main(int argc, char **argv)
{
    char c;
    char header[61];
    int i;
    int in_fd;
    int file_size = 0; //size of file in archive in bytes
    char temp_fname[16];

    //Display usage
    if (argc == 1) {
        printf("Usage: myar [-]{tvqdxA} archive-file file\n");
        printf(" commands:\n");
        printf("  t\t\t- Display contents of the archive.\n");
        printf("  v\t\t- Print verbose table of contents of the archive.\n");
        printf("  q\t\t- Quickly append a file to the archive.\n");
        printf("  d\t\t- Delete files from the archive.\n");
        printf("  x\t\t- Extract files from the archive, all files if none specified.\n");
        printf("  A\t\t- Add all regular files from current directory to the archive.\n");
    }

    while ((c = getopt(argc, argv, "t:v:q:d:x:A:")) != -1) {
        switch (c) {
            //Print concise table of contents of archive:
            case 't':
                if (argc < 3) {
                    printf("%s: Not enough arguments\n", argv[0]);
                    exit(EXIT_FAILURE);
                }

                in_fd = open_archive(optarg, argv[0]);
                int counter =0;
                while (read(in_fd, header, 60) == 60) {
                    counter+=1;
                    printf("Counter is %d",counter);
                    header[60] = '\0';
                    print_filename(header);
                    file_size = parse_filesize(header);
                    if (file_size%2)
                        file_size += 1;
                    lseek(in_fd, file_size, SEEK_CUR);
                }

                if (close(in_fd) == -1)
                    exit(EXIT_FAILURE);

                break;

            //Quickly append a file to the archive
            case 'q':
                if (argc < 4) {
                    printf("%s: Not enough arguments\n", argv[0]);
                    exit(EXIT_FAILURE);
                }

                //argv[2] is first variable (archive name)
                //Make sure all files exist, exit without modifying archive if not.
                for (i = 3; i < argc; ++i) {
                    if ((in_fd = open(argv[i], O_RDONLY) == -1)) {
                        exit_nsfile(argv[0], argv[i]);
                    }
                }

                close(in_fd);
                for (i = 3; i < argc; ++i) {
                    append_to_archive(argv[2], argv[i], argv[0]);
                }
                break;

            //Extract files
            case 'x':
                if (argc < 3) {
                    printf("%s: Not enough arguments\n", argv[0]);
                    exit(EXIT_FAILURE);
                }

                //Extract all files:
                if (argc == 3) {
                    in_fd = open_archive(optarg, argv[0]);

                    while (read(in_fd, header, 60) == 60) {
                        header[60] = '\0';
                        get_file_name(header, temp_fname);
                        extract_from_archive(argv[2], temp_fname, argv[0]);
                        file_size = parse_filesize(header);
                        if (file_size%2)
                            file_size += 1;
                        lseek(in_fd, file_size, SEEK_CUR);
                    }

                    close(in_fd);
                }

                //Extract files named in command line:
                else {
                    for (i = 3; i < argc; ++i) {
                        extract_from_archive(argv[2], argv[i], argv[0]);
                    }
                }
                break;

            //Print verbose table of contents of archive:
            case 'v':
                if (argc < 3) {
                    printf("%s: Not enough arguments\n", argv[0]);
                    exit(EXIT_FAILURE);
                }

                in_fd = open_archive(optarg, argv[0]);

                while (read(in_fd, header, 60) == 60) {
                    header[60] = '\0';
                    file_size = print_verbose(header);

                    if (file_size%2)
                        file_size += 1;

                    lseek(in_fd, file_size, SEEK_CUR);
                }

                if (close(in_fd) == -1)
                    exit(EXIT_FAILURE);

                break;

            //Delete files from archive
            case 'd':
                if (argc < 4) {
                    printf("%s: Not enough arguments\n", argv[0]);
                    exit(EXIT_FAILURE);
                }

                for (i = 3; i < argc; ++i) {
                    delete_from_archive(argv[2], argv[i], argv[0]);
                }

                break;

            //Add all regular files from current directory
            case 'A':
                if (argc < 3) {
                    printf("%s: Not enough arguments\n", argv[0]);
                    exit(EXIT_FAILURE);
                }

                append_all_reg(argv[2], argv[0]);
                break;
        }
    }

    return 0;
}

/* Opens an archive file, exits with error if specified file doesn't
   exist or does not contain the magic string indicating that it's an
   archive.
   Parameters: file_name is the name of the archive, prog_name is the
   name of the program, passed from main as argv[0]. */
int open_archive(char * file_name, char * prog_name)
{
    int in_fd = open(file_name, O_RDONLY);
    char buf[BLOCKSIZE];

    if (in_fd == -1) {
        exit_nsfile(prog_name, file_name);
    }

    else {
        if (read(in_fd, buf, 8) != 8)
            exit_formatnr(prog_name, file_name); //must be at least 8 bytes long to be an archive

        if (is_archive(buf) != 1) {
            exit_formatnr(prog_name, file_name);
        }
    }

    return in_fd;
}

/* Checks if a file has the magic string header indicating that it's
   an archive.
   Parameter: magic_string should be read from the first 8 bytes of a
   file. */
int is_archive(char * magic_string)
{
    if (!magic_string)
        return -1;

    magic_string[8] = '\0';
    printf("ARMAG is %s.\n",ARMAG);
    if (strcmp(magic_string, ARMAG) == 0)
        return 1;

    return 0;
}

/* Outputs the filename contained in an archived file's header.
   Parameter: header is the 60 byte header that preceeds each file
   in an archive.
   Returns: -1 on failure, 1 on success. */
int print_filename(char * header)
{
    char file_name[16];
    int i;

    if (!header)
        return -1;

    //File name is contained in bytes 0 to 15 in the header
    for (i = 0; i < 16; ++i)
    {
        file_name[i] = header[i];
        if (header[i] == '/') {
            file_name[i] = '\0';
            break;
        }
    }
    printf("%s\n", file_name);

    return 1;
}

/* Compares a file name to the file name in an archive file's header.
   Parameters: file_name is the name of the file to be compared, header
   is the 60 byte header that preceeds each file in an archive.
   Returns: 0 if file names are equal. */
int cmp_filename(char * file_name, char * header)
{
    char header_file[16];
    int i;

    if (!header)
        return -1;

    //File name is contained in bytes 0 to 15 in the header
    for (i = 0; i < 16; ++i)
    {
        header_file[i] = header[i];
        if (header[i] == '/') {
            header_file[i] = '\0';
            break;
        }
    }

    return strcmp(header_file, file_name);
}

/* Gets the file name from an archive header and returns it through
   dest.
   Parameters: header is the 60 byte header, dest is a 16 char array. */
int get_file_name(char * header, char * dest)
{
    int i;

    if (!header)
        return -1;

    //File name is contained in bytes 0 to 15 in the header
    for (i = 0; i < 16; ++i)
    {
        dest[i] = header[i];
        if (header[i] == '/') {
            dest[i] = '\0';
            break;
        }
    }

    return 1;
}

/* Returns the file size from an archive header.
   Parameter: header is the 60 byte header.
   Returns: -1 on error, file size on success. */
int parse_filesize(char * header)
{
    char str_size[11];
    int file_size;
    int i;
    int j = 0;

    if (!header)
        return -1;

    //File size is contained from bytes 48 to 57 of the header
    for (i = 48; i < 58; ++i)
    {
        str_size[j] = header[i];
        if (header[i] == ' ') {
            str_size[j] = '\0';
            break;
        }
        ++j;
    }

    file_size = (int)strtol(str_size, (char**)NULL, 10);
    return file_size;
}

/* Returns the octal file mode from an archive header.
   Parameter: header is the 60 byte header.
   Returns: -1 on error, file mode on success. */
int parse_file_mode(char * header)
{
    char str_mode[9];
    int file_mode;
    int i;
    int j = 0;

    if (!header)
        return -1;

    //File mode is contained from bytes 40 to 47 of the header
    for (i = 40; i < 48; ++i)
    {
        str_mode[j] = header[i];
        if (header[i] == ' ') {
            str_mode[j] = '\0';
            break;
        }
        ++j;
    }

    file_mode = (int)strtol(str_mode, (char**)NULL, 8);
    return file_mode;
}

/* Prints the textual representation of an octal mode.
   Parameter: st_mode is an octal mode stored as an integer. */
void output_mode(int st_mode)
{
    if (st_mode & S_IRUSR)
        putchar('r');
    else
        putchar('-');

    if (st_mode & S_IWUSR)
        putchar('w');
    else
        putchar('-');

    if (st_mode & S_IXUSR)
        putchar('x');
    else
        putchar('-');

    if (st_mode & S_IRGRP)
        putchar('r');
    else
        putchar('-');

    if (st_mode & S_IWGRP)
        putchar('w');
    else
        putchar('-');

    if (st_mode & S_IXGRP)
        putchar('x');
    else
        putchar('-');

    if (st_mode & S_IROTH)
        putchar('r');
    else
        putchar('-');

    if (st_mode & S_IWOTH)
        putchar('w');
    else
        putchar('-');

    if (st_mode & S_IXOTH)
        putchar('x');
    else
        putchar('-');
}

/* Returns the timestamp field from an archive header
   Parameters: header is the 60 byte header
   Returns: -1 on failure, timestamp as an integer on success. */
int parse_time(char * header)
{
    char str_time[13];
    int timestamp;
    int i;
    int j = 0;

    //Timestamp is contained in bytes 16 to 27 of the header
    for (i = 16; i < 28; ++i)
    {
        str_time[j] = header[i];
        if (header[i] == ' ') {
            str_time[j] = '\0';
            break;
        }
        ++j;
    }

    timestamp = (int)strtol(str_time, (char**)NULL, 10);
    return timestamp;
}

/* Prints the owner and group id from an archive header.
   Parameters: header is the 60 byte header.
   Returns: 1 on success, -1 on failure. */
int print_ids(char * header)
{
    char owner_id[7];
    char group_id[7];
    int i;
    int j = 0;

    if (!header)
        return -1;

    /*Owner id is stored from byte 28 to 33 of the header,
      group id from 34 to 39 */
    for (i = 28; i < 34; ++i)
    {
        owner_id[j] = header[i];
        group_id[j] = header[i+6];
        if (header[i] == ' ') {
            owner_id[j] = '\0';
        }

        if (header[i+6] == ' ') {
            group_id[j] = '\0';
        }
        owner_id[j+1] = '\0';
        group_id[j+1] = '\0';

        ++j;
    }
    printf("%s/%s", owner_id, group_id);

    return 1;

}

/* This function handles the -v option. It prints a verbose list of the details of
   one member of the archive, replicating the behavior of the "ar tv" command.
   Parameter: header is the 60 byte header to be displayed.
   Returns: -1 on failure, file size on success. */
int print_verbose(char * header)
{
    int file_mode;
    int file_size;
    time_t timestamp;
    struct tm timestruct;
    char months[12][4] = {"Jan\0", "Feb\0", "Mar\0", "Apr\0", "May\0", "Jun\0",
                          "Jul\0", "Aug\0", "Sep\0", "Oct\0", "Nov\0", "Dec\0"};

    if (!header)
        return -1;

    //Print permissions
    file_mode = parse_file_mode(header);
    output_mode(file_mode);
    putchar(' ');

    //Print user/group ID
    print_ids(header);
    putchar(' ');

    //Print file size
    file_size = parse_filesize(header);
    printf("%6d", file_size);
    putchar(' ');

    //Print modification time
    timestamp = parse_time(header);
    timestruct = *localtime(&timestamp);
    printf("%s %d %02d:%02d %d", months[timestruct.tm_mon], timestruct.tm_mday, timestruct.tm_hour,
                             timestruct.tm_min, timestruct.tm_year+1900);
    putchar(' ');

    //Print filename:
    print_filename(header);

    return file_size;
}

/* Appends a file to the specified archive without checking if the archive already contains
   the file. This handles the -q option.
   Parameters: archive_name is the name of the archive to be created or added to, file_name
   is the name of the file to be added, prog_name is the name of the current program (argv[0]).
   Returns: -1 if file doesn't exist, or exits. 1 on success. */
int append_to_archive(char * archive_name, char * file_name, char * prog_name)
{
    struct stat file_stat;
    char full_filename[17]; //16 bytes
    char mod_time[13]; //12 bytes (stat.st_mtime)
    char owner_id[7]; //6 bytes (stat.uid)
    char group_id[7]; //6 bytes (stat.gid)
    char file_mode[9]; //8 bytes (stat.st_mode)
    char file_size[11]; //10 bytes (stat.st_size)
    char file_magic[3] = {0x60, 0x0A, 0x00};

    char check_archive[16];
    char buf[1];
    int bytes_written = 0;
    int read_error;
    int out_fd;
    int in_fd;

    if (!file_name || !prog_name)
        return -1;

    if (stat(file_name, &file_stat) == -1) {
        exit_nsfile(prog_name, file_name);
    }

    //Check if file is regular
    if (!S_ISREG(file_stat.st_mode)) {
        exit_formatnr(prog_name, file_name);
    }

    //Convert to strings
    sprintf(full_filename, "%-16s", file_name);
    full_filename[strlen(file_name)] = '/';
    sprintf(mod_time, "%-12d", (int)file_stat.st_mtime);
    sprintf(owner_id, "%-6d", (int)file_stat.st_uid);
    sprintf(group_id, "%-6d", (int)file_stat.st_gid);
    sprintf(file_mode, "%-8o", file_stat.st_mode);
    sprintf(file_size, "%-10d", (int)file_stat.st_size);

    /* Open archive. If it exists, verify that it's an archive file, otherwise
       initialize it with the correct file magic. */
    in_fd = open(archive_name, O_RDONLY);
    if ((read_error = read(in_fd, check_archive, 8)) == 8) {
        if (is_archive(check_archive) != 1) {
            exit_formatnr(prog_name, archive_name);
        }
    }

    //File doesn't exist:
    else if (read_error == -1) {
        out_fd = open(archive_name, O_WRONLY | O_CREAT, 0666);
        printf("%s: creating %s\n", prog_name, archive_name);
        write(out_fd, "!<arch>\n", 8);
        close(out_fd);
    }

    close(in_fd);

    //Open the archive and target file for writing
    out_fd = open(archive_name, O_WRONLY | O_APPEND);
    in_fd = open(file_name, O_RDONLY);

    if (in_fd == -1) {
        exit_nsfile(prog_name, file_name);
    }

    /* This error should never occur because the file should have been created above
       if it doesn't already exist */
    if (out_fd == -1) {
        exit_nsfile(prog_name, file_name);
    }

    //Write header
    write(out_fd, full_filename, 16);
    write(out_fd, mod_time, 12);
    write(out_fd, owner_id, 6);
    write(out_fd, group_id, 6);
    write(out_fd, file_mode, 8);
    write(out_fd, file_size, 10);
    write(out_fd, file_magic, 2);
    int file_size_bytes = (int)file_stat.st_size;

    //Write file data
    while (bytes_written < file_size_bytes) {
        read(in_fd, buf, 1);
        bytes_written += write(out_fd, buf, 1);
    }

    //Add additional \n if file size is odd
    buf[0] = '\n';
    if (file_size_bytes%2 != 0)
        write(out_fd, buf, 1);

    close(out_fd);
    close(in_fd);

    return 1;
}

/* Finds an archived file in an archive based on its name and returns the offset of the start of the
   file's header if found.
   Parameters: archive_name is the archive's name, file_name is the name of the file to search for,
   prog_name is the name of the current program (argv[0]), *f_size is a pointer to allocated memory
   for 1 integer which is used to return the found file's size, *header_copy is a pointer to a char
   array large enough to hold 60 bytes.
   Returns: offset of file's header, size of file (through *f_size), a copy of the file's header if
   header_copy is passed as NULL. */
int find_file(char * archive_name, char * file_name, char * prog_name, int * f_size, char * header_copy)
{
    int in_fd = open_archive(archive_name, prog_name);
    int file_size = 0;
    char header[61];
    int offset;

    while (read(in_fd, header, 60) == 60) {
        header[60] = '\0';
        file_size = parse_filesize(header);

        //File found. Update pointers and return
        if (cmp_filename(file_name, header) == 0) {
            *f_size = file_size;
            if (header_copy != NULL) {
                strcpy(header_copy, header);
            }
            offset = lseek(in_fd, -60, SEEK_CUR); //seek to start of matching header
            close(in_fd);
            return offset;
        }
        if (file_size%2)
            file_size += 1;

        //Skip to next header
        lseek(in_fd, file_size, SEEK_CUR);
    }


    close(in_fd);
    return -1; //no match found
}

/* Deletes the specified file from the archive by rebuilding it under the same name.
   Parameters: archive_name is the name of the archive, file_name is the name of the
   file to remove from the archive, prog_name is the name of the program (argv[0]).
   This function is used for the -d option.
   Returns: 1 on success, -1 on failure. */
int delete_from_archive(char * archive_name, char * file_name, char * prog_name)
{
    int * file_size = malloc(sizeof(int)); //amount to skip when remaking archive
    int match_offset = find_file(archive_name, file_name, prog_name, file_size, NULL);
    int bytes_read = 0;
    char buf[1];
    int in_fd;
    int out_fd;

    if (match_offset == -1) {
        free(file_size);
        return -1;
    }

    //Add 1 to odd file sizes to skip extra \n when rebuilding archive
    if (*file_size%2)
        *file_size += 1;

    in_fd = open(archive_name, O_RDONLY);

    if (in_fd == -1)
        exit_nsfile(prog_name, archive_name);

    //Remake archive
    unlink(archive_name);
    out_fd = open(archive_name, O_WRONLY | O_CREAT | O_EXCL, 0666);

    if (out_fd == -1)
        exit_nsfile(prog_name, archive_name);

    //Read till end of old archive, copy into new file of same name
    while(read(in_fd, buf, 1) == 1) {
        ++bytes_read;
        if (bytes_read == match_offset)
            lseek(in_fd, *file_size+60, SEEK_CUR); //seek +60 to skip header, +filesize to skip to end of file
        write(out_fd, buf, 1);
    }
    close(in_fd);
    close(out_fd);
    free(file_size);

    return 1;
}

/* Extracts a specified file from an archive.
   Parameters: archive_name is the name of the archive, file_name is the name of the
   file to be extracted, prog_name is the name of the program (argv[0]). This function
   is used for the -x option.
   Returns: -1 on failure, 1 on success. */
int extract_from_archive(char * archive_name, char * file_name, char * prog_name)
{
    struct utimbuf file_times; //time_t actime, time_t modtime
    int * file_size = malloc(sizeof(int)); //amount to skip when remaking archive
    char * header = malloc(61*sizeof(char));
    int match_offset = find_file(archive_name, file_name, prog_name, file_size, header);

    int file_mode;
    int mod_time;

    int bytes_written = 0;
    char buf[1];
    int in_fd;
    int out_fd;

    if (match_offset == -1) {
        free(file_size);
        free(header);
        return -1;
    }

    file_mode = parse_file_mode(header); //get file's mode
    mod_time = parse_time(header); //get file's time
    file_times.actime = mod_time; //initialize struct for utime
    file_times.modtime = mod_time;

    //Open files:
    in_fd = open_archive(archive_name, prog_name);
    out_fd = open(file_name, O_WRONLY | O_TRUNC | O_CREAT, 0666);

    if (out_fd == -1) {
        printf("Output error");
        exit(EXIT_FAILURE);
    }

    //Seek to start of matched file's data
    lseek(in_fd, match_offset + 60, SEEK_SET);

    //Extract to new file
    while (bytes_written < *file_size) {
        read(in_fd, buf, 1);
        bytes_written += write(out_fd, buf, 1);
    }

    //Update permissions and timestamps
    fchmod(out_fd, file_mode);
    utime(file_name, &file_times);

    //Clean up
    close(out_fd);
    close(in_fd);
    free(file_size);
    free(header);

    return 1;
}

/* Appends every regular file in the current directory to an archive. This is the
   -A option.
   Parameters: archive_name is the archive name, prog_name is the name of the current
   program (argv[0]).
   Returns: -1 on failure, 1 on success. */
int append_all_reg(char * archive_name, char * prog_name)
{
    DIR * dirp;
    struct dirent * current_dir;
    bool skip_file;
    char program_name[16];
    int i, j = 0;

    if (!archive_name || !prog_name)
        return -1;

    dirp = opendir(".");

    //Get rid of the ./ that comes before argv[0] to prevent adding the executable itself
    for (i = 2; prog_name[i] != '\0'; ++i) {
        program_name[j] = prog_name[i];
        if (prog_name[i+1] == '\0')
            program_name[j+1] = '\0';
        ++j;
    }

    current_dir = readdir(dirp);
    while (current_dir != NULL) {
        //Do not add non-regular files or excluded file names
        skip_file = (current_dir->d_type != DT_REG) || strcmp(current_dir->d_name, archive_name) == 0
                                                    || strcmp(current_dir->d_name, program_name) == 0
                                                    || strcmp(current_dir->d_name, "myar.c") == 0
                                                    || strcmp(current_dir->d_name, "myar.h") == 0
                                                    || strcmp(current_dir->d_name, "myar.o") == 0;
        if (!skip_file) {
            append_to_archive(archive_name, current_dir->d_name, prog_name);
        }

        current_dir = readdir(dirp); //read next entry
    }

    closedir(dirp);
    return 1;
}

/* Prints an error message and exits for when a file doesn't exist. */
void exit_nsfile(char * prog_name, char * file_name)
{
    printf("%s: %s: No such file or directory\n", prog_name, file_name);
    exit(EXIT_FAILURE);
}

/* Prints an error message and exits for when a file isn't valid archive. */
void exit_formatnr(char * prog_name, char * file_name)
{
    printf("%s: %s: File format not recognized\n", prog_name, file_name);
    exit(EXIT_FAILURE);
}