#include <cstdio>
#include <cstdlib>
#include <string>
#include <climits>
#include <cassert>
#include <cmath>

#include "miniFile.h"
#include "miniBlock.h"

FILE_SYSTEM_HEADER *g_file_system_header;		// �ļ�ϵͳͷ����
Bitmap *g_bitmap;					// λͼ����
extern FILE *g_storage;					// ��������ļ�(from miniBlock.cpp)

DIRECTORY_DESCRIPTOR *g_root;				// ��Ŀ¼������
DIRECTORY_DESCRIPTOR *g_current_dir;			// ��ǰĿ¼������
char g_current_path[MAX_PATH_NAME];			// ȫ�ֵ�ǰĿ¼�ַ���

#define MAGIC		"Nerv"
#define PATH_SEPS	"/"
#define AMOUNT_OF_SET	4
#define GROUP_PER_GB	128				// �����1GB��Ҫ128��������
#define BYTES_PER_GROUP	(1 * GB / GROUP_PER_GB)		// ÿ�����������ֽ���
#define GB_PER_BLOCK	16				// ��СΪһ��BLOCK��������֧�ֵ����������̴�С
#define BITMAP_BLOCK_PER_GB	8			// ÿGB���������Ҫ8��BLOCK��bitmap
#define ENTRY_PER_TABLE		((BLOCK_SIZE - sizeof(int)) / sizeof(DIRECTORY_ENTRY))		// һ��Ŀ¼�������漸��Ŀ¼��
#define BLOCK_PER_FILE_TABLE	(BLOCK_SIZE / sizeof(int) - 1)

// ��target��ָ����ڴ������еĵ�bit_posλ��Ϊvalue
void SetBit(unsigned char *target, int bit_pos, int value)
{
	int byte = bit_pos / 8;
	int offset = bit_pos % 8;
	int x = 1 << (7 - offset);
	if(value == 1)
		target[byte] |= x;
	else
		target[byte] &= (~x);
}

// ����target��ָ����ڴ������е�bit_posλ�Ƿ�Ϊ1
int IsBitSet(unsigned char *target, int bit_pos)
{
	int byte = bit_pos / 8;
	int offset = bit_pos % 8;
	int x = 1 << (7 - offset);
	return x & target[byte];
}

// �������֮�����fflush
void UpdateBitmap(int bit_pos, int value)
{
	SetBit(g_bitmap, bit_pos, value);
	int byte = bit_pos / 8;		// λͼ�е��ֽ�
	fseek(g_storage, g_file_system_header->bitmap_address * BLOCK_SIZE + byte, SEEK_SET);
	fwrite(&g_bitmap[byte], sizeof(Bitmap), 1, g_storage);

	// ά��Ⱥ�ڿ��п���
	SET *list = g_file_system_header->set;
	SET *set = NULL;
	for(int i = 0; i < AMOUNT_OF_SET; i++)
	{
		if(byte <= list[i].end)
		{
			set = &list[i];
			break;
		}
	}
	if(value == 1)
	{
		set->free_block--;
		g_file_system_header->amount_of_free_block--;
	}
	else
	{
		set->free_block++;
		g_file_system_header->amount_of_free_block++;
	}

	fwrite(list, sizeof(SET), AMOUNT_OF_SET, g_storage);
	fwrite(&g_file_system_header->amount_of_free_block, sizeof(g_file_system_header->amount_of_free_block), 1, g_storage);
}

ERROR_CODE miniFormat(void)
{
	ERROR_CODE status = ERR_SUCCESS;
	unsigned char buffer[BLOCK_SIZE];
	memset(buffer, 0, BLOCK_SIZE * sizeof(unsigned char));

	// ��ô����ļ���С
	__int64 disk_size;
	_fseeki64(g_storage, 0, SEEK_END);
	disk_size = _ftelli64(g_storage);
	_fseeki64(g_storage, 0, SEEK_SET);

	memset(g_file_system_header, 0, BLOCK_SIZE);

	memcpy(g_file_system_header->magic, MAGIC, strlen(MAGIC));
	g_file_system_header->version = MAKE_VERSION(0, 1);
	g_file_system_header->amount_of_free_block =
	g_file_system_header->amount_of_block = disk_size / BLOCK_SIZE;

	// �ļ���С�������ӣ���1GB�ı��ʡ�
	int factor = (int)ceil(g_file_system_header->amount_of_block * 4.0 / 1024 / 1024);
	int block = g_file_system_header->amount_of_block;

	// һ���ļ�ϵͳͷ + ���������(��ȡ��)
	g_file_system_header->bitmap_address = 1;

	g_file_system_header->bitmap_size = factor * BITMAP_BLOCK_PER_GB;

	// ��ʼ��SET
	g_file_system_header->amount_of_set = AMOUNT_OF_SET;
	SET *set = g_file_system_header->set;

	int total_group = 0;
	for(int i = 0; i < AMOUNT_OF_SET; i++)
	{
		set[i].amount_of_group = (i != AMOUNT_OF_SET - 1) ? (2 * (int)pow(4.0, i)) : (disk_size / GB * GROUP_PER_GB - total_group);
		total_group += set[i].amount_of_group;
		set[i].max_size = 8 * (int)pow(16.0, i) / (BLOCK_SIZE / KB);
		set[i].begin = (i != 0) ? (set[i-1].end + 1) : 0;
		set[i].end = set[i].begin + BYTES_PER_GROUP / BLOCK_SIZE * set[i].amount_of_group / 8 - 1;
		set[i].free_block = BYTES_PER_GROUP / BLOCK_SIZE * set[i].amount_of_group;
		set[i].last_search = set[i].begin;
	}
	// 0��Ⱥ����λ�������ļ�ϵͳͷ��λͼ
	set[0].begin = g_file_system_header->root_address / 8;
	set[0].last_search = set[0].begin;
	// 3��Ⱥ
	set[3].max_size = INT_MAX;
	set[3].end = g_file_system_header->bitmap_size * BLOCK_SIZE - 1;

	// ��ʼ��λͼ
	for(int i = 0; i < g_file_system_header->bitmap_size; i++)
	{
		status = miniWriteBlock(i + g_file_system_header->bitmap_address, BLOCK_SIZE, buffer);
		if(status != ERR_SUCCESS)
		{
			DbgPrint(status);
			return status;
		}
	}

	// root�ļ�ͷ��
	g_file_system_header->root_address = g_file_system_header->bitmap_address + g_file_system_header->bitmap_size;
	if(g_root == NULL)
		InitRoot();
	HEADER *h = g_root->self;
	strcpy(h->name, "root");
	h->create_time = h->modified_time = h->access_time = _time64(NULL);
	h->file_size = 0;
	h->hidden = 0;
	h->is_file = IS_DIR;
	h->access = MAKE_ACCESS(1, 0, 1);
	h->allocate_table_number = 1;
	h->table_block_number[0] = g_file_system_header->root_address + 1;
	miniWriteBlock(g_file_system_header->root_address, BLOCK_SIZE, h);
	miniWriteBlock(g_file_system_header->root_address + 1, BLOCK_SIZE, buffer);

	if(g_bitmap == NULL)
		CacheBitmap();
	memset(g_bitmap, 0, g_file_system_header->bitmap_size * BLOCK_SIZE);
	// ����ļ�ϵͳͷ��λͼ��root�ļ�ͷ��rootĿ¼�����
	for(int i = 0; i <= g_file_system_header->bitmap_size + 2; i++)
	{
		UpdateBitmap(i, 1);
	}
	fflush(g_storage);

	g_file_system_header->amount_of_free_block -= (g_file_system_header->bitmap_size + 3);

	status = miniWriteBlock(0, BLOCK_SIZE, g_file_system_header);
	return status;
}

