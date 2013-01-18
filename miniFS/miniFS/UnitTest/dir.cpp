#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <Windows.h>

#include "miniCommandParsing.h"
#include "miniAPP.h"
#include "miniFile.h"
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


	while(true)
	{
		char dir[100];
		DIRECTORY_DESCRIPTOR *dd;
		int size = 0;
		DIRECTORY_ENTRY *entry;
		char tmp;

		printf("w/r/d ? ");
		ans = getchar();
		tmp = getchar();
		if(ans == 'q')
			goto end;
		printf("directory name? ");
		scanf("%s", dir);
		tmp = getchar();

		switch(ans)
		{
		case 'r':
			miniCreateDirectory(dir, "r", &dd);
			if(miniReadDirectory(dd, &size, NULL) == ERR_BUFFER_OVERFLOW)
			{
				entry = (DIRECTORY_ENTRY*)calloc(1, size);
				miniReadDirectory(dd, &size, entry);
				for(int i = 0; i < size; i++)
					printf("%s\n", entry[i].name);
				free(entry);
			}
			break;
		case 'w':
			miniCreateDirectory(dir, "w", &dd);
			break;
		case 'd':
			miniCreateDirectory(dir, "r", &dd);
			miniDeleteDirectory(dd);
			break;

		}
		miniCloseDirectory(dd);
	}
end:
	miniExitSystem();
	return 0;
}