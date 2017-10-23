#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct _hdr { // ar header data structure to store file infomration
	char identifier[16]; // length 16 identifier
	int timestamp[12]; // length 12 identifier
	int uid[6]; // length 6 owner id
	int gid[6];// length 6 group id
	int size[10];// length 10 file size
	int filedes;// file descriptor associated (used for archive)
	}hdr;

hdr* ar_hdr;
char *ar_identifier = {"!<arch>"};

hdr* read_header(int fildes);
int is_archive(hdr* header); // check if file is archive file
int print_file(int fildes);

hdr* read_header(int fildes){
	hdr* file_hdr;
	char buf_header[60];
   	char buf_identifier[16]; 
	int buf_timestamp[12];
	int buf_uid[6];
	int buf_gid[6];
	int buf_size[10];

	file_hdr = (hdr*) malloc(sizeof(hdr));
	int bits = read(fildes,buf_header,60);
	/* Read the file identifier */
	for (int i=0;i<15;i++){
		file_hdr->identifier[i] = buf_header[i];
	}
	printf("header is %d\n",bits);
	return file_hdr;

}

int is_archive(hdr* header)
{
	char arch_magic[8];
	for (int i=0;i<7;i++){
		arch_magic[i] = header->identifier[i];
	}
	if(strcmp(arch_magic,"!<arch>")==0){
		return 1;
	}

	return 0;
}

int print_file(int fildes)
{
	char filename[16];
	hdr* buf_header = read_header(fildes);
	for(int i=0;i<15;i++){
		filename[i] = buf_header->identifier[i];
	}
	printf("File name: %s\n",filename);
	return 0;

}
int q_method(char* aname, char* name)
{
	printf("q_method is called with file name %s\n",name);
}

int x_method(char* aname, char* name)
{
	printf("x_method is called with file name %s\n",name);
}

int t_method(int fildes)
{
	//printf("t_method is called with afile name %s\n",aname);
	return print_file(fildes);

}

int v_method(char* aname)
{
	printf("v_method is called");
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

	int fildes = open(aname, O_RDONLY);
	if (fildes == -1) {
	    printf("File %s doesn't exist.\n",aname);
	    exit(0);
    }

	ar_hdr = (hdr*) malloc(sizeof(hdr));
	ar_hdr = read_header(fildes);

	if(is_archive(ar_hdr)==0){
		printf("File not valid ...");
		exit(0);
	}
	else{
		printf("Valid archive ...");
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
    while ((key = getopt(argc, argv, "qxtvdA")) != -1){ //this function is obtained from C documentation
    	switch (key) {
            case 'q':
                q_method(aname,name); break;
            case 'x':
            	x_method(aname,name); break;
            case 't':
                t_method(fildes); break;
            case 'v':
            	v_method(aname); break;
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