bool miniEnoughSpace(FILE_DESCRIPTOR *fd)
{
	__int64 file_size = fd->self->file_size;
	__int64 system_free = g_file_system_header->amount_of_free_block * BLOCK_SIZE;

	return file_size < system_free;
}

bool miniEnoughSpace(FILE *fd)
{
	__int64 pos = _ftelli64(fd);
	_fseeki64(fd, 0, SEEK_END);
	__int64 file_size = _ftelli64(fd);
	_fseeki64(fd, pos, SEEK_SET);

	__int64 system_free = g_file_system_header->amount_of_free_block * BLOCK_SIZE;

	return file_size < system_free;
}

ERROR_CODE GetSearchStart(char *path, DIRECTORY_DESCRIPTOR **p_dd)
{
	if(path[0] == '/')
		*p_dd = g_root;
	else
		*p_dd = g_current_dir;

	return ERR_SUCCESS;
}

ERROR_CODE miniInitFileSystem(void)
{
	ERROR_CODE status = ERR_SUCCESS;
	status = CacheFileSystemHeader();

	if(status != ERR_SUCCESS)
	{
		return status;
	}
	CacheBitmap();
	InitRoot();
	InitCurrentDir();

	return ERR_SUCCESS;
}

ERROR_CODE CheckFileSystem(void)
{
	// ϵͳͷ��ʶ��
	if(memcmp(g_file_system_header->magic, MAGIC, strlen(MAGIC)) != 0)
		return ERR_UNKNOWN_FILE_SYSTEM;

	// У�����Ϣ
	__int64 disk_size;
	_fseeki64(g_storage, 0, SEEK_END);
	disk_size = _ftelli64(g_storage);
	_fseeki64(g_storage, 0, SEEK_SET);

	int amount_of_block = disk_size / BLOCK_SIZE;
	if(g_file_system_header->amount_of_block != amount_of_block)
		return ERR_NOT_FORMATED;

	// У��λͼ��Ϣ
	if(g_file_system_header->bitmap_address != 1 ||
		g_file_system_header->bitmap_size != disk_size / GB * BITMAP_BLOCK_PER_GB)
		return ERR_NOT_FORMATED;

	// У��root
	if(g_file_system_header->root_address != g_file_system_header->bitmap_address + g_file_system_header->bitmap_size)
		return ERR_NOT_FORMATED;

	// У��Ⱥ
	if(g_file_system_header->amount_of_set != AMOUNT_OF_SET)
		return ERR_NOT_FORMATED;

	return ERR_SUCCESS;
}
ERROR_CODE CacheFileSystemHeader(void)
{
	g_file_system_header = (FILE_SYSTEM_HEADER*)calloc(1,BLOCK_SIZE);
	memset(g_file_system_header, 0, BLOCK_SIZE);
	miniReadBlock(0, BLOCK_SIZE, g_file_system_header);

	// �������û�и�ʽ��
	if(CheckFileSystem() != ERR_SUCCESS)
	{
		free(g_file_system_header);
		return ERR_NOT_FORMATED;
	}
	else
	{
		return ERR_SUCCESS;
	}
}

ERROR_CODE CacheBitmap(void)
{
	g_bitmap = (Bitmap*)calloc(1,g_file_system_header->bitmap_size * BLOCK_SIZE);
	unsigned char *p = g_bitmap;
	for(int i = 0; i < g_file_system_header->bitmap_size; i++)
	{
		ERROR_CODE status = miniReadBlock(g_file_system_header->bitmap_address + i, BLOCK_SIZE, p);
		if(status != ERR_SUCCESS)
		{
			DbgPrint(status);
			return status;
		}
		p += BLOCK_SIZE;
	}
	return ERR_SUCCESS;
}

ERROR_CODE InitRoot(void)
{
	DIRECTORY_DESCRIPTOR *root;
	root = (DIRECTORY_DESCRIPTOR*)calloc(1, sizeof(DIRECTORY_DESCRIPTOR));
	root->offset = 0;
	root->dir = (DIRECTORY_HEADER*)calloc(1, BLOCK_SIZE);

	// �ļ�ͷָ��
	void *buf = calloc(1, BLOCK_SIZE);
	miniReadBlock(g_file_system_header->root_address, BLOCK_SIZE, buf);
	root->self = (HEADER*)buf;

	// �������
	root->table_buffer = calloc(1, BLOCK_SIZE);

	g_root = root;
	return ERR_SUCCESS;
}

ERROR_CODE InitCurrentDir(void)
{
	g_current_dir = (DIRECTORY_DESCRIPTOR*)calloc(1,sizeof(DIRECTORY_DESCRIPTOR));
	g_current_dir->offset = 0;

	g_current_dir->self = (HEADER*)calloc(1,BLOCK_SIZE);
	memcpy(g_current_dir->self, g_root->self, BLOCK_SIZE);

	g_current_dir->dir = (DIRECTORY_HEADER*)calloc(1,BLOCK_SIZE);
	memcpy(g_current_dir->dir, g_root->dir, BLOCK_SIZE);

	g_current_dir->table_buffer = calloc(1,BLOCK_SIZE);

	strcpy(g_current_path, "/");
	return ERR_SUCCESS;
}

