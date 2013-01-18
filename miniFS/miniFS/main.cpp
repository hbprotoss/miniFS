#include <cstdio>
#include <cstdlib>

#include "miniCommandParsing.h"
#include "miniAPP.h"

extern char g_current_path[MAX_PATH_NAME];			// 全局当前目录字符串(from miniFile.cpp)
int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("参数格式不符。参数格式：mount 文件名\n");
		exit(1);
	}
	if(mount(argv[1]) == ERR_NOT_FOUND)
	{
		printf("虚拟磁盘未找到!\n");
		exit(1);
	}

	Cmd cmd;		//存储命令解析结果

	for(  ;  ;   )	//循环读入、解析并执行命令
	{
		printf("[%s ]:  ", g_current_path);
		cmd = ParsingCommand();		//读入并解析命令

		if(cmd.type == TYPE_CLOSE)			//退出命令
		{
			EXEC ord = Confirm();	//确定执行询问
			if(ord == EXEC_Y)			//执行退出命令
			{
				ExecuteCommand(cmd);
				break;
			}
			else		//不执行退出命令
				continue;
		}
		else			//其他命令
		{
				ExecuteCommand(cmd);	//执行命令
		}
	}

	return 0;
}