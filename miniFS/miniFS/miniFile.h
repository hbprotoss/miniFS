#ifndef MINI_FILE_H
#define MINI_FILE_H

#include <cstdio>
#include <ctime>

#include "miniError.h"

#define	MAX_NAME	12
#define MAX_PATH_NAME	256
#define MAX_MAGIC	4

#pragma pack(push, 1)
////////////////////////////////////////////////////////////////////////////////////////////
// �ļ�ϵͳ

typedef unsigned char Bitmap;

/*
* �飺��λͼ��n���ֽڷֳ�m���飬ÿ����n/m���ֽ�
* Ⱥ�������鹹��һ��Ⱥ����������ָ���ļ���С�������µ��ļ�
*/
typedef struct _SET {
	int amount_of_group;	// Ⱥ����ĸ���
	int free_block;		// ���п����
	int max_size;		// ������ļ���С���ޣ���λ���飩
	int begin;		// bitmap����ʼ��ַ����λ���ֽڣ�
	int end;		// bitmap����ֹ��ַ����λ���ֽڣ�
	int last_search;	// Ⱥ���ϴ�������ֹ��ַ����λ���ֽڣ�
} SET, *PSET;

// �ļ�ϵͳͷ
typedef struct _FILE_SYSTEM_HEADER {
	char magic[4];			// �ļ�ϵͳ��ʶ��
	int version;			// �ļ�ϵͳ�汾��
	int amount_of_block;		// �ܿ���
	int amount_of_free_block;	// ���п���
	int root_address;		// ��Ŀ¼�ļ����
	int bitmap_address;		// λͼ��ʼ���
	int bitmap_size;		// λͼ��С����λ���飩
	int group_size;			// ��������Ҫ�Ŀ���
	int amount_of_set;		// Ⱥ��SET������
	SET set[1];			// Ⱥ��SET���б�
} FILE_SYSTEM_HEADER, *PFILE_SYSTEM_HEADER;

////////////////////////////////////////////////////////////////////////////////////////////
// �ļ�

// �ļ������
typedef struct _FILE_ALLOCATE_TABLE {
	int amount_of_block;		// ���п�����
	int block_number[1];		// ÿ������ڱ��
} FILE_ALLOCATE_TABLE, *PFILE_ALLOCATE_TABLE;

// Ŀ¼��
typedef struct _DIRECTORY_ENTRY {
	int block_number;		// Ŀ¼�ļ����š�
	short is_file;			// �Ƿ����ļ�
	char name[MAX_NAME];		// Ŀ¼��
} DIRECTORY_ENTRY, *PDIRECTORY_ENTRY;

// Ŀ¼�����
typedef struct _DIRECTORY_ALLOCATE_TABLE {
	int amount_of_entry;		// ����Ŀ¼������
	DIRECTORY_ENTRY entry[1];	// Ŀ¼���
} DIRECTORY_ALLOCATE_TABLE, *PDIRECTORY_ALLOCATE_TABLE;

typedef unsigned char FLAG_DIR_FILE;
#define	IS_DIR		0
#define IS_FILE		1

// ����ͷ
typedef struct _HEADER {
	char name[MAX_NAME];		// �ļ���
	__time64_t create_time;		// ����ʱ��
	__time64_t modified_time;	// �޸�ʱ��
	__time64_t access_time;		// ����ʱ��
	__int64 file_size;		// �ļ���С
	char hidden;			// �Ƿ�����
	FLAG_DIR_FILE is_file;		// �ļ�/Ŀ¼���
	char access;			// ����Ȩ��
	int allocate_table_number;	// �ļ����������
	int table_block_number[1];	// �ļ������
} HEADER, *PHEADER;

// �ļ�����ͷ
typedef HEADER FILE_HEADER;
// Ŀ¼����ͷ
typedef HEADER DIRECTORY_HEADER;

// ������
typedef struct _DESCRIPTOR {
	__int64 offset;				// �ļ�ƫ����
	int dir_block;				// Ŀ¼ͷ���
	DIRECTORY_HEADER *dir;			// ����Ŀ¼�ļ�ָ��
	int self_block;				// �ļ�ͷ���
	HEADER *self;				// �ļ�ͷָ��
	void *table_buffer;			// �ļ�/Ŀ¼������棨һ�����С����ֹƵ��malloc��
} DESCRIPTOR, *PDESCRIPTOR;

