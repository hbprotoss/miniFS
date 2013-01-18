#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "miniCommandParsing.h"
#include "miniApp.h"
#include "miniError.h"

/*
* 函数名：ExecuteCommand
*功能：命令执行，并输出执行结果
* 参数：Cmd，存储所要执行的命令及命令参数
* 返回值：空
*/
void ExecuteCommand(Cmd cmd)
{
	ERROR_CODE err = ERR_SUCCESS;	//存储命令执行结果

	switch(cmd.type)	//执行命令
	{
		/***************************空命令*******************************/
		case TYPE_NO:
		{
			return ;
		};

		/***************************清屏命令*******************************/
		case TYPE_CLS:
		{
			err = cls();
		}break;

		/***************************命令：退出系统*******************************/
		case TYPE_CLOSE:
		{
			err = close();
		}break;

		/***************************显示系统信息命令*******************************/
		case TYPE_SYS:
		{
			err = sys();
		}break;

		/***************************命令：显示帮助信息*******************************/
		case TYPE_HEIP:
		{
			if(cmd.flag == 0)
				help(NULL);
			else
				help(cmd.para1);
		}break;

		/***************************命令：格式化系统*******************************/
		case TYPE_FMT:
		{
			EXEC ord = Confirm();	//确定执行询问
			if(ord == EXEC_Y)			//执行格式化命令
				err = fmt();
			else		//不执行格式化命令
				return ;
		}break;

		/***************************命令：优化存储空间*******************************/
		case TYPE_OPT:
		{
			EXEC ord = Confirm();	//确定执行询问
			if(ord == EXEC_Y)			//执行优化命令
				err = opt();
			else		//不执行格式化命令
				return ;
		}break;

		/***************************命令：建立目录*******************************/
		case TYPE_MKDIR:
		{
			err = mkdir(cmd.para1);
		}break;

		/***************************命令：切换目录*******************************/
		case TYPE_CD:
		{
			err = cd(cmd.para1);
		}break;

		/***************************命令：显示目录*******************************/
		case TYPE_DR:
		{
			if(cmd.flag == 0)
				err = dr(NULL);
			else
				err = dr(cmd.para1);
		}break;

		/***************************命令：显示子目录/文件属性*******************/
		case TYPE_ATT:
		{
			err = att(cmd.para1);
		}break;

		/***************************命令：显示文件内容**************************/
		case TYPE_TP:
		{
			err = tp(cmd.para1);
		}break;

		/***************************命令：分页显示文件内容*********************/
		case TYPE_MORE:
		{
			err = more(cmd.para1);
		}break;

		/***************************命令：拷贝文件*******************************/
		case TYPE_CP:
		{
			err = cp(cmd.para1,cmd.para2);
		}break;

		/***************************命令：删除子目录/文件***********************/
		case TYPE_DL:
		{
			EXEC ord = Confirm();	//确定执行询问
			if(ord == EXEC_Y)			//执行删除命令
				err = dl(cmd.para1);
			else		//不执行删除命令
				return ;
		}break;

		/***************************命令不存在***********************/
		case TYPE_NOT_FOUND:
		{
			printf("Command not found! You can type 'help' to help.\n");
		}break;

		/***************************参数不符合要求***********************/
		case TYPE_INVALID_PARAMETER:
		{
			printf("Invalid parameter! You can type 'help command' to help.\n");
		}break;

		/***************************缓冲区溢出***********************/
		case TYPE_BUFFER_OVERFLOW:
		{
			printf("你认为有这么长的命令吗！禁止欺负苦逼又敬业的程序猴子！！！\n");
		}break;

		/***************************未知情况***********************/
		default:
		{
			printf("未知错误！请联系这群苦逼又敬业的程序猴子（Nerv开发组）：youjianghulsk@gmail.com，看看发生了什么！\n");
		}
	}

	switch(err)		//输出执行结果
	{
		case ERR_SUCCESS:
			;	break;
		case ERR_TYPE_MISMATCH:	
			printf("Type mismatch！You can type 'help command' to help.\n");	break;
		case ERR_NOT_FOUND:
			printf("File or directory not found.\n");	break;
		case ERR_BUFFER_OVERFLOW:
			printf("Buffer overflow.\n");	break;
		case ERR_READ_ONLY:
			printf("File read only！You can type 'help command' to help.\n");	break;
		case ERR_OUT_OF_RANGE:
			printf("File pointer out of range！\n");	break;
		case ERR_UNKNOWN_FILE_SYSTEM:
			printf("Unkown file system！\n");	break;
		case ERR_INVALID_FILE_NAME:
			printf("Invalid file or dir name.\n");	break;
		case ERR_UNKNOWN_OPEN_MODE:
			printf("Unknown open mode！\n");	break;;	break;
		case ERR_NOT_ENOUGH_DISK_SPACE:
			printf("There is no enough disk space.\n");	break;
		case ERR_NOT_FORMATED:
			printf("System is not formated.\n");	break;
		case ERR_OTHER:
			printf("Unknown error！\n");	break;
		case ERR_DIRECTORY_NOT_EMPTY:
			printf("Directory not empty!\n");	break;
		case ERR_PATH_TOO_LONG:
			printf("Path too long!\n");	break;
		default:
			printf("Unknown error！请联系这群苦逼又敬业的程序猴子（Nerv开发组）：youjianghulsk@gmail.com，看看发生了什么！\n");
	}

	return ;
}
