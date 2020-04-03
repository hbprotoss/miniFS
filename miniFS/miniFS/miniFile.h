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
// 文件系统

typedef unsigned char Bitmap;

/*
* 组：把位图的n个字节分成m个组，每个组n/m个字节
* 群：几个组构成一个群，用来分配指定文件大小上限以下的文件
*/
typedef struct _SET {
    int amount_of_group;    // 群内组的个数
    int free_block;        // 空闲块个数
    int max_size;        // 分配的文件大小上限（单位：块）
    int begin;        // bitmap内起始地址（单位：字节）
    int end;        // bitmap内终止地址（单位：字节）
    int last_search;    // 群内上次搜索终止地址（单位：字节）
} SET, *PSET;

// 文件系统头
typedef struct _FILE_SYSTEM_HEADER {
    char magic[4];            // 文件系统标识符
    int version;            // 文件系统版本号
    int amount_of_block;        // 总块数
    int amount_of_free_block;    // 空闲块数
    int root_address;        // 根目录文件块号
    int bitmap_address;        // 位图起始块号
    int bitmap_size;        // 位图大小（单位：块）
    int group_size;            // 索引组需要的块数
    int amount_of_set;        // 群（SET）总数
    SET set[1];            // 群（SET）列表
} FILE_SYSTEM_HEADER, *PFILE_SYSTEM_HEADER;

////////////////////////////////////////////////////////////////////////////////////////////
// 文件

// 文件分配表
typedef struct _FILE_ALLOCATE_TABLE {
    int amount_of_block;        // 表中块总数
    int block_number[1];        // 每个块入口编号
} FILE_ALLOCATE_TABLE, *PFILE_ALLOCATE_TABLE;

// 目录项
typedef struct _DIRECTORY_ENTRY {
    int block_number;        // 目录文件块编号。
    short is_file;            // 是否是文件
    char name[MAX_NAME];        // 目录名
} DIRECTORY_ENTRY, *PDIRECTORY_ENTRY;

// 目录分配表
typedef struct _DIRECTORY_ALLOCATE_TABLE {
    int amount_of_entry;        // 表中目录项总数
    DIRECTORY_ENTRY entry[1];    // 目录项表
} DIRECTORY_ALLOCATE_TABLE, *PDIRECTORY_ALLOCATE_TABLE;

typedef unsigned char FLAG_DIR_FILE;
#define	IS_DIR		0
#define IS_FILE		1

// 属性头
typedef struct _HEADER {
    char name[MAX_NAME];        // 文件名
    time_t create_time;        // 创建时间
    time_t modified_time;    // 修改时间
    time_t access_time;        // 访问时间
    long long file_size;        // 文件大小
    char hidden;            // 是否隐藏
    FLAG_DIR_FILE is_file;        // 文件/目录标记
    char access;            // 访问权限
    int allocate_table_number;    // 文件分配表总数
    int table_block_number[1];    // 文件分配表
} HEADER, *PHEADER;

// 文件属性头
typedef HEADER FILE_HEADER;
// 目录属性头
typedef HEADER DIRECTORY_HEADER;

// 描述符
typedef struct _DESCRIPTOR {
    long long offset;                // 文件偏移量
    int dir_block;                // 目录头块号
    DIRECTORY_HEADER *dir;            // 所在目录文件指针
    int self_block;                // 文件头块号
    HEADER *self;                // 文件头指针
    void *table_buffer;            // 文件/目录分配表缓存（一个块大小，防止频繁malloc）
} DESCRIPTOR, *PDESCRIPTOR;

// 文件描述符
typedef DESCRIPTOR FILE_DESCRIPTOR;
// 目录描述符
typedef DESCRIPTOR DIRECTORY_DESCRIPTOR;

#pragma pack(pop)

ERROR_CODE miniCreateFile(char *path, long long size, char *mode, FILE_DESCRIPTOR **p_fd);

ERROR_CODE miniReadFile(FILE_DESCRIPTOR *fd, long long size, int buffer_size, void *buffer, long long *size_read);

ERROR_CODE miniWriteFile(FILE_DESCRIPTOR *fd, long long size, int buffer_size, void *buffer, long long *size_write);

ERROR_CODE miniSeekFile(FILE_DESCRIPTOR *fd, long long offset);

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
// 辅助函数
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

ERROR_CODE NewEmptyItem(char *name, long long size, DIRECTORY_HEADER *dh, int dir_block, HEADER *hh, int *file_block);

ERROR_CODE DeleteItem(DESCRIPTOR *d);
ERROR_CODE CloseItem(DESCRIPTOR *d);
ERROR_CODE FreeItemDiskSpace(int item_block);
inline int LookForFreeBlock(int size);
inline int LogicalToPhysical(FILE_HEADER *fh, int logical_block);

////////////////////////////////////////////////////////////////////////////////////////////////////////
// 宏
#define MAKE_VERSION(major, minor)	((major << 16) | minor)
#define MAKE_ACCESS(r, w, x)		((r << 2) | (w << 1) | x)

#endif