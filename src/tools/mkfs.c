
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

#define I_MAP_SLOTS (8)
#define Z_MAP_SLOTS (8)
#define BLOCK_SIZE  (1024)

static unsigned char inode_map[I_MAP_SLOTS * BLOCK_SIZE];
static unsigned char zone_map[ Z_MAP_SLOTS * BLOCK_SIZE];

#define UNMARK(head, bits)  ((head)[bits/8]) = (((head)[bits/8])&(~(1<<(bits%8))))

#define ROOT_INO_STRING "\001\000"
#define BAD_INO_STRING "\002\000"

static unsigned char root_block[BLOCK_SIZE] =
ROOT_INO_STRING ".\0\0\0\0\0\0\0\0\0\0\0\0\0"
ROOT_INO_STRING "..\0\0\0\0\0\0\0\0\0\0\0\0"
BAD_INO_STRING  ".badblocks\0\0\0\0";

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

void make_map(unsigned char * pbyMap_out, const int iSize_in, const int iStart_in, const int iCount_in, const int iBlocks_in) 
{
	int i = 0;
	int count = 0;
	unsigned char map = 0;

#ifdef MKFS_DEBUG
	fprintf(stderr, "[make_map][iSize_in ][%d] \r\n", iSize_in);
	fprintf(stderr, "[make_map][iStart_in][%d] \r\n", iStart_in);
	fprintf(stderr, "[make_map][iCount_in][%d] \r\n", iCount_in);
#endif // MKFS_DEBUG
	
	memset(pbyMap_out, 0xFF, iSize_in);
	for (i = iStart_in; i < iCount_in; ++i) {
		UNMARK(pbyMap_out, i);
	}
	
	// check the result
	for (i = iStart_in/8; i < iSize_in; ++i) {
		if (0xFF == pbyMap_out[i]) {
			break;
		}
	}
#ifdef MKFS_DEBUG
	fprintf(stderr, "[make_map][%d] \r\n", i);
#endif // MKFS_DEBUG
	count = i * 8;
	map = pbyMap_out[i - 1];
	for (i = 0; i < 8; ++i) {
		if ((1 << i) & map) {
			count--;
		}
	}
	if (count != iCount_in) {
		fprintf(stderr, "[make_map][ERROR][%d][%d] \r\n", iCount_in, count);
	}
	for (i = count/8; i < iBlocks_in; ++i) {
		if (0xFF != pbyMap_out[i]) {
			fprintf(stderr, "[make_map][ERROR][%d][0x%X] \r\n", i, pbyMap_out[i]);
		}
	}
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
	unsigned char buf[1024];
	int i = 0;
	int blocks = 0;
	unsigned char * inode_buffer = 0;
		
	// the MBR or boot section
	memset(buf, 0x00, 1024);
	i = write(1, buf, 1024);
	if (i != 1024) {
		die("write failed \r\n");
	}
	++blocks;
	
	// super block
	make_super(1024, 512);
	memcpy(buf, &Super, sizeof(Super));
	i = write(1, buf, 1024);
	if (i != 1024) {
		die("write failed \r\n");
	}
	++blocks;
	
	// inode map
	make_map(inode_map, I_MAP_SLOTS * BLOCK_SIZE, 1, Super.s_ninodes, Super.s_imap_blocks);
	
	// zone map
	make_map(zone_map, Z_MAP_SLOTS * BLOCK_SIZE, Super.s_firstdatazone, Super.s_nzones, Super.s_zmap_blocks);
	
	inode_buffer = malloc(INODE_BLOCKS(512)*BLOCK_SIZE);
	if (0 == inode_buffer) {
		die("out of memory \r\n");
	}
	
	
	
	i = write(1, inode_map, Super.s_imap_blocks * BLOCK_SIZE);
	if (i != Super.s_imap_blocks * BLOCK_SIZE) {
		die("write failed \r\n");
	}
	blocks += Super.s_imap_blocks;
	i = write(1, zone_map, Super.s_zmap_blocks * BLOCK_SIZE);
	if (i != Super.s_zmap_blocks * BLOCK_SIZE) {
		die("write failed \r\n");
	}
	blocks += Super.s_zmap_blocks;
	
	// fill the reset
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

