#ifndef PARSING_COMMAND_H
#define PARSING_COMMAND_H

typedef int TYPE;
typedef int EXEC;

#define	TYPE_CLOSE			0x00		//关闭系统命令
#define	TYPE_HEIP			0x01		//帮助命令
#define TYPE_SYS			0x02		//显示系统信息
#define	TYPE_FMT			0x03		//格式化命令
#define	TYPE_OPT			0x04		//优化空间命令
#define TYPE_CLS			0x05		//清屏命令
#define TYPE_MKDIR			0x06		//建立目录命令
#define	TYPE_CD				0x07		//切换目录命令
#define	TYPE_DR				0x08		//显示目录命令
#define TYPE_ATT			0x09		//显示文件属性命令
#define TYPE_TP				0x0a		//显示文件命令
#define TYPE_MORE			0x0b		//分页显示文件命令
#define TYPE_CP				0x0c		//拷贝命令
#define TYPE_DL				0x0d		//删除命令
#define TYPE_NOT_FOUND		0x0e		//命令不存在
#define TYPE_INVALID_PARAMETER	0x0f	//命令参数不符
#define TYPE_BUFFER_OVERFLOW	0x10	//缓冲区溢出
#define TYPE_NO				0x11		//未读入命令


#define EXEC_Y		0x00				//命令执行
#define EXEC_N		0x01				//命令不执行

typedef struct Command		//存储命令解析结果
{
	TYPE type;		//命令类型
	int flag;		//参数标记
	char para1[260];	//参数一
	char para2[260];	//参数二
}Cmd;

Cmd ParsingCommand( );		//解析命令函数
EXEC Confirm();		//命令执行性询问
void ExecuteCommand(Cmd cmd);		//执行命令函数

#endif