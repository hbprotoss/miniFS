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
// 辅助函数


// 每1000块更新一次进度条
#define BAR_UPDATE	1000
// 进度条样式：[==>      ] XX.X%
typedef struct _BAR_INFO {
    char bar[256];            // 进度条容器
    char progress_chr;        // 进度字符
    char arrow_chr;            // 箭头字符
    int width;            // "[]"之间的宽度
    float ratio;            // 进度比率
    float last_pos;            // 上一次位置，用于绘制
    int str_ratio_offset;        // 百分比字符串开始位置的下标
} BAR_INFO;
void InitBar(BAR_INFO *bi, int width, float init_ratio);
void SetBarPos(BAR_INFO *bi, float new_ratio);
void PrintBar(BAR_INFO *bi);

inline long long GetFileSize(FILE *fp);

int WildcardMatches(char *wildcard, char *str);

ERROR_CODE att_print(char *file_name);

#endif