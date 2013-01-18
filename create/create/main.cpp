#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <Windows.h>

int main(int argc, char *argv[])
{
	HANDLE hFile = CreateFileA(argv[1], GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		fprintf(stdout, "Invalid file\n");
		return 1;
	}

	// 默认大小1G
	int size = 1;
	if(argc == 3)
	{
		if(sscanf(argv[2], "%d", &size) <= 0 ||
			(size <=0 || size > 1024))
		{
			fprintf(stdout, "Invalid size\n");
			return 1;
		}
	}
	__int64 size_GB = (__int64)size << 30;
	LARGE_INTEGER li = *(LARGE_INTEGER*)&size_GB;
	SetFilePointerEx(hFile, li, NULL, FILE_BEGIN);
	SetEndOfFile(hFile);

	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	CloseHandle(hFile);

	// 设置文件系统标识符
	hFile = CreateFileA(argv[1], GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD dwWritten = 0;
	if(WriteFile(hFile, "Nerv", strlen("Nerv"), &dwWritten, NULL) == 0)
	{
		printf("0x%08x\n", GetLastError());
	}


	CloseHandle(hFile);

	
	return 0;
}