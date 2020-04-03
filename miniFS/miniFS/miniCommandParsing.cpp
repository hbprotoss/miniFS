#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "miniCommandParsing.h"

/*
* 函数名：ParsingCommand
*功能：读入命令，命令合法性检查（命令不存在，参数不符，缓冲区溢出等），命令类型解析，命令参数解析
* 参数：空
* 返回值：Cmd，用于存储命令解析结果
*/
Cmd ParsingCommand( ) {
	char str[520];
	Cmd cmd;

	memset(str, '#', 520);        //初始化数组

	for (;;)        //读入命令前面的空格
	{
		str[0] = getchar();
		if (str[0] == ' ')
			continue;
		else
			break;
	}
	fgets(&str[1], 517, stdin);    //读入命令
	int len = strlen(str);        //计算命令长度

	/***************************缓冲区溢出*******************************/
	if (len == 517 && str[516] != '\n')        //缓冲区溢出，返回命令解析结果：TYPE_BUFFER_OVERFLOW
	{
		fflush(stdin);
		cmd.type = TYPE_BUFFER_OVERFLOW;
		return cmd;
	}
	if (len != 1)
		str[len - 1] = '\0';

	/***************************仅读入换行符*******************************/
	if (strcmp(str, "\n") == 0) {
		cmd.type = TYPE_NO;
		return cmd;
	}

	/***************************命令：退出系统*******************************/
	if (strcmp(str, "close") == 0 || memcmp(str, "close ", 6) == 0) {
		if (str[5] == '\0')    //返回命令解析结果：TYPE_CLOSE
		{
			cmd.type = TYPE_CLOSE;
			return cmd;
		} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
	}

	/***************************清屏命令*******************************/
	if (strcmp(str, "cls") == 0 || memcmp(str, "cls ", 4) == 0) {
		if (str[3] == '\0')    //返回命令解析结果：TYPE_CLS
		{
			cmd.type = TYPE_CLS;
			return cmd;
		} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
	}

	/***************************清屏命令*******************************/
	if (strcmp(str, "sys") == 0 || memcmp(str, "sys ", 4) == 0) {
		if (str[3] == '\0')    //返回命令解析结果：TYPE_SYS
		{
			cmd.type = TYPE_SYS;
			return cmd;
		} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
	}

	/***************************命令：显示帮助信息*******************************/
	if (strcmp(str, "help") == 0 || memcmp(str, "help ", 5) == 0) {
		if (str[4] == '\0')    //无参数。返回命令解析结果：TYPE_HEIP
		{
			cmd.flag = 0;
			cmd.type = TYPE_HEIP;
			return cmd;
		} else {
			int count = 0;
			for (int i = 5; i < len; i++)
				if (str[i] == ' ') ++count;
			if (count == 0 && len - 1 > 5)        //有一个参数。返回命令解析结果：TYPE_HEIP
			{
				strncpy(cmd.para1, &str[5], len - 5);
				cmd.flag = 1;
				cmd.type = TYPE_HEIP;
				return cmd;
			} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
			{
				cmd.type = TYPE_INVALID_PARAMETER;
				return cmd;
			}
		}
	}

	/***************************命令：格式化系统*******************************/
	if (strcmp(str, "fmt") == 0 || memcmp(str, "fmt ", 4) == 0) {
		if (str[3] == '\0')    //返回命令解析结果：TYPE_FMT
		{
			cmd.type = TYPE_FMT;
			return cmd;
		} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
	}

	/***************************命令：优化存储空间*******************************/
	if (strcmp(str, "opt") == 0 || memcmp(str, "opt ", 4) == 0) {
		if (str[3] == '\0')    //返回命令解析结果：TYPE_OPT
		{
			cmd.type = TYPE_OPT;
			return cmd;
		} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
	}

	/***************************命令：建立目录*******************************/
	if (strcmp(str, "mkdir") == 0 || memcmp(str, "mkdir ", 6) == 0) {
		if (str[5] == '\0')    //无参数，返回命令解析结果TYPE_INVALID_PARAMETER
		{
			cmd.flag = 0;
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		} else {
			int count = 0;
			for (int i = 6; i < len; i++)
				if (str[i] == ' ') ++count;
			if (count == 0 && len - 1 > 6)        //有一个参数。返回命令解析结果：TYPE_MKDIR
			{
				strncpy(cmd.para1, &str[6], len - 6);
				if (strlen(cmd.para1) < 257) {
					cmd.flag = 1;
					cmd.type = TYPE_MKDIR;
					return cmd;
				} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
				{
					cmd.type = TYPE_INVALID_PARAMETER;
					return cmd;
				}
			} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
			{
				cmd.type = TYPE_INVALID_PARAMETER;
				return cmd;
			}
		}
	}

	/***************************命令：切换目录*******************************/
	if (strcmp(str, "cd") == 0 || memcmp(str, "cd ", 3) == 0) {
		if (str[2] == '\0')    //无参数，返回命令解析结果TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		} else {
			int count = 0;
			for (int i = 3; i < len; i++)
				if (str[i] == ' ') ++count;
			if (count == 0 && len - 1 > 3)        //有一个参数。返回命令解析结果：TYPE_CD
			{
				strncpy(cmd.para1, &str[3], len - 3);
				if (strlen(cmd.para1) < 257) {
					cmd.flag = 1;
					cmd.type = TYPE_CD;
					return cmd;
				} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
				{
					cmd.type = TYPE_INVALID_PARAMETER;
					return cmd;
				}
			} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
			{
				cmd.type = TYPE_INVALID_PARAMETER;
				return cmd;
			}
		}
	}

	/***************************命令：显示目录*******************************/
	if (strcmp(str, "dr") == 0 || memcmp(str, "dr ", 3) == 0) {
		if (str[2] == '\0')    //无参数。返回命令解析结果：TYPE_DR
		{
			cmd.flag = 0;
			cmd.type = TYPE_DR;
			return cmd;
		} else {
			int count = 0;
			for (int i = 3; i < len; i++)
				if (str[i] == ' ') ++count;
			if (count == 0 && len - 1 > 3)        //有一个参数。返回命令解析结果：TYPE_DR
			{
				strncpy(cmd.para1, &str[3], len - 3);
				if (strlen(cmd.para1) < 257) {
					cmd.flag = 1;
					cmd.type = TYPE_DR;
					return cmd;
				} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
				{
					cmd.type = TYPE_INVALID_PARAMETER;
					return cmd;
				}
			} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
			{
				cmd.type = TYPE_INVALID_PARAMETER;
				return cmd;
			}
		}
	}

	/***************************命令：显示子目录/文件属性*******************************/
	if (strcmp(str, "att") == 0 || memcmp(str, "att ", 4) == 0) {
		if (str[3] == '\0')    //无参数，返回命令解析结果TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
		{
			int count = 0;
			for (int i = 4; i < len; i++)
				if (str[i] == ' ') ++count;
			if (count == 0 && len - 1 > 4)        //有一个参数。返回命令解析结果：TYPE_ATT
			{
				strncpy(cmd.para1, &str[4], len - 4);
				if (strlen(cmd.para1) < 257) {
					cmd.flag = 1;
					cmd.type = TYPE_ATT;
					return cmd;
				} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
				{
					cmd.type = TYPE_INVALID_PARAMETER;
					return cmd;
				}
			} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
			{
				cmd.type = TYPE_INVALID_PARAMETER;
				return cmd;
			}
		}
	}

	/***************************命令：显示文件内容*******************************/
	if (strcmp(str, "tp") == 0 || memcmp(str, "tp ", 3) == 0) {
		if (str[2] == '\0')    //无参数，返回命令解析结果TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
		{
			int count = 0;
			for (int i = 3; i < len; i++)
				if (str[i] == ' ') ++count;
			if (count == 0 && len - 1 > 3)        //有一个参数。返回命令解析结果：TYPE_TP
			{
				strncpy(cmd.para1, &str[3], len - 3);
				if (strlen(cmd.para1) < 257) {
					cmd.flag = 1;
					cmd.type = TYPE_TP;
					return cmd;
				} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
				{
					cmd.type = TYPE_INVALID_PARAMETER;
					return cmd;
				}
			} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
			{
				cmd.type = TYPE_INVALID_PARAMETER;
				return cmd;
			}
		}
	}

	/***************************命令：分页显示文件内容*******************************/
	if (strcmp(str, "more") == 0 || memcmp(str, "more ", 5) == 0) {
		if (str[4] == '\0')    //无参数，返回命令解析结果TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
		{
			int count = 0;
			for (int i = 5; i < len; i++)
				if (str[i] == ' ') ++count;
			if (count == 0 && len - 1 > 5)        //有一个参数。返回命令解析结果：TYPE_MORE
			{
				strncpy(cmd.para1, &str[5], len - 5);
				if (strlen(cmd.para1) < 257) {
					cmd.flag = 1;
					cmd.type = TYPE_MORE;
					return cmd;
				} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
				{
					cmd.type = TYPE_INVALID_PARAMETER;
					return cmd;
				}
			} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
			{
				cmd.type = TYPE_INVALID_PARAMETER;
				return cmd;
			}
		}
	}

	/***************************命令：拷贝文件*******************************/
	if (strcmp(str, "cp") == 0 || memcmp(str, "cp ", 3) == 0) {
		if (str[2] == '\0')    //无参数，返回命令解析结果TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
		{
			int count = 0;
			int pos = 0;
			for (int i = 3; i < len; i++)
				if (str[i] == ' ') {
					++count;
					pos = i;
				}
			if (count == 1 && str[len - 2] != ' ')        //有两个参数。返回命令解析结果：TYPE_CP
			{
				strncpy(cmd.para1, &str[3], pos - 3);
				cmd.para1[pos - 3] = '\0';
				strncpy(cmd.para2, &str[pos + 1], len - pos - 1);

				if (strlen(cmd.para1) < 257 && strlen(cmd.para2) < 257) {
					cmd.flag = 1;
					cmd.type = TYPE_CP;
					return cmd;
				} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
				{
					cmd.type = TYPE_INVALID_PARAMETER;
					return cmd;
				}
			} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
			{
				cmd.type = TYPE_INVALID_PARAMETER;
				return cmd;
			}
		}
	}

	/***************************命令：删除子目录/文件*******************************/
	if (strcmp(str, "dl") == 0 || memcmp(str, "dl ", 3) == 0) {
		if (str[2] == '\0')    //无参数，返回命令解析结果TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
		{
			int count = 0;
			for (int i = 3; i < len; i++)
				if (str[i] == ' ') ++count;
			if (count == 0 && len - 1 > 3)        //有一个参数。返回命令解析结果：TYPE_DL
			{
				strncpy(cmd.para1, &str[3], len - 3);
				if (strlen(cmd.para1) < 257) {
					cmd.flag = 1;
					cmd.type = TYPE_DL;
					return cmd;
				} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
				{
					cmd.type = TYPE_INVALID_PARAMETER;
					return cmd;
				}
			} else    //参数不符，返回命令解析结果TYPE_INVALID_PARAMETER
			{
				cmd.type = TYPE_INVALID_PARAMETER;
				return cmd;
			}
		}
	}

	/***************************命令不存在*******************************/
	{
		cmd.type = TYPE_NOT_FOUND;
		return cmd;
	}
}

/*
* 函数名：Confirm
*功能：命令执行性询问
* 参数：空
* 返回值：int。0为执行，1为不执行
*/
EXEC Confirm()
{
	char ch1, ch2;
	for( ;  ;  ) {
		printf("Are you sure to continue? Y or N: ");    //输出提示
		ch1 = getchar();
		if (ch1 == '\n')
			continue;
		else {
			ch2 = getchar();
			if ((ch1 == 'Y' || ch1 == 'y') && ch2 == '\n')    //执行命令
				return EXEC_Y;
			if ((ch1 == 'N' || ch1 == 'n') && ch2 == '\n')    //不执行命令
				return EXEC_N;
			fflush(stdin);
		}
	}
}
