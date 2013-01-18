#include <cstdio>

#include "miniBlock.h"
#include "miniFile.h"

extern FILE_SYSTEM_HEADER *g_file_system_header;	// 文件系统头缓存
extern FILE *g_storage;					// OS上的物理文件

int main(int argc, char *argv[])
{
	g_file_system_header = new FILE_SYSTEM_HEADER;
	g_file_system_header->amount_of_block = 1 * GB / (4 * KB);

	FILE *fp = fopen("F:\\MyProjects\\Projects\\miniFS\\test", "rb+");
	g_storage = fp;

	// 功能测试
	unsigned char buf[BLOCK_SIZE];
	int *p = (int*)buf;
	for(int i = 0; i < BLOCK_SIZE / sizeof(int); i++)
	{
		*p = i;
		p++;
	}
	miniWriteBlock(0, sizeof(buf) * sizeof(unsigned char), buf);
	miniReadBlock(0, sizeof(buf) * sizeof(unsigned char), buf);
	
	//p = (int*)buf;
	//for(int i = 0; i < BLOCK_SIZE / sizeof(int); i++)
	//{
	//	printf("%d, ", *p);
	//	p++;
	//}

	// 异常测试
	if(miniWriteBlock(0x10000000, sizeof(buf) * sizeof(unsigned char), buf) == ERR_OUT_OF_RANGE)
		printf("OUT_OF_RANGE\n");
	if(miniWriteBlock(0, 1, buf) == ERR_BUFFER_OVERFLOW)
		printf("BUFFER_OVERFLOW\n");

	fclose(fp);
	return 0;
}