/*
* ˵�������p_dhĿ¼�µ�name��Ŀ¼p_child_dh��
*      p_dh��p_child_dh����Ϊͬһ��������
* ������name, Ŀ¼��
*      p_dh, ��Ŀ¼ͷ
* �����p_child_dh, ��Ŀ¼ͷ
*      p_child_block����Ŀ¼ͷ�Ŀ��
*/
ERROR_CODE GetChildDirectory(char *name, DIRECTORY_HEADER *p_dh, DIRECTORY_HEADER *p_child_dh, int *p_child_block)
{
	ERROR_CODE status = ERR_SUCCESS;
	unsigned char buf[BLOCK_SIZE];

	// �������з����
	for(int i = 0; i < p_dh->allocate_table_number; i++)
	{
		// ��ȡĿ¼�����
		status = miniReadBlock(p_dh->table_block_number[i], BLOCK_SIZE, buf);
		if(status != ERR_SUCCESS)
		{
			DbgPrint(status);
			return status;
		}

		// ����Ŀ¼�����
		register DIRECTORY_ALLOCATE_TABLE *dir = (DIRECTORY_ALLOCATE_TABLE*)buf;
		for(int dir_index = 0; dir_index < dir->amount_of_entry; dir_index++)
		{
			// Ŀ¼��ƥ��
			if(strcmp(name, dir->entry[dir_index].name) == 0)
			{
				int child_block = dir->entry[dir_index].block_number;
				// Ŀ¼��ƥ��ɹ��Ͷ�ȡĿ¼���ļ�ͷ
				status = miniReadBlock(child_block, BLOCK_SIZE, buf);
				if(status != ERR_SUCCESS)
				{
				    DbgPrint(status);
				    return status;
				}

				DIRECTORY_HEADER *dir_header = (DIRECTORY_HEADER*)buf;
				if(dir_header->is_file == IS_DIR)
				{
					// ���Ŀ¼�ļ�ͷp_child_dh
					memcpy(p_child_dh, dir_header, BLOCK_SIZE);
					*p_child_block = child_block;
					return ERR_SUCCESS;
				}
				else
				{
					// ���Ͳ�ƥ��
					*p_child_block = -1;
					return ERR_TYPE_MISMATCH;
				}
			}
		}
	}

	return ERR_NOT_FOUND;
}

/*
* ˵�������p_dh�µ��ļ�name���ļ�ͷp_fh
* ������name���ļ���
*      p_dh������Ŀ¼ͷ
* �����p_fh���ļ�ͷ
*/
ERROR_CODE GetChildFile(char *name, DIRECTORY_HEADER *p_dh, FILE_HEADER *p_fh, int *file_block)
{
	ERROR_CODE status = ERR_SUCCESS;
	unsigned char buf[BLOCK_SIZE];

	// �������з����
	for(int i = 0; i < p_dh->allocate_table_number; i++)
	{
		status = miniReadBlock(p_dh->table_block_number[i], BLOCK_SIZE, buf);
		if(status != ERR_SUCCESS)
		{
			DbgPrint(status);
			return status;
		}

		// ����Ŀ¼�����
		DIRECTORY_ALLOCATE_TABLE *dir = (DIRECTORY_ALLOCATE_TABLE*)buf;
		for(int dir_index = 0; dir_index < dir->amount_of_entry; dir_index++)
		{
			if(strcmp(name, dir->entry[dir_index].name) == 0)
			{
				// �ļ���ƥ��ɹ��Ͷ�ȡ�ļ����ļ�ͷ
				int block = dir->entry[dir_index].block_number;
				status = miniReadBlock(block, BLOCK_SIZE, buf);
				if(status != ERR_SUCCESS)
				{
				    DbgPrint(status);
				    return status;
				}

				FILE_HEADER *file = (FILE_HEADER*)buf;
				if(file->is_file == IS_FILE)
				{
					// ����ļ�ͷp_fh
					memcpy(p_fh, file, BLOCK_SIZE);
					*file_block = block;
					return ERR_SUCCESS;
				}
				else
				{
					// ���Ͳ�ƥ��
					*file_block = -1;
					return ERR_TYPE_MISMATCH;
				}
			}
		}
	}

	return ERR_NOT_FOUND;
}

/*
* ������path - miniCreateFile��miniCreateDirectory�Ĳ���path
*      p_dir_block - ��Ŀ¼ͷ���
* ˵����path�������Ԥ����֮�����ʽΪ/path/to/file
*/
ERROR_CODE GetParentDirectory(char *path, DIRECTORY_HEADER *p_dh, int *dir_block)
{
	ERROR_CODE status = ERR_SUCCESS;
	char tmp[MAX_PATH_NAME + 1];
	int length = strlen(path);
	strncpy(tmp, path, (length < MAX_PATH_NAME) ? length : MAX_PATH_NAME);

	// ��֤·��βû��'/'
	if(tmp[length - 1] == '/')
		tmp[length - 1] = '\0';

	// '/'����
	int slash_total = 0;
	for(int i = 0; i < length; i++)
		if(tmp[i] == '/')
			slash_total++;

	// ������ʼ����·��
	DIRECTORY_DESCRIPTOR *start = NULL;
	GetSearchStart(path, &start);
	memcpy(p_dh, start->self, BLOCK_SIZE);

	// ������Ŀ¼
	char *token = strtok(tmp, PATH_SEPS);
	int index = 1;
	int block = -1;
	while(index <= slash_total - 1)
	{
		status = GetChildDirectory(token, p_dh, p_dh, &block);
		if(status != ERR_SUCCESS)
		{
			DbgPrint(status);
			return status;
		}
		token = strtok(NULL, PATH_SEPS);

		index++;
	}
	if(block != -1)
		*dir_block = block;
	else
	{
		// ��Ŀ¼�Ǹ�Ŀ¼
		memcpy(p_dh, g_root->self, BLOCK_SIZE);
		*dir_block = g_file_system_header->root_address;
	}
	return ERR_SUCCESS;
}

