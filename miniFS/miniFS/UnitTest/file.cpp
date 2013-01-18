#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "miniCommandParsing.h"
#include "miniAPP.h"
#include "miniBlock.h"

extern FILE_SYSTEM_HEADER *g_file_system_header;		// 文件系统头缓存

int main(int argc, char *argv[])
{
	mount(argv[1]);
	char ans = 'y';
	printf("format?(Y/n) ");
	scanf("%c", &ans);
	getchar();
	if(ans == 'y' || ans == 'Y')
	{
		miniFormat();
		printf("Format done!\n");
	}

	char buf[BLOCK_SIZE];
	char buf_read[BLOCK_SIZE];

	FILE_DESCRIPTOR *fd;
	FILE *fp = fopen("F:\\MyProjects\\Projects\\小学期\\miniFS\\test.txt", "rb");

	int size = 0;
	__int64 res = 0;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	fread(buf, size, 1, fp);

	while(true)
	{
		printf("w/r/d ? ");
		scanf("%c", &ans);
		if(ans == 'q')
			break;
		getchar();

		miniCreateFile("/test.txt", size, "w", &fd);
		switch(ans)
		{
		case 'w':
			miniSeekFile(fd, 0);
			miniWriteFile(fd, size, BLOCK_SIZE, buf, &res);
			break;
		case 'r':
			miniSeekFile(fd, 0);
			miniReadFile(fd, size, BLOCK_SIZE, buf_read, &res);
			break;
		case 'd':
			miniDeleteFile(fd);
			break;
		default:
			;
		}
		miniCloseFile(fd);
	}
	
	fclose(fp);
	return 0;
}