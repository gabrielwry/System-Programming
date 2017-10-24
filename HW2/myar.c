#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <utime.h>
#include <errno.h>
#include <string.h>

extern int errno ;

typedef struct _hdr { // ar header data structure to store file infomration
	char identifier[16]; // length 16 identifier
	time_t timestamp; // timestamp
	int uid; // owner id
	int gid;// group id
	int mode; // file mode
	int size; // size already
	int bits;// actual bits read by read()
	}hdr;

char *ar_identifier = {"!<arch>"};
char ar_end_char[3] = {0x60, 0x0A, 0x00};
char new_line[2] = {0x0A,0x00};

void print_mode(int mode);
void print_time(time_t timestamp);
int is_archive(int fildes); // check if file is archive file
int write_data(int in_fildes,int out_fildes,hdr* file_hdr);
int find_header(int fildes, char* filename); 
int extract(int in_fildes, int out_fildes, int offset);
int t_method(int fildes);
int v_method(int fildes);
int q_method(char* filename, char* aname);
int x_method(int fildes, char* filename, char* aname);
hdr* read_header(int fildes);
hdr* write_header(int out_fildes,char* filename);


hdr* read_header(int fildes){ // read next header, no lseek size
	hdr* file_hdr;
	char buf_header[60];
	int bits;
	int size;
	int mode;
	time_t timestamp;
	int uid;
	int gid;
	char buf_size[10]; // at most 10 bits for size
	char buf_mode[9]; // at most 8 bits for mode
	char buf_uid[7]; // at most 6 bits for owner id
	char buf_gid[7]; // at most 6 bits for group ids
	char buf_timestamp[13]; // at most 12 bits for timestamp

	file_hdr = (hdr*) malloc(sizeof(hdr));
	bits = read(fildes,buf_header,60);
	file_hdr->bits = bits;

	/* Read the file identifier */
	for (int i=0;i<15;++i){
		file_hdr->identifier[i] = buf_header[i];
		if(file_hdr->identifier[i]=='/'){
			file_hdr->identifier[i] = '\0';
			break;
		}
	}

	/* Read the file's timestamp */
	for(int i=16;i<28;++i){
		buf_timestamp[i-16]=buf_header[i];
		if(buf_timestamp[i-16] == ' '){
			buf_timestamp[i-16] = '\0';
			break;
		}
	}
	timestamp = (int)strtol(buf_timestamp,(char**)NULL,10);
	file_hdr->timestamp = timestamp;

	/* Read the file's owner ID */
	for(int i=28;i<34;++i){
		buf_uid[i-28]=buf_header[i];
		if(buf_uid[i-28] == ' '){
			buf_uid[i-28] = '\0';
			break;
		}
	}
	uid = (int)strtol(buf_uid,(char**)NULL,6);
	file_hdr->uid = uid;

	

	/* Read the file's group ID */
	for(int i=34;i<40;++i){
		buf_gid[i-34]=buf_header[i];
		if(buf_gid[i-34] == ' '){
			buf_gid[i-34] = '\0';
			break;
		}
	}
	gid = (int)strtol(buf_gid,(char**)NULL,6);
	file_hdr->gid = gid;

	/*Read the file mode*/
	for (int i=40;i<48;++i){
		buf_mode[i-40]=buf_header[i];
		if(buf_mode[i-40] == ' '){
			buf_mode[i-40] = '\0';
			break;
		}
	}
	mode = (int)strtol(buf_mode, (char**)NULL, 8);
	file_hdr->mode = mode; // parse the mode char[] into int 

	/* Read the file size */
	for (int i=48;i<58;++i){
		buf_size[i-48]=buf_header[i];
		if(buf_size[i-48] == ' '){
    		buf_size[i-48] = '\0';
    		break;
    	}
	}
	size = (int)strtol(buf_size, (char**)NULL, 10);//dump non-digit part
	file_hdr->size = size; // parse the size char[] into int 
	//printf("identifier is %s, size is %d, bits is %d",file_hdr->identifier,file_hdr->size,file_hdr->bits);
	return file_hdr;

}

