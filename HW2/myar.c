#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

typedef struct _hdr { // ar header data structure to store file infomration
	char identifier[16]; // length 16 identifier
	time_t timestamp; // timestamp
	int uid; // owner id
	int gid;// group id
	int mode; // file mode
	int size; // size already
	int bits;// actual bits read by read()
	}hdr;

hdr* ar_hdr;
char *ar_identifier = {"!<arch>"};

void print_mode(int mode);
void print_time(time_t timestamp);
int is_archive(char* identifier); // check if file is archive file
int t_method(int fildes);
int v_method(int fildes);
hdr* read_header(int fildes);

hdr* read_header(int fildes){
	hdr* file_hdr;
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

	file_hdr = (hdr*) malloc(sizeof(hdr));
	bits = read(fildes,buf_header,60);
	file_hdr->bits = bits;

	/* Read the file identifier */
	for (int i=0;i<15;i++){
		file_hdr->identifier[i] = buf_header[i];
		if(file_hdr->identifier[i]=='/'){
			file_hdr->identifier[i] = '\0';
			break;
		}
	}

	/* Read the file's timestamp */
	for(int i=16;i<28;i++){
		buf_timestamp[i-16]=buf_header[i];
		if(buf_timestamp[i-16] == ' '){
			buf_timestamp[i-16] = '\0';
			break;
		}
	}
	timestamp = (int)strtol(buf_timestamp,(char**)NULL,12);
	file_hdr->timestamp = timestamp;

	/* Read the file's owner ID */
	for(int i=28;i<34;i++){
		buf_uid[i-28]=buf_header[i];
		if(buf_uid[i-28] == ' '){
			buf_uid[i-28] = '\0';
			break;
		}
	}
	uid = (int)strtol(buf_uid,(char**)NULL,6);
	file_hdr->uid = uid;

	

	/* Read the file's group ID */
	for(int i=34;i<40;i++){
		buf_gid[i-34]=buf_header[i];
		if(buf_gid[i-34] == ' '){
			buf_gid[i-34] = '\0';
			break;
		}
	}
	gid = (int)strtol(buf_gid,(char**)NULL,6);
	file_hdr->gid = gid;

	/*Read the file mode*/
	for (int i=40;i<48;i++){
		buf_mode[i-40]=buf_header[i];
		if(buf_mode[i-40] == ' '){
			buf_mode[i-40] = '\0';
			break;
		}
	}
	mode = (int)strtol(buf_mode, (char**)NULL, 8);
	file_hdr->mode = mode; // parse the mode char[] into int 

	/* Read the file size */
	for (int i=48;i<58;i++){
		buf_size[i-48]=buf_header[i];
		if(buf_size[i-48] == ' '){
    		buf_size[i-48] = '\0';
    		break;
    	}
	}
	size = (int)strtol(buf_size, (char**)NULL, 10);
	file_hdr->size = size; // parse the size char[] into int 
	//printf("identifier is %s, size is %d, bits is %d",file_hdr->identifier,file_hdr->size,file_hdr->bits);
	return file_hdr;

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
int is_archive(char* identifier)
{
	char arch_magic[8];
	for (int i=0;i<7;i++){
		arch_magic[i] = identifier[i];
	}
	if(strcmp(arch_magic,"!<arch>")==0){//examine against the archive identifier
		return 1;
	}

	return 0;
}

int t_method(int fildes)
{	
	hdr* file_hdr;
	int file_size;
	int counter = 0;
	char filename[16];
	char buf[60];

	file_hdr = read_header(fildes);
	while(file_hdr->bits == 60){
		file_size = file_hdr->size;
    	if (file_size%2){
        	file_size += 1;
    	}
    	lseek(fildes, file_size, SEEK_CUR);
    	file_hdr = read_header(fildes);
    	if(strcmp(file_hdr->identifier,"\n")==0){
    		break;
    	}
    	printf("%s\n",file_hdr->identifier);
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


int q_method(char* aname, char* name)
{
	printf("q_method is called with file name %s\n",name);
}

int x_method(char* aname, char* name)
{
	printf("x_method is called with file name %s\n",name);
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
	char *aname = argv[2];
	char identifier[8];
	int fildes = open(aname, O_RDONLY);

	if (fildes == -1) {
	    printf("File %s doesn't exist.\n",aname);
	    exit(0);
    }
	read(fildes,identifier,8);
	if(is_archive(identifier)==0){
		printf("File not valid ...\n");
		exit(0);
	}

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
                q_method(aname,name); break;
            case 'x':
            	x_method(aname,name); break;
            case 't':
                t_method(fildes); 
                break;
            case 'v':
            	v_method(fildes); 
            	break;
            case 'd':
                d_method(aname,name); break;
            case 'A':
            	A_method(aname); break;
            default:
                printf("Usage: %s [-qxtvdA] [afile...] [file...]\n", argv[0]);
                exit(0);
        }
    }
    exit(0);
}