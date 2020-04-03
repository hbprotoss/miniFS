#include <cstdio>

#include "miniBlock.h"
#include "miniFile.h"

extern FILE_SYSTEM_HEADER *g_file_system_header;    // 文件系统头缓存（from miniFile.cpp）
FILE *g_storage;                    // OS上的物理文件

ERROR_CODE miniReadBlock(int block_number, int buffer_size, void *buf) {
	// 缓冲区大小至少保证一个块的大小
	if (buffer_size < BLOCK_SIZE)
		return ERR_BUFFER_OVERFLOW;

	// 移动文件指针
	fseek(g_storage, block_number * BLOCK_SIZE, SEEK_SET);
	// 读取
	if (fread(buf, BLOCK_SIZE, 1, g_storage) == 1)
		return ERR_SUCCESS;
	else if (feof(g_storage))
		return ERR_SUCCESS;
	else
		return ERR_OTHER;
}

ERROR_CODE miniWriteBlock(int block_number, int buffer_size, void *buf) {
	// 写入数量不能大于一个块的大小
	if (buffer_size > BLOCK_SIZE)
		return ERR_BUFFER_OVERFLOW;

	// 移动文件指针
	fseek(g_storage, block_number * BLOCK_SIZE, SEEK_SET);
	// 写入
	if (fwrite(buf, BLOCK_SIZE, 1, g_storage) == 1) {
		return ERR_SUCCESS;
	} else
		return ERR_OTHER;
}