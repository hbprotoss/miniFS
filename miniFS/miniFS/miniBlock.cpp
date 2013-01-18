#include <cstdio>

#include "miniBlock.h"
#include "miniFile.h"

extern FILE_SYSTEM_HEADER *g_file_system_header;	// �ļ�ϵͳͷ���棨from miniFile.cpp��
FILE *g_storage;					// OS�ϵ������ļ�

ERROR_CODE miniReadBlock(int block_number, int buffer_size, void *buf)
{
	// ��������С���ٱ�֤һ����Ĵ�С
	if(buffer_size < BLOCK_SIZE)
		return ERR_BUFFER_OVERFLOW;

	// �ƶ��ļ�ָ��
	_fseeki64(g_storage, block_number * BLOCK_SIZE, SEEK_SET);
	// ��ȡ
	if(fread(buf, BLOCK_SIZE, 1, g_storage) == 1)
		return ERR_SUCCESS;
	else
		if(feof(g_storage))
			return ERR_SUCCESS;
		else
			return ERR_OTHER;
}

ERROR_CODE miniWriteBlock(int block_number, int buffer_size, void *buf)
{
	// д���������ܴ���һ����Ĵ�С
	if(buffer_size > BLOCK_SIZE)
		return ERR_BUFFER_OVERFLOW;

	// �ƶ��ļ�ָ��
	_fseeki64(g_storage, block_number * BLOCK_SIZE, SEEK_SET);
	// д��
	if(fwrite(buf, BLOCK_SIZE, 1, g_storage) == 1)
	{
		return ERR_SUCCESS;
	}
	else
		return ERR_OTHER;
}