/*
* ·�������˳�򣺳��Ȳ��ó���MAX_PATH_NAME��·���в����зǷ��ַ�����·����ʼ����".."Ԥ����ɵ�ǰĿ¼�ĸ�Ŀ¼
*/
ERROR_CODE PreProcessPath(char *path)
{
	int length = strlen(path);
	if(length > MAX_PATH_NAME)
	{
		DbgPrint(ERR_PATH_TOO_LONG);
		return ERR_PATH_TOO_LONG;
	}

	if(length == 2 && path[0] == '.' && path[1] == '.')
	{
		path[2] = '/';
		path[3] = '\0';
		length++;
	}

	for(int i = 0; i < length; i++)
	{
		if(path[i] != '/' && path[i] != '.' && !isalpha(path[i]) && !isdigit(path[i]))
			return ERR_INVALID_FILE_NAME;
	}

	char tmp[MAX_PATH_NAME];
	int cur_length = strlen(g_current_path);
	
	// �����ϼ�·��
	if(path[0] == '.' && path[1] == '.' && path[2] == '/')
	{
		if(strcmp(g_current_path, "/") == 0)
		{
			// ��Ŀ¼û�и�Ŀ¼
			return ERR_INVALID_FILE_NAME;
		}

		if(cur_length + length - 2 > MAX_PATH_NAME)
			return ERR_PATH_TOO_LONG;

		int last_slash_pos = cur_length - 1;
		for(;last_slash_pos >= 0; last_slash_pos--)
		{
			if(g_current_path[last_slash_pos] == '/')
				break;
		}
		// ������ͷ��".."
		strncpy(tmp, &path[2], MAX_PATH_NAME);
		/* tmp�бض���'/'��ͷ�����g_current_pathֻ��һ��'/'(/xxx)��
		*  ��last_slash_pos == 0��strncpy������g_current_path��
		*  strcat��Ϊ��ȷ·��
		*/ 
		strncpy(path, g_current_path, last_slash_pos);
		path[last_slash_pos] = '\0';
		strcat(path, tmp);
		length = strlen(path);
	}
	else if(path[0] != '/')
	{
		if(cur_length + length > MAX_PATH_NAME)
			return ERR_PATH_TOO_LONG;
		strcpy(tmp, g_current_path);
		// ��ǰĿ¼���Ǹ�Ŀ¼ʱ��·����'/'
		if(!(cur_length == 1 && g_current_path[0] == '/'))
		{
			tmp[cur_length] = '/';
			tmp[cur_length + 1] = '\0';
		}
		strcat(tmp, path);
		strcpy(path, tmp);
		length = strlen(path);
	}
	if(length != 1 && path[length - 1] == '/')
		path[length - 1] = '\0';
	return ERR_SUCCESS;
}
/*
* ������size���ļ�ʵ�����ݴ�С��rģʽ���Ըò�����ֻ��wģʽ��Ч����λ���ֽڣ�
*      
*/
ERROR_CODE miniCreateFile(char *path, __int64 size, char *mode, FILE_DESCRIPTOR **p_fd)
{
	ERROR_CODE status = ERR_SUCCESS;

	status = PreProcessPath(path);
	if(status != ERR_SUCCESS)
	{
		DbgPrint(status);
		return ERR_INVALID_FILE_NAME;
	}

	DIRECTORY_HEADER *dh = (DIRECTORY_HEADER*)calloc(1,BLOCK_SIZE);
	// Ŀ¼ͷ�Ŀ��
	int dir_block = 0;
	status = GetParentDirectory(path, dh, &dir_block);
	if(status != ERR_SUCCESS)
	{
		DbgPrint(status);
		free(dh);
		return status;
	}

	// ���һ��б��λ�á��ļ�����ʼ��last_slash_pos + 1��
	int last_slash_pos = -1;
	for(int i = strlen(path) - 1; i >= 0; i--)
		if(path[i] == '/')
		{
			last_slash_pos = i;
			break;
		}

	// ����½����ļ�������
	int path_len = strlen(path);
	if(path_len - last_slash_pos - 1 > MAX_NAME)
		return ERR_INVALID_FILE_NAME;
	FILE_HEADER *fh = (FILE_HEADER*)calloc(1,BLOCK_SIZE);
	int file_block = 0;
	status = GetChildFile(&path[last_slash_pos + 1], dh, fh, &file_block);
	if(mode[0] == 'r')
	{
		if(status != ERR_SUCCESS)
		{
			DbgPrint(status);
			free(dh);
			free(fh);
			return status;
		}
		else
		{
			fh->access_time = _time64(NULL);
		}
	}

	if(mode[0] == 'w')
	{
		if(status == ERR_SUCCESS)
		{
			fh->access_time = fh->modified_time = _time64(NULL);
		}
		// wģʽ���ļ��������򴴽�
		if(status == ERR_NOT_FOUND)
		{
			status = NewEmptyItem(&path[last_slash_pos + 1], size, dh, dir_block, fh, &file_block);
			if(status != ERR_SUCCESS)
			{
				DbgPrint(status);
				free(dh);
				free(fh);
				return status;
			}
		}
		else	// �������󷵻ش���״̬
		{
			DbgPrint(status);
			free(dh);
			free(fh);
			return status;
		}
	}
	miniWriteBlock(file_block, BLOCK_SIZE, fh);

	// �����ļ�������
	FILE_DESCRIPTOR *fd = (FILE_DESCRIPTOR*)calloc(1, sizeof(FILE_DESCRIPTOR));
	fd->dir_block = dir_block;
	fd->dir = dh;
	fd->self_block = file_block;
	fd->self = fh;
	fd->offset= 0;
	fd->table_buffer = calloc(1,BLOCK_SIZE);
	*p_fd = fd;

	return status;
}
/*
* ��������dhĿ¼���½�name�������flag����
*      size���ļ����ݴ�С(��λ���ֽ�)
*
*/
ERROR_CODE NewEmptyItem(char *name, __int64 size, DIRECTORY_HEADER *dh, int dir_block, HEADER *hh, int *item_block)
{
	int how_many_blocks = 0;
	int data_block = (int)ceil((double)size / BLOCK_SIZE);
	int table_number = 0;

	// Ŀ¼
	if(size == 0)
	{
		// ��Ŀ¼��֤һ��Ŀ¼�����Ĵ���
		table_number = 1;
		// һ��Ŀ¼ͷ + һ�������
		how_many_blocks = 2;
	}
	else
	{
	// �ļ�
		// һ���ļ�������ܴ�1023������ڡ�
		table_number = (int)ceil((double)data_block / BLOCK_PER_FILE_TABLE);
		// һ���ļ�ͷ + ���ݿ���� + �ļ������
		how_many_blocks = 1 + data_block + table_number;
	}

	if(how_many_blocks > g_file_system_header->amount_of_free_block)
		return ERR_NOT_ENOUGH_DISK_SPACE;

	// �����ļ�ͷ
	int block_number = LookForFreeBlock(how_many_blocks);
	if(block_number == -1)
	{
		DbgPrint(ERR_OTHER);
		return ERR_OTHER;
	}
	*item_block = block_number;
	fflush(g_storage);
	memset(hh->name, 0, MAX_NAME);
	memcpy(hh->name, name, strlen(name));
	hh->create_time = hh->access_time = hh->modified_time = _time64(NULL);
	hh->file_size = size;
	hh->hidden = 0;
	hh->is_file = (size == 0) ? IS_DIR : IS_FILE;
	hh->access = MAKE_ACCESS(1, 0, ((size == 0) ? 1 : 0));	// Ĭ��Ȩ�ޣ��ļ�r--�� Ŀ¼r-x
	hh->allocate_table_number = table_number;

	// �����ļ������
	for(int i_table = 0; i_table < table_number; i_table++)
	{
		int table_block = LookForFreeBlock(how_many_blocks);
		if(table_block == -1)
		{
			DbgPrint(ERR_OTHER);
			return ERR_OTHER;
		}
		hh->table_block_number[i_table] = table_block;

		// �������ݿ�
		char buf[BLOCK_SIZE];
		memset(buf, 0, BLOCK_SIZE);
		register FILE_ALLOCATE_TABLE *fat = (FILE_ALLOCATE_TABLE*)buf;
		for(int i_data = 0; i_data < BLOCK_PER_FILE_TABLE && data_block > 0; i_data++, data_block--)
		{
			int data = LookForFreeBlock(how_many_blocks);
			if(data == -1)
			{
				DbgPrint(ERR_OTHER);
				return ERR_OTHER;
			}
			fat->amount_of_block++;
			fat->block_number[i_data] = data;
		}
		miniWriteBlock(table_block, BLOCK_SIZE, fat);
	}
	fflush(g_storage);

	miniWriteBlock(block_number, BLOCK_SIZE, hh);
	InsertToDirectory(dh, dir_block, hh, block_number);
	return ERR_SUCCESS;
}

