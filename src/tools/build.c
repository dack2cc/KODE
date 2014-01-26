#include <stdio.h>	/* fprintf */
#include <stdlib.h>	/* contains exit */
#include <sys/types.h>	/* unistd.h needs this */
#include <sys/stat.h>
#include <unistd.h>	/* contains read/write */
#include <fcntl.h>
#include <string.h>

#define DEFAULT_MAJOR_ROOT 3
#define DEFAULT_MINOR_ROOT 1

/* max nr of sectors of setup: don't change unless you also change
 * bootsect etc */
#define SETUP_SECTS 4

#define STRINGIFY(x) #x

#define HDISK_SECTS (153*4*17)

void die(char * str)
{
	fprintf(stderr,"%s\n",str);
	exit(1);
}

void usage(void)
{
	die("Usage: build boot setup system [> image]");
}

int main(int argc, char ** argv)
{
	int i,c,id;
	char buf[1024];
	char major_root, minor_root;
	//struct stat sb;
	int sectors = 0;
	
	if ((argc != 4) && (argc != 5))
	    usage();
	
	// judge the boot device
	major_root = DEFAULT_MAJOR_ROOT;
	minor_root = DEFAULT_MINOR_ROOT;
	if (0 == strcmp(argv[1], "FLOPPY")) {
		major_root = 0;
		minor_root = 0;
	} else {
		//if (stat(argv[1], &sb)) {
		//	perror(argv[1]);
		//	die("Couldn't stat root device.");
		//}
		//major_root = major(sb.st_rdev);
		//minor_root = minor(sb.st_rdev);
	}
	fprintf(stderr, "Root device is (%d, %d)\n", major_root, minor_root);
	if ((major_root != 2) && (major_root != 3) &&
	    (major_root != 0)) {
		fprintf(stderr, "Illegal root device (major = %d)\n",
			major_root);
		die("Bad root device --- major #");
	}
	
	// copy the boot
	for (i=0;i<sizeof buf; i++) buf[i]=0;
	if ((id=open(argv[2],O_RDONLY,0))<0)
		die("Unable to open 'boot'");
	i=read(id,buf,sizeof buf);
	fprintf(stderr,"Boot sector %d bytes.\n",i);
	if (i!=512)
		die("Boot block may not exceed 512 bytes");
	if ((*(unsigned short *)(buf+510)) != 0xAA55)
		die("Boot block hasn't got boot flag (0xAA55)");
	buf[508] = (char) minor_root;
	buf[509] = (char) major_root;
	i=write(1,buf,512);
	if (i!=512)
		die("Write call failed");
	close (id);

	sectors++;
	
	// copy the setup
	if ((id=open(argv[3],O_RDONLY,0))<0)
		die("Unable to open 'setup'");
	for (i=0 ; (c=read(id,buf,sizeof buf))>0 ; i+=c )
		if (write(1,buf,c)!=c)
			die("Write call failed");
	close (id);
	
	if (i > SETUP_SECTS*512)
		die("Setup exceeds " STRINGIFY(SETUP_SECTS)
			" sectors - rewrite build/boot/setup");
	fprintf(stderr,"Setup is %d bytes.\n",i);
	for (c=0 ; c<sizeof(buf) ; c++)
		buf[c] = '\0';
	while (i<SETUP_SECTS*512) {
		c = SETUP_SECTS*512-i;
		if (c > sizeof(buf))
			c = sizeof(buf);
		if (write(1,buf,c) != c)
			die("Write call failed");
		i += c;
	}
	
	sectors+=SETUP_SECTS;
	
	// copy the system
	if (argc == 5) {
		if ((id=open(argv[4],O_RDONLY,0))<0)
			die("Unable to open 'system'");
		for (i=0 ; (c=read(id,buf,sizeof buf))>0 ; i+=c ) {
			if (write(1,buf,c)!=c)
				die("Write call failed");
			if (c > 512) 
			    sectors+=2;
			else 
			    sectors++;
		}
		close(id);
		
		fprintf(stderr,"System %d bytes.\n",i);
		for (c=0 ; c<sizeof(buf) ; c++)
			buf[c] = '\0';
		if (i > 512) {
			i = i%512;
		}
		while (i<512) {
			c = 512-i;
			if (c > sizeof(buf))
				c = sizeof(buf);
			if (write(1,buf,c) != c)
				die("Write call failed");
			i += c;
		}
	} 
	
	if (0 == strcmp(argv[1], "HDISK")) {
		while (sectors < HDISK_SECTS) {
			if (write(1,buf,512) != 512)
				die("Write call failed");
			sectors++;
		}
	}
	
	return(0);
}