hdr* write_header(int out_fildes, char* filename)
{
	hdr* file_hdr;
	struct stat file_stat;
	char buf_header[60];
	int bits;
	int size;
	int mode;
	time_t timestamp;
	int uid;
	int gid;
	char buf_size[10]; // at most 10 bits for size
	char buf_mode[8]; // at most 8 bits for mode
	char buf_uid[6]; // at most 6 bits for owner id
	char buf_gid[6]; // at most 6 bits for group ids
	char buf_timestamp[12]; // at most 12 bits for timestamp
	int errnum;

	file_hdr = (hdr*) malloc(sizeof(hdr));
	if(stat(filename,&file_stat)==-1){
		errnum = errno;
		fprintf(stderr, "Error opening file: %s\n", strerror(errnum));
		exit(0);
	}

	sprintf(file_hdr->identifier, "%-16s", filename);
    file_hdr->identifier[strlen(filename)] = '/';

    sprintf(buf_timestamp, "%-12d", (int)file_stat.st_mtime);
    timestamp = (int)strtol(buf_timestamp,(char**)NULL,10);
    file_hdr->timestamp = timestamp;

    sprintf(buf_uid, "%-6d", (int)file_stat.st_uid);
    uid = (int)strtol(buf_uid,(char**)NULL,10);
    file_hdr->uid = uid;

    sprintf(buf_gid, "%-6d", (int)file_stat.st_gid);
    gid = (int)strtol(buf_gid,(char**)NULL,10);
    file_hdr->gid = gid;

    sprintf(buf_mode, "%-8o", file_stat.st_mode);//stored as octal
    mode = (int)strtol(buf_mode,(char**)NULL,10);
    file_hdr->mode = mode;

    sprintf(buf_size, "%-10d", (int)file_stat.st_size);
    size = (int)strtol(buf_size,(char**)NULL,10);
    file_hdr->size = size;

    if(out_fildes!=-1){
    	write(out_fildes, file_hdr->identifier, 16);
	    write(out_fildes, buf_timestamp, 12);
	    write(out_fildes, buf_uid, 6);
	    write(out_fildes, buf_gid, 6);
	    write(out_fildes, buf_mode, 8);
	    write(out_fildes, buf_size, 10);
	    write(out_fildes, ar_end_char, 2); //end char

    }
    
    return file_hdr;

}
int find_header(int fildes, char* filename)
{
	int offset;
	int file_size;
	hdr* file_hdr;
	file_hdr = read_header(fildes);

	while(file_hdr->bits == 60){
		//printf("Identifier is %s, filename is %s\n.",file_hdr->identifier,filename);
		if(strcmp(file_hdr->identifier,filename)==0){
    		offset =lseek(fildes, -60, SEEK_CUR);
    		return offset; //file found return position offset
    	}
		
    	if (file_hdr->size%2){
        	file_hdr->size += 1;
    	}
    	lseek(fildes, file_hdr->size, SEEK_CUR);
    	file_hdr = read_header(fildes);
    	if(strcmp(file_hdr->identifier,"\n")==0){
    		break;
    	} 	
	}
	return -1; //file not found
	
}
int write_data(int in_fildes, int out_fildes, hdr* file_hdr)
{
	char buf[1];
    int counter =0;
	while (counter < file_hdr->size) {
        read(in_fildes, buf, 1);
        write(out_fildes, buf, 1);
        counter+=1;
    }
    if (file_hdr->size%2 != 0){ //append extra '/n' for odd size
        write(out_fildes, new_line, 1);
    }
    close(out_fildes);
    close(in_fildes);
    return 1;
}
int extract(int in_fildes, int out_fildes, int offset)
{
	struct utimbuf file_time;
	hdr* file_hdr;
    int* buf_size; 
    int* buf_header;
    char buf[1];
    char* filename;
    int mode;
    time_t timestamp;

    if (offset == -1) {
        return -1;
    }

    buf_header = malloc(sizeof(61));
    buf_size = malloc(sizeof(file_hdr->size));// reserve buffer for header of file

    lseek(in_fildes, offset, SEEK_SET); // move pointer to file data
    file_hdr = read_header(in_fildes); // read in header

    mode = file_hdr->mode;

    timestamp = file_hdr->timestamp;
    //file_time.actime = timestamp; 
    file_time.modtime = timestamp;
    time(&file_time.actime);

    filename = file_hdr->identifier;

    //extract data up to file size back to file
    for (int i=0;i<file_hdr->size;i++){
    	read(in_fildes, buf, 1);
        write(out_fildes,buf,1);
    }

    fchmod(out_fildes, mode);//restore file permission
    utime(filename, &file_time);//restore file mtime and atime

    //Free buffer
    close(out_fildes);
    close(in_fildes);
    free(buf_size);
    free(buf_header);

    return 1;
}
void print_mode(int mode)
{
	printf( (S_ISDIR(mode)) ? "d" : "-");
    printf( (mode & S_IRUSR) ? "r" : "-");
    printf( (mode & S_IWUSR) ? "w" : "-");
    printf( (mode & S_IXUSR) ? "x" : "-");
    printf( (mode & S_IRGRP) ? "r" : "-");
    printf( (mode & S_IWGRP) ? "w" : "-");
    printf( (mode & S_IXGRP) ? "x" : "-");
    printf( (mode & S_IROTH) ? "r" : "-");
    printf( (mode & S_IWOTH) ? "w" : "-");
    printf( (mode & S_IXOTH) ? "x" : "-");
}
void print_time(time_t timestamp)
{	
	
	struct tm timestruct;
    char months[12][4] = {"Jan\0", "Feb\0", "Mar\0", "Apr\0", "May\0", "Jun\0",
                          "Jul\0", "Aug\0", "Sep\0", "Oct\0", "Nov\0", "Dec\0"};
    timestruct = *localtime(&timestamp);
    printf("%s %d %02d:%02d %d", months[timestruct.tm_mon], timestruct.tm_mday, timestruct.tm_hour,
                             timestruct.tm_min, timestruct.tm_year+1900);

}
int is_archive(int fildes)
{
	char arch_magic[8];
	char buf_magic[8];
	if (fildes == -1) {
	    return 0; //file doesn't exist
    }
	read(fildes,buf_magic,8);
	for (int i=0;i<7;i++){
		arch_magic[i] = buf_magic[i];
	}
	if(strcmp(arch_magic,"!<arch>")==0){//examine against the archive identifier
		return 1; //open archive file successuly
	}

	return -1; //file format is not archive
}