/*
* ������size���������ļ���С����λ���飩
* ���أ��ҵ��Ŀ��п�š������򷵻�-1
*
*/
int LookForFreeBlock(int size)
{
	SET *list = g_file_system_header->set;
	int total_set = g_file_system_header->amount_of_set;

	// Ѱ�����ʺϵ�Ⱥ��
	int set_index;
	for(set_index = 0; set_index < total_set; set_index++)
	{
		if(size <= list[set_index].max_size)
		{
			break;
		}
	}

	int origin = set_index;

	// ��ǰȺ��δ�ҵ���������һ��
	register SET *set;
	register int addr;
	int end_pos;
	do
	{
		set = &list[set_index];
		// Ⱥ������ʱֱ���������Ⱥ
		if(set->free_block == 0)
		{
			goto fast_skip;
		}

		// ����Ⱥ�������ֽ�
		end_pos = set->last_search - 1;
		if(end_pos < set->begin)
			end_pos = set->end;

		for(addr = set->last_search;
			addr != end_pos;
			addr = ((addr + 1 <= set->end) ? (addr + 1) : (set->begin))
			)
		{
			// ��������λ
			for(int bit = 0; bit < 8; bit++)
			{
				if(!IsBitSet(&g_bitmap[addr], bit))
				{
					set->last_search = addr;
					int rtn = addr * 8 + bit;
					if(!((0 <= addr) && (addr < g_file_system_header->bitmap_size * BLOCK_SIZE)))
					{
						printf("Block 0x%08x out of range!\n", rtn);
						return -1;
					}
					assert(IsBitSet(g_bitmap, rtn) == false);
					UpdateBitmap(rtn, 1);
					return rtn;
				}
			}
		}

		// Ⱥ�ڿռ�����ʱֱ����������
		fast_skip:
		set_index++;
		// ����0��Ⱥ������ļ�
		if(size != 2)
			(set_index == total_set) ? (set_index = 1) : (0);
		else
			set_index %= total_set;
	}while(origin != set_index);
	return -1;
}

// ���ļ�fd�У���fd->offset����ʼ��ȡbuffer�е�size���ֽڣ����ض���size_read��
ERROR_CODE miniReadFile(FILE_DESCRIPTOR *fd, __int64 size, int buffer_size, void *buffer, __int64 *size_read)
{
	ERROR_CODE status = ERR_SUCCESS;
	char *buf = (char*)buffer;
	register int *table_block_number = fd->self->table_block_number;

	// ��黺��������
	if(size > buffer_size)
	{
		*size_read = 0;
		return ERR_BUFFER_OVERFLOW;
	}

	// �����ͼ��ȡ�ĳ��ȳ����ļ����ȣ����ȡ���ļ�β
	if(fd->offset + size > fd->self->file_size)
		size = fd->self->file_size - fd->offset;

	int start_block, end_block;
	start_block = fd->offset / BLOCK_SIZE;
	end_block = (fd->offset + size) / BLOCK_SIZE;

	// �ȴ����һ��
	miniReadBlock(LogicalToPhysical(fd->self, start_block), BLOCK_SIZE, fd->table_buffer);
	int first_size = fd->offset % BLOCK_SIZE;
	char *start_addr = (char*)fd->table_buffer + first_size;
	if(start_block == end_block)
	{
		// ������һ����֮��
		memcpy(buf, start_addr, size);
		*size_read = size;
		fd->offset += size;
		return ERR_SUCCESS;
	}
	else
	{
		// ���ݳ��ȳ���һ����
		memcpy(buf, start_addr, BLOCK_SIZE - first_size);
		buf += first_size;
	}

	// �������һ������һ��֮�������
	for(int i = start_block + 1; i <= end_block - 1; i++)
	{
		miniReadBlock(LogicalToPhysical(fd->self, i), BLOCK_SIZE, buf);
		buf += BLOCK_SIZE;
	}

	// �������һ������
	int end_size = (fd->offset + size) % BLOCK_SIZE;
	miniReadBlock(LogicalToPhysical(fd->self, end_block), BLOCK_SIZE, fd->table_buffer);
	memcpy(buf, fd->table_buffer, end_size);

	fd->offset += size;
	*size_read = size;
	return ERR_SUCCESS;
}

