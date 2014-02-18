
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

//#define MKFS_DEBUG

struct d_inode {
	unsigned short i_mode;
	unsigned short i_uid;
	unsigned long i_size;
	unsigned long i_time;
	unsigned char i_gid;
	unsigned char i_nlinks;
	unsigned short i_zone[9];
};

struct super_block {
	unsigned short s_ninodes;
	unsigned short s_nzones;
	unsigned short s_imap_blocks;
	unsigned short s_zmap_blocks;
	unsigned short s_firstdatazone;
	unsigned short s_log_zone_size;
	unsigned long s_max_size;
	unsigned short s_magic;
};

static struct super_block Super;

#define SUPER_MAGIC                 (0x137F)
#define UPPER(size,n)               ((size+((n)-1))/(n))
#define BITS_PER_BLOCK(blocksize)   (blocksize << 3)
#define INODES_PER_BLOCK(blocksize) ((blocksize)/(sizeof (struct d_inode)))
#define INODE_BLOCKS(blocksize)     UPPER((Super.s_ninodes), INODES_PER_BLOCK(blocksize))
#define NORM_FIRSTZONE(blocksize)   (2 + Super.s_imap_blocks + Super.s_zmap_blocks + INODE_BLOCKS(blocksize))

void make_super(const unsigned int uiBlockSize_in, const unsigned int uiBlockCount_in)
{
	Super.s_ninodes = uiBlockCount_in / 3;
	Super.s_nzones  = uiBlockCount_in;
	Super.s_imap_blocks = UPPER(uiBlockCount_in, BITS_PER_BLOCK(uiBlockSize_in));
	Super.s_zmap_blocks = 0;
	Super.s_firstdatazone = 0;
	Super.s_log_zone_size = 0;
	Super.s_max_size = (7 + 512 + 512*512)*1024;
	Super.s_magic = SUPER_MAGIC;
	
	while (Super.s_zmap_blocks != UPPER(uiBlockCount_in - NORM_FIRSTZONE(uiBlockSize_in), BITS_PER_BLOCK(uiBlockSize_in))) {
		Super.s_zmap_blocks = UPPER(uiBlockCount_in - NORM_FIRSTZONE(uiBlockSize_in), BITS_PER_BLOCK(uiBlockSize_in));
	}
	Super.s_firstdatazone = NORM_FIRSTZONE(uiBlockSize_in);
	
#ifdef MKFS_DEBUG
	fprintf(stderr, "[s_magic   ][0x%x] \r\n", Super.s_magic);
	fprintf(stderr, "[s_ninodes ][%d] \r\n", Super.s_ninodes);
	fprintf(stderr, "[s_nzones  ][%d] \r\n", Super.s_nzones);
	fprintf(stderr, "[s_max_size][%ld] \r\n", Super.s_max_size);
	fprintf(stderr, "[s_imap_blocks][%d] \r\n", Super.s_imap_blocks);
	fprintf(stderr, "[s_zmap_blocks][%d] \r\n", Super.s_zmap_blocks);
	fprintf(stderr, "[s_firstdatazone][%d][%d] \r\n", Super.s_firstdatazone, NORM_FIRSTZONE(uiBlockSize_in));
	fprintf(stderr, "[s_log_zone_size][%d] \r\n", Super.s_log_zone_size);
#endif // MKFS_DEBUG
}

void die(char * str)
{
	fprintf(stderr,"%s\n",str);
	exit(1);
}

void usage(void)
{
	die("Usage: mkfs [> image]");
}


int main(int argc, char** argv)
{
	char buf[1024];
	int i = 0;
	int blocks = 0;
	//fprintf(stderr, "mkfs ... \r\n");
	
	memset(buf, 0x00, 1024);
	
	make_super(1024, 512);
	memcpy(buf, &Super, sizeof(Super));
	i = write(1, buf, 1024);
	if (i != 1024) {
		die("write failed \r\n");
	}
	++blocks;
	
	memset(buf, 0x00, 1024);
	while (blocks < 512) {
		i = write(1, buf, 1024);
		if (i != 1024) {
			die("write failed \r\n");
		}
		++blocks;		
	}
	
	return 0;
}