int t_method(int fildes)
{	
	hdr* file_hdr;
	int file_size;

	file_hdr = read_header(fildes);
	while(file_hdr->bits == 60){
		file_size = file_hdr->size;
    	if (file_size%2){
        	file_size += 1;
    	}
    	lseek(fildes, file_size, SEEK_CUR);
    	if(strcmp(file_hdr->identifier,"\n")==0){
    		break;
    	}
    	printf("%s\n",file_hdr->identifier);
    	file_hdr = read_header(fildes);
	}
	return 1;
	
}


int v_method(int fildes)
{	
	hdr* file_hdr;
	int file_mode;
    int file_size;
    time_t timestamp;
    

    file_hdr = read_header(fildes);
    while(file_hdr->bits == 60){
		file_size = file_hdr->size;
    	if (file_size%2){
        	file_size += 1;
    	}
    	lseek(fildes, file_size, SEEK_CUR);
    	if(strcmp(file_hdr->identifier,"\n")==0){
    		break;
    	}
    	print_mode(file_hdr->mode);
    	putchar(' ');
    	printf("%d/%d", file_hdr->uid, file_hdr->gid);
    	putchar(' ');
    	printf("%6d", file_hdr->size);
    	putchar(' ');

    	/* Print time stamp */
    	timestamp = file_hdr->timestamp;
    	print_time(timestamp);
    	putchar(' ');
    	printf("%s\n",file_hdr->identifier);
    	file_hdr = read_header(fildes);
	}
	return 1;
}


int q_method(char* archive_name, char* file_name)
{
    int in_fildes;
    int out_fildes;

    out_fildes = open(archive_name, O_WRONLY | O_APPEND);
    in_fildes = open(file_name, O_RDONLY);
    hdr* file_hdr = write_header(out_fildes,file_name); //create_header for file
    write_data(in_fildes,out_fildes,file_hdr);

    return 1;
}

int x_method(int in_fildes, char* archive_name, char* file_name)
{
    int out_fildes;
    int offset;
    hdr* header;

    offset = find_header(in_fildes,file_name); //head now should point to the match header

    if(offset==-1){
    	printf("Error extracting file %s.",file_name);
    	exit(0);
    	return -1;
    }
    out_fildes = open(file_name, O_WRONLY | O_TRUNC | O_CREAT, 0666);
    extract(in_fildes,out_fildes,offset);
	
    return 1;
}




int d_method(char* aname, char* name)
{
	printf("d_method is called with file name %s\n",name);
}

int A_method(char* aname)
{
	printf("A_method is called");
}

int main(int argc, char **argv)
{
	char* name;
	int key;
	int exist; //return value from is_archive
	int out_fildes;//use to create file

	char *aname = argv[2];
	int fildes = open(aname, O_RDONLY);


	exist = is_archive(fildes);
	if(argc == 4){
	  name = argv[3];
	}
	 /* Options:
     * -q = quickly append named files to archive 
     * -x = extract named files
     * -t = print a concise table of contents of the archive
     * -v = print a verbose table of contents of the archive
     * -d = delete named files from archive
     * -A = quickly append all "ordinary" files in the current directory that have been 
     		modified within the last two hours (except the archive itself.)
     */
    while ((key = getopt(argc, argv, "qxtvdA")) != -1){ //checking EOF
    	switch (key) {
            case 'q':
            	if (exist == 0){
            		/*Archive file doesn't exist, create one*/
            		out_fildes = open(aname, O_WRONLY | O_CREAT, 0666);
        			write(out_fildes, "!<arch>\n", 8);
        			close(out_fildes);
            	}
                q_method(aname,name); 
                break;
            case 'x':
            	if (exist ==0){
            		fprintf(stderr, "Archive %s doesn't exist\n",aname);
            	}
            	//find_header(fildes,name);
            	x_method(fildes,aname,name);
            	break;
            case 't':
                t_method(fildes); 
                break;
            case 'v':
            	v_method(fildes); 
            	break;
            case 'd':
                d_method(aname,name);
                 break;
            case 'A':
            	A_method(aname); 
            	break;
            default:
                printf("Usage: %s [-qxtvdA] [afile...] [file...]\n", argv[0]);
                exit(0);
        }
    }
    exit(0);
}