// ���ļ�fd�У���fd->offset����ʼд��buffer�е�size���ֽڣ�����д��size_write��
ERROR_CODE miniWriteFile(FILE_DESCRIPTOR *fd, __int64 size, int buffer_size, void *buffer, __int64 *size_write)
{
	ERROR_CODE status = ERR_SUCCESS;
	char *buf = (char*)buffer;
	register int *table_block_number = fd->self->table_block_number;

	// ��黺��������
	if(size > buffer_size)
	{
		*size_write = size;
		return ERR_BUFFER_OVERFLOW;
	}

	// �߼����
	int start_block, end_block;
	start_block = fd->offset / BLOCK_SIZE;
	end_block = (fd->offset + size - 1) / BLOCK_SIZE;

	// �ȴ����һ��
	int first_size = fd->offset % BLOCK_SIZE;
	char *start_addr = (char*)fd->table_buffer + first_size;
	// ֻ�е���һ�鲻�Ǵ�ͷ��ʼд��ʱ����Ҫ����ԭ�������ݷ�ֹ���ǲ���Ҫд�Ĳ���
	if(first_size != 0)
		miniReadBlock(LogicalToPhysical(fd->self, start_block), BLOCK_SIZE, fd->table_buffer);
	if(start_block == end_block)
	{
		// ������һ��֮��
		memcpy(start_addr, buf, size);
		miniWriteBlock(LogicalToPhysical(fd->self, start_block), BLOCK_SIZE, fd->table_buffer);
		*size_write = size;
		fd->offset += size;
		return ERR_SUCCESS;
	}
	else
	{
		// ���ݳ��ȳ���һ��
		memcpy(start_addr, buf, BLOCK_SIZE - first_size);
		miniWriteBlock(LogicalToPhysical(fd->self, start_block), BLOCK_SIZE, fd->table_buffer);
		buf += first_size;
	}

	// �������һ������һ��֮�������
	for(int i = start_block + 1; i <= end_block - 1; i++)
	{
		miniWriteBlock(LogicalToPhysical(fd->self, i), BLOCK_SIZE, buf);
		buf += BLOCK_SIZE;
	}

	// �������һ������
	int end_size = (fd->offset + size - 1) % BLOCK_SIZE;
	// ���һ���ֽڲ��ڿ�ĩβʱ�Ŷ���ԭ���ݷ�ֹ���ǲ���Ҫд�Ĳ���
	if(end_size != BLOCK_SIZE - 1)
		miniReadBlock(LogicalToPhysical(fd->self, end_block), BLOCK_SIZE, fd->table_buffer);
	memcpy(fd->table_buffer, buf, end_size);
	miniWriteBlock(LogicalToPhysical(fd->self, end_block), BLOCK_SIZE, fd->table_buffer);

	fd->offset += size;
	*size_write = size;
	return ERR_SUCCESS;
}

// ���ļ�/Ŀ¼hh���ļ�ͷ�Ŀ�Ų��뵽dh��Ŀ¼���������
ERROR_CODE InsertToDirectory(DIRECTORY_HEADER *dh, int dir_block, HEADER *hh, int header_block)
{
	unsigned char buf[BLOCK_SIZE];
	ERROR_CODE status = ERR_SUCCESS;
	int target_block = dh->table_block_number[dh->allocate_table_number - 1];

	status = miniReadBlock(target_block, BLOCK_SIZE, buf);
	if(status != ERR_SUCCESS)
	{
		DbgPrint(status);
		return status;
	}

	DIRECTORY_ALLOCATE_TABLE *dat = (DIRECTORY_ALLOCATE_TABLE*)buf;
	// ���һ��Ŀ¼���������
	if(dat->amount_of_entry == ENTRY_PER_TABLE)
	{
		target_block = LookForFreeBlock(2);
		dh->table_block_number[dh->allocate_table_number] = target_block;
		dh->allocate_table_number++;
		fflush(g_storage);

		// �������dat���
		memset(dat, 0, BLOCK_SIZE);
		dat->amount_of_entry = 0;
	}


	DIRECTORY_ENTRY *entry = &dat->entry[dat->amount_of_entry];
	entry->block_number = header_block;
	entry->is_file = hh->is_file;
	strcpy(entry->name, hh->name);

	dat->amount_of_entry++;

	// д��Ŀ¼�����
	miniWriteBlock(target_block, BLOCK_SIZE, dat);
	// д������Ŀ¼�ļ�ͷ
	miniWriteBlock(dir_block, BLOCK_SIZE, dh);

	return ERR_SUCCESS;
}

// ��fd��ƫ�����Ƶ�offset��
ERROR_CODE miniSeekFile(FILE_DESCRIPTOR *fd, __int64 offset)
{
	if(offset > fd->self->file_size)
		return ERR_OUT_OF_RANGE;

	fd->offset = offset;
	return ERR_SUCCESS;
}

ERROR_CODE miniCloseFile(FILE_DESCRIPTOR *fd)
{
	CloseItem(fd);

	return ERR_SUCCESS;
}

ERROR_CODE miniExitSystem(void)
{
	miniWriteBlock(0, BLOCK_SIZE, g_file_system_header);
	for(int i = 0; i < g_file_system_header->bitmap_size; i++)
	{
		miniWriteBlock(1 + i, BLOCK_SIZE, &g_bitmap[i]);
	}
	free(g_file_system_header);
	free(g_bitmap);

	return ERR_SUCCESS;
}

// ɾ���ļ�fd
ERROR_CODE miniDeleteFile(FILE_DESCRIPTOR *fd)
{
	return DeleteItem(fd);
}

