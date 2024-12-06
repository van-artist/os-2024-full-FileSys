#include <stdio.h>
#include <string.h>
#include "filesys.h"

void format()
{
	MemoryINode *inode;
	DirectoryEntry dir_buf[BLOCKSIZ / (DIRSIZ + 4)];
	UserPassword passwd[32];
	unsigned int block_buf[BLOCKSIZ / sizeof(int)];

	// 初始化硬盘
	memset(disk, 0x00, ((DINODEBLK + FILEBLK + 2) * BLOCKSIZ));

	/* 0.initialize the passwd */
	passwd[0].user_id = 2116;
	passwd[0].group_id = 03;
	strcpy(passwd[0].password, "dddd");

	passwd[1].user_id = 2117;
	passwd[1].group_id = 03;
	strcpy(passwd[1].password, "bbbb");

	passwd[2].user_id = 2118;
	passwd[2].group_id = 04;
	strcpy(passwd[2].password, "abcd");

	passwd[3].user_id = 2119;
	passwd[3].group_id = 04;
	strcpy(passwd[3].password, "cccc");

	passwd[4].user_id = 2120;
	passwd[4].group_id = 05;
	strcpy(passwd[4].password, "eeee");

	/* 1.creat the main directory and its sub dir etc and the file password */

	inode = iget(0);			/* 0 empty dinode id*/
	inode->reference_count = 1; //??空i节点有何用????
	inode->mode = DIEMPTY;
	iput(inode);

	inode = iget(1); /* 1 main dir id*/
	inode->reference_count = 1;
	inode->mode = DEFAULTMODE | DIDIR;
	inode->file_size = 3 * (DIRSIZ + 4);
	inode->block_addresses[0] = 0; /*block 0# is used by the main directory*/

	strcpy(dir_buf[0].name, "..");
	dir_buf[0].inode_number = 1;
	strcpy(dir_buf[1].name, ".");
	dir_buf[1].inode_number = 1;
	strcpy(dir_buf[2].name, "etc");
	dir_buf[2].inode_number = 2;

	memcpy(disk + DATASTART, &dir_buf, 3 * (DIRSIZ + 4));
	iput(inode);

	inode = iget(2); /* 2  etc dir id */
	inode->reference_count = 1;
	inode->mode = DEFAULTMODE | DIDIR;
	inode->file_size = 3 * (DIRSIZ + 4);
	inode->block_addresses[0] = 1; /*block 1# is used by the etc directory*/

	strcpy(dir_buf[0].name, "..");
	dir_buf[0].inode_number = 1;
	strcpy(dir_buf[1].name, ".");
	dir_buf[1].inode_number = 2;
	strcpy(dir_buf[2].name, "password");
	dir_buf[2].inode_number = 3;

	memcpy(disk + DATASTART + BLOCKSIZ * 1, dir_buf, 3 * (DIRSIZ + 4));
	iput(inode);

	inode = iget(3); /* 3  password id */
	inode->reference_count = 1;
	inode->mode = DEFAULTMODE | DIFILE;
	inode->file_size = BLOCKSIZ;
	inode->block_addresses[0] = 2; /*block 2# is used by the password file*/

	for (int i = 5; i < PWDNUM; i++)
	{
		passwd[i].user_id = 0;
		passwd[i].group_id = 0;
		strcpy(passwd[i].password, "            "); // PWDSIZ " "
	}

	memcpy(pwd, passwd, 32 * sizeof(struct UserPassword));
	memcpy(disk + DATASTART + BLOCKSIZ * 2, passwd, BLOCKSIZ);
	iput(inode);

	/*2. initialize the superblock */

	fileSystem.inode_block_count = DINODEBLK;
	fileSystem.data_block_count = FILEBLK;

	fileSystem.free_inode_count = DINODEBLK * BLOCKSIZ / DINODESIZ - 4;
	fileSystem.free_block_count = FILEBLK - 3;

	for (int i = 0; i < NICINOD; i++)
	{
		/* begin with 4,    0,1,2,3, is used by main,etc,password */
		fileSystem.free_inodes[i] = 4 + i;
	}

	fileSystem.free_inode_pointer = 0;
	fileSystem.last_allocated_inode = NICINOD + 4;

	block_buf[NICFREE - 1] = FILEBLK + 1; /*FILEBLK+1 is a flag of end*/
	for (int i = 0; i < NICFREE - 1; i++)
	{
		block_buf[NICFREE - 2 - i] = FILEBLK - i - 1; // 从最后一个数据块开始分配??????
	}

	memcpy(disk + DATASTART + BLOCKSIZ * (FILEBLK - NICFREE), block_buf, BLOCKSIZ);
	int i, j;
	for (i = FILEBLK - 2 * NICFREE + 1; i > 2; i -= NICFREE)
	{
		for (j = 0; j < NICFREE; j++)
		{
			block_buf[j] = i + j;
		}
		memcpy(disk + DATASTART + BLOCKSIZ * (i - 1), block_buf, BLOCKSIZ);
	}
	i += NICFREE;
	j = 1;
	for (; i > 3; i--)
	{
		fileSystem.free_blocks[NICFREE - j] = i - 1;
		j++;
	}

	fileSystem.free_block_pointer = NICFREE - j + 1;
	memcpy(disk + BLOCKSIZ, &fileSystem, sizeof(FileSystem));
	return;
}