// �ļ�������
typedef DESCRIPTOR FILE_DESCRIPTOR;
// Ŀ¼������
typedef DESCRIPTOR DIRECTORY_DESCRIPTOR;

#pragma pack(pop)

ERROR_CODE miniCreateFile(char *path, __int64 size, char *mode, FILE_DESCRIPTOR **p_fd);
ERROR_CODE miniReadFile(FILE_DESCRIPTOR *fd, __int64 size, int buffer_size, void *buffer, __int64 *size_read);
ERROR_CODE miniWriteFile(FILE_DESCRIPTOR *fd, __int64 size, int buffer_size, void *buffer, __int64 *size_write);
ERROR_CODE miniSeekFile(FILE_DESCRIPTOR *fd, __int64 offset);
ERROR_CODE miniDeleteFile(FILE_DESCRIPTOR *fd);
ERROR_CODE miniCloseFile(FILE_DESCRIPTOR *fd);

ERROR_CODE miniCreateDirectory(char *path, char *mode, DIRECTORY_DESCRIPTOR **p_dd);
ERROR_CODE miniReadDirectory(DIRECTORY_DESCRIPTOR *dd, int *buffer_size, void *buffer);
ERROR_CODE miniWriteDirectory(DIRECTORY_DESCRIPTOR *dd, DESCRIPTOR *target);
ERROR_CODE miniDeleteDirectory(DIRECTORY_DESCRIPTOR *dd);
ERROR_CODE miniCloseDirectory(DIRECTORY_DESCRIPTOR *dd);

ERROR_CODE miniChangeCurrentDirectory(char *path);
ERROR_CODE miniGetSystemInfo(char *magic, int *major, int *minor, int *free);

bool miniEnoughSpace(FILE_DESCRIPTOR *fd);
bool miniEnoughSpace(FILE *fd);
ERROR_CODE miniFormat(void);
ERROR_CODE miniOptimize(void);
ERROR_CODE miniInitFileSystem(void);
ERROR_CODE miniExitSystem(void);

/////////////////////////////////////////////////////////////////////////////////////////////////
// ��������
inline void SetBit(unsigned char *target, int bit_pos, int value);
inline int IsBitSet(unsigned char *target, int bit_pos);
inline void UpdateBitmap(int bit_pos, int value);

ERROR_CODE CheckFileSystem(void);
ERROR_CODE CacheFileSystemHeader(void);
ERROR_CODE CacheBitmap(void);
ERROR_CODE InitRoot(void);
ERROR_CODE InitCurrentDir(void);

ERROR_CODE GetParentDirectory(char *path, DIRECTORY_HEADER *p_dh, int *dir_block);
ERROR_CODE GetChildDirectory(char *name, DIRECTORY_HEADER *p_dh, DIRECTORY_HEADER *p_child_dh, int *dir_block);
ERROR_CODE GetChildFile(char *name, DIRECTORY_HEADER *p_dh, FILE_HEADER *p_fh, int *file_block);
ERROR_CODE GetSearchStart(char *path, DIRECTORY_DESCRIPTOR **p_dd);
ERROR_CODE InsertToDirectory(DIRECTORY_HEADER *dh, int block, HEADER *hh, int header_block);

ERROR_CODE PreProcessPath(char *path);

ERROR_CODE NewEmptyItem(char *name, __int64 size, DIRECTORY_HEADER *dh, int dir_block, HEADER *hh, int *file_block);
ERROR_CODE DeleteItem(DESCRIPTOR *d);
ERROR_CODE CloseItem(DESCRIPTOR *d);
ERROR_CODE FreeItemDiskSpace(int item_block);
inline int LookForFreeBlock(int size);
inline int LogicalToPhysical(FILE_HEADER *fh, int logical_block);

////////////////////////////////////////////////////////////////////////////////////////////////////////
// ��
#define MAKE_VERSION(major, minor)	((major << 16) | minor)
#define MAKE_ACCESS(r, w, x)		((r << 2) | (w << 1) | x)

#endif