// �ڸ�Ŀ¼��������ҵ���ɾ������ɾ��
ERROR_CODE DeleteItem(DESCRIPTOR *d)
{
	ERROR_CODE status = ERR_SUCCESS;
	char buf[BLOCK_SIZE];
	register DIRECTORY_HEADER *dh = d->dir;
	char *name = d->self->name;
	register DIRECTORY_ALLOCATE_TABLE *dat = (DIRECTORY_ALLOCATE_TABLE*)d->table_buffer;
	DIRECTORY_ALLOCATE_TABLE *last_table = (DIRECTORY_ALLOCATE_TABLE*)buf;
	DIRECTORY_ENTRY de;

	// ��ȡ���һ�������
	miniReadBlock(dh->table_block_number[dh->allocate_table_number - 1], BLOCK_SIZE, dat);
	// �������һ�������
	memcpy(last_table, dat, BLOCK_SIZE);
	// �������һ��Ŀ¼��
	memcpy(&de, &dat->entry[dat->amount_of_entry - 1], sizeof(DIRECTORY_ENTRY));

	bool success = false;
	// ��������Ŀ¼�����
	for(int i_table = 0; i_table < dh->allocate_table_number; i_table++)
	{
		miniReadBlock(dh->table_block_number[i_table], BLOCK_SIZE, dat);
		DIRECTORY_ENTRY *entry = dat->entry;
		// ��������Ŀ¼������е�����Ŀ¼��
		for(int i_entry = 0; i_entry < dat->amount_of_entry; i_entry++)
		{
			// �ҵ�Ŀ¼��
			if(strcmp(name, entry[i_entry].name) == 0)
			{
				status = FreeItemDiskSpace(entry[i_entry].block_number);
				// ����Ŀ¼���ܷǿ�
				if(status != ERR_SUCCESS)
				{
					return status;
				}
				// �����һ��Ŀ¼��ǵ��ҵ���Ŀ¼��
				memcpy(&entry[i_entry], &de, sizeof(DIRECTORY_ENTRY));
				miniWriteBlock(dh->table_block_number[i_table], BLOCK_SIZE, dat);
				// ��������һ��Ŀ¼�����ҵ�Ŀ¼�����»����е����һ��Ŀ¼��last_table
				if(i_table == dh->allocate_table_number - 1)
					memcpy(last_table, dat, BLOCK_SIZE);
				// ���һ���������Ŀ¼������1
				last_table->amount_of_entry--;
				success = true;

				goto out;
			}
		}
	}

out:
	// û�ҵ�
	if(!success)
		return ERR_NOT_FOUND;

	// ���һ�ŷ����Ϊ�����ͷ�
	if(last_table->amount_of_entry == 0)
	{
		// ���ٱ�֤һ��Ŀ¼�����
		if(dh->allocate_table_number >= 2)
		{
			dh->allocate_table_number--;
			miniWriteBlock(d->self_block, BLOCK_SIZE, dh);
		}
	}
	miniWriteBlock(dh->table_block_number[dh->allocate_table_number - 1], BLOCK_SIZE, last_table);

	return ERR_SUCCESS;
}

/* �ͷ��ļ�/Ŀ¼��ռ�õ����д��̿ռ�
*  ����ֵ��ERR_DIRECTORY_NOT_EMPTY��ERR_SUCCESS
*/
ERROR_CODE FreeItemDiskSpace(int item_block)
{
	char header_buf[BLOCK_SIZE];
	char table_buf[BLOCK_SIZE];

	register HEADER *h = (HEADER*)header_buf;
	miniReadBlock(item_block, BLOCK_SIZE, h);

	
	if(h->is_file == IS_DIR)
	{
		if(h->allocate_table_number > 1)
			return ERR_DIRECTORY_NOT_EMPTY;
		miniReadBlock(h->table_block_number[0], BLOCK_SIZE, table_buf);
		DIRECTORY_ALLOCATE_TABLE *dat = (DIRECTORY_ALLOCATE_TABLE*)table_buf;
		if(dat->amount_of_entry != 0)
			return ERR_DIRECTORY_NOT_EMPTY;

		UpdateBitmap(h->table_block_number[0], 0);
		UpdateBitmap(item_block, 0);
	}
	else
	{
		register FILE_ALLOCATE_TABLE *fat = (FILE_ALLOCATE_TABLE*)table_buf;
		// �������з����
		for(int i_table = 0; i_table < h->allocate_table_number; i_table++)
		{
			miniReadBlock(h->table_block_number[i_table], BLOCK_SIZE, fat);
			// �����������ݿ�
			for(int i_data = 0; i_data < fat->amount_of_block; i_data++)
			{
				UpdateBitmap(fat->block_number[i_data], 0);
			}
			UpdateBitmap(h->table_block_number[i_table], 0);
		}
	}
	fflush(g_storage);
	return ERR_SUCCESS;
}

// �߼����ת����������(�̲߳���ȫ)
inline int LogicalToPhysical(FILE_HEADER *fh, int logical_block)
{
	// ������ƣ�����д��ͬһ���ļ��������ʱֱ�Ӵӻ����ȡ���ݿ��ַ
	// ǰһ���ļ������������
	static int last_physical_block = -1;
	// ǰһ���ļ��������
	static char buf[BLOCK_SIZE];

	register int index = logical_block / BLOCK_PER_FILE_TABLE;
	register int offset = logical_block % BLOCK_PER_FILE_TABLE;
	FILE_ALLOCATE_TABLE *fat = (FILE_ALLOCATE_TABLE*)buf;
	int physical_block = fh->table_block_number[index];

	if(last_physical_block != physical_block)
	{
		miniReadBlock(physical_block, BLOCK_SIZE, fat);
		last_physical_block = physical_block;
	}
	return fat->block_number[offset];
}

