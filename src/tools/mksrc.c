#include <stdio.h>	/* fprintf */
#include <stdlib.h>	/* contains exit */
#include <sys/types.h>	/* unistd.h needs this */
#include <sys/stat.h>
#include <unistd.h>	/* contains read/write */
#include <fcntl.h>
#include <string.h>


void die(char * str)
{
	fprintf(stderr,"%s\n",str);
	exit(1);
}

void usage(void)
{
	die("Usage: mksrc binary [> src.c]");
}

int main(int argc, char** argv)
{
	int i,c,id,len;
	unsigned char bufIn[1024];
	char bufOut[1024];

	if (argc < 2) {
		usage();
	}
	
	if ((id=open(argv[1],O_RDONLY,0))<0) {
		die("Unable to open binary file");
	}
	
	while ((c=read(id, bufIn, sizeof bufIn)) > 0) {
		for (i = 0; i < c; i += 8) {
			sprintf(bufOut, "0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, \r\n", 
				    bufIn[i + 0], bufIn[i + 1], bufIn[i + 2], bufIn[i + 3], bufIn[i + 4], bufIn[i + 5], bufIn[i + 6], bufIn[i + 7]);
			len = strlen(bufOut);
			if (len != write(1,bufOut,len)) {
				die("write failed");
			}
		}
	}
	
	
	return 0;
}
