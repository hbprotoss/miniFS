#ifndef MINI_APP_H
#define MINI_APP_H

#include "miniError.h"
#include "miniFile.h"

ERROR_CODE mount(char *file_name);
ERROR_CODE close(void);
ERROR_CODE fmt(void);
ERROR_CODE mkdir(char *path);
ERROR_CODE cd(char *path);
ERROR_CODE dr(char *path);
ERROR_CODE cp(char *src, char *dst);
ERROR_CODE dl(char *file_name);
ERROR_CODE tp(char *file_name);
ERROR_CODE more(char *file_name);
ERROR_CODE att(char *file_name);
ERROR_CODE opt(void);
ERROR_CODE cls(void);
ERROR_CODE sys(void);

void help(char *cmd);

/////////////////////////////////////////////////////////////////////////////////////////////////
// ��������


// ÿ1000�����һ�ν�����
#define BAR_UPDATE	1000
// ��������ʽ��[==>      ] XX.X%
typedef struct _BAR_INFO {
	char bar[256];			// ����������
	char progress_chr;		// �����ַ�
	char arrow_chr;			// ��ͷ�ַ�
	int width;			// "[]"֮��Ŀ��
	float ratio;			// ���ȱ���
	float last_pos;			// ��һ��λ�ã����ڻ���
	int str_ratio_offset;		// �ٷֱ��ַ�����ʼλ�õ��±�
} BAR_INFO;
void InitBar(BAR_INFO *bi, int width, float init_ratio);
void SetBarPos(BAR_INFO *bi, float new_ratio);
void PrintBar(BAR_INFO *bi);

inline __int64 GetFileSize(FILE *fp);

int WildcardMatches(char *wildcard, char *str);

ERROR_CODE att_print(char *file_name);

#endif