// 
ERROR_CODE miniCreateDirectory(char *path, char *mode, DIRECTORY_DESCRIPTOR **p_dd)
{
	ERROR_CODE status = ERR_SUCCESS;
	if(path == NULL)
		path = g_current_path;

	status = PreProcessPath(path);
	if(status != ERR_SUCCESS)
	{
		DbgPrint(status);
		return status;
	}

	// ���path�Ǹ�Ŀ¼����ֱ�Ӹ���һ�ݸ�Ŀ¼������
	if(strcmp(path, "/") == 0)
	{
		DIRECTORY_DESCRIPTOR *dd = (DIRECTORY_DESCRIPTOR*)calloc(1, sizeof(DIRECTORY_DESCRIPTOR));
		dd->dir = (DIRECTORY_HEADER*)calloc(1, BLOCK_SIZE);
		memcpy(dd->dir, g_root->dir, BLOCK_SIZE);
		dd->dir_block = g_root->dir_block;
		dd->self = (DIRECTORY_HEADER*)calloc(1, BLOCK_SIZE);
		memcpy(dd->self, g_root->self, BLOCK_SIZE);
		dd->self_block = g_root->self_block;
		dd->offset = 0;
		dd->table_buffer = calloc(1, BLOCK_SIZE);
		*p_dd = dd;

		return ERR_SUCCESS;
	}


	DIRECTORY_HEADER *dh = (DIRECTORY_HEADER*)calloc(1,BLOCK_SIZE);
	// Ŀ¼ͷ�Ŀ��
	int dir_block = 0;
	status = GetParentDirectory(path, dh, &dir_block);
	if(status != ERR_SUCCESS)
	{
		DbgPrint(status);
		free(dh);
		return status;
	}

	// ���һ��б��λ�á��ļ�����ʼ��last_slash_pos + 1��
	int last_slash_pos = -1;
	for(int i = strlen(path) - 1; i >= 0; i--)
		if(path[i] == '/')
		{
			last_slash_pos = i;
			break;
		}
	// ����½����ļ�������
	int path_len = strlen(path);
	if(path_len - last_slash_pos - 1 > MAX_NAME)
		return ERR_INVALID_FILE_NAME;

	DIRECTORY_HEADER *child_dh = (DIRECTORY_HEADER*)calloc(1, BLOCK_SIZE);
	int child_dir_block = 0;
	status = GetChildDirectory(&path[last_slash_pos + 1], dh, child_dh, &child_dir_block);
	if(mode[0] == 'r')
	{
		if(status != ERR_SUCCESS)
		{
			DbgPrint(status);
			free(dh);
			free(child_dh);
			return status;
		}
		else
		{
			child_dh->access_time = _time64(NULL);
		}
	}

	if(mode[0] == 'w')
	{
		if(status == ERR_SUCCESS)
		{
			child_dh->access_time = child_dh->modified_time = _time64(NULL);
		}
		else if(status == ERR_NOT_FOUND)	// wģʽ���ļ��������򴴽�
		{
			status = NewEmptyItem(&path[last_slash_pos + 1], 0, dh, dir_block, child_dh, &child_dir_block);

			if(status != ERR_SUCCESS)
			{
				DbgPrint(status);
				free(dh);
				free(child_dh);
				return status;
			}
		}
		else	// �������󷵻ش���״̬
		{
			DbgPrint(status);
			free(dh);
			free(child_dh);
			return status;
		}
	}
	miniWriteBlock(child_dir_block, BLOCK_SIZE, child_dh);
	// ����Ŀ¼������
	DIRECTORY_DESCRIPTOR *dd = (DIRECTORY_DESCRIPTOR*)calloc(1, sizeof(DIRECTORY_DESCRIPTOR));
	dd->dir_block = dir_block;
	dd->dir = dh;
	dd->self_block = child_dir_block;
	dd->self = child_dh;
	dd->offset = 0;
	dd->table_buffer = calloc(1, BLOCK_SIZE);
	*p_dd = dd;

	return ERR_SUCCESS;
}

// ɾ��Ŀ¼dd
ERROR_CODE miniDeleteDirectory(DIRECTORY_DESCRIPTOR *dd)
{
	return DeleteItem(dd);
}

// �ر��ļ�/Ŀ¼
ERROR_CODE CloseItem(DESCRIPTOR *d)
{
	free(d->dir);
	free(d->self);
	free(d->table_buffer);
	free(d);

	return ERR_SUCCESS;
}

ERROR_CODE miniCloseDirectory(DIRECTORY_DESCRIPTOR *dd)
{
	return CloseItem(dd);
}

/* ÿ����һ�δ�dd�ж���һ��Ŀ¼�buffer
*  ˵������bufferΪNULL����buffer_size������Ҫ�Ļ��������ȣ���λ���ֽڣ� 
*       ��buffer��ΪNULL������buffer_sizeΪ���������ȣ���λ���ֽڣ������buffer_sizeΪ������ddĿ¼��Ŀ¼������
*/
ERROR_CODE miniReadDirectory(DIRECTORY_DESCRIPTOR *dd, int *buffer_size, void *buffer)
{
	register DIRECTORY_HEADER *parent_dir = dd->dir;
	register DIRECTORY_HEADER *dir = dd->self;
	register int rtn_size = 0;
	// real_sizeȡ���з�����С
	int real_size = dir->allocate_table_number * BLOCK_SIZE;
	if(*buffer_size < real_size)
	{
		*buffer_size = real_size;
		return ERR_BUFFER_OVERFLOW;
	}

	DIRECTORY_ENTRY *p = (DIRECTORY_ENTRY*)buffer;
	for(int i_table = 0; i_table < dir->allocate_table_number; i_table++)
	{
		// ��ȡĿ¼�����
		miniReadBlock(dir->table_block_number[i_table], BLOCK_SIZE, dd->table_buffer);
		DIRECTORY_ALLOCATE_TABLE *dat = (DIRECTORY_ALLOCATE_TABLE*)dd->table_buffer;
		// ������Ŀ¼��Ƶ�buffer��
		memcpy(p, dat->entry, dat->amount_of_entry * sizeof(DIRECTORY_ENTRY));
		p += dat->amount_of_entry;
		rtn_size += dat->amount_of_entry;
	}
	*buffer_size = rtn_size;
	return ERR_SUCCESS;
}

ERROR_CODE miniWriteDirectory(DIRECTORY_DESCRIPTOR *dd, DESCRIPTOR *target)
{
	return InsertToDirectory(dd->self, dd->self_block, target->self, target->self_block);
}

ERROR_CODE miniChangeCurrentDirectory(char *path)
{
	ERROR_CODE status = ERR_SUCCESS;
	status = PreProcessPath(path);
	if(status != ERR_SUCCESS)
		return status;

	DIRECTORY_DESCRIPTOR *dd;
	status = miniCreateDirectory(path, "r", &dd);
	if(status != ERR_SUCCESS)
		return status;

	g_current_dir->dir_block = dd->dir_block;
	memcpy(g_current_dir->dir, dd->dir, BLOCK_SIZE);
	g_current_dir->self_block = dd->self_block;
	memcpy(g_current_dir->self, dd->self, BLOCK_SIZE);
	g_current_dir->offset = dd->offset;

	// ����·��
	if(path[0] == '/')
		strcpy(g_current_path, path);
	else
	{
		int length = strlen(g_current_path);
		// ��Ŀ¼����Ҫ��'/'
		if(length > 1)
		{
			g_current_path[length] = '/';
			g_current_path[length + 1] = '\0';
		}
		strcat(g_current_path, path);
	}

	miniCloseDirectory(dd);
	return ERR_SUCCESS;
}

ERROR_CODE miniGetSystemInfo(char *magic, int *major, int *minor, int *free)
{
	memcpy(magic, g_file_system_header->magic, 4);
	magic[4] = '\0';
	
	int version = g_file_system_header->version;
	*major = version >> 16;
	*minor = version & 0xffff;

	*free = g_file_system_header->amount_of_free_block / (1 * MB / BLOCK_SIZE);

	return ERR_SUCCESS;
}