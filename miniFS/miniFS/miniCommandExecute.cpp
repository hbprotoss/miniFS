#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "miniCommandParsing.h"
#include "miniApp.h"
#include "miniError.h"

/*
* ��������ExecuteCommand
*���ܣ�����ִ�У������ִ�н��
* ������Cmd���洢��Ҫִ�е�����������
* ����ֵ����
*/
void ExecuteCommand(Cmd cmd)
{
	ERROR_CODE err = ERR_SUCCESS;	//�洢����ִ�н��

	switch(cmd.type)	//ִ������
	{
		/***************************������*******************************/
		case TYPE_NO:
		{
			return ;
		};

		/***************************��������*******************************/
		case TYPE_CLS:
		{
			err = cls();
		}break;

		/***************************����˳�ϵͳ*******************************/
		case TYPE_CLOSE:
		{
			err = close();
		}break;

		/***************************��ʾϵͳ��Ϣ����*******************************/
		case TYPE_SYS:
		{
			err = sys();
		}break;

		/***************************�����ʾ������Ϣ*******************************/
		case TYPE_HEIP:
		{
			if(cmd.flag == 0)
				help(NULL);
			else
				help(cmd.para1);
		}break;

		/***************************�����ʽ��ϵͳ*******************************/
		case TYPE_FMT:
		{
			EXEC ord = Confirm();	//ȷ��ִ��ѯ��
			if(ord == EXEC_Y)			//ִ�и�ʽ������
				err = fmt();
			else		//��ִ�и�ʽ������
				return ;
		}break;

		/***************************����Ż��洢�ռ�*******************************/
		case TYPE_OPT:
		{
			EXEC ord = Confirm();	//ȷ��ִ��ѯ��
			if(ord == EXEC_Y)			//ִ���Ż�����
				err = opt();
			else		//��ִ�и�ʽ������
				return ;
		}break;

		/***************************�������Ŀ¼*******************************/
		case TYPE_MKDIR:
		{
			err = mkdir(cmd.para1);
		}break;

		/***************************����л�Ŀ¼*******************************/
		case TYPE_CD:
		{
			err = cd(cmd.para1);
		}break;

		/***************************�����ʾĿ¼*******************************/
		case TYPE_DR:
		{
			if(cmd.flag == 0)
				err = dr(NULL);
			else
				err = dr(cmd.para1);
		}break;

		/***************************�����ʾ��Ŀ¼/�ļ�����*******************/
		case TYPE_ATT:
		{
			err = att(cmd.para1);
		}break;

		/***************************�����ʾ�ļ�����**************************/
		case TYPE_TP:
		{
			err = tp(cmd.para1);
		}break;

		/***************************�����ҳ��ʾ�ļ�����*********************/
		case TYPE_MORE:
		{
			err = more(cmd.para1);
		}break;

		/***************************��������ļ�*******************************/
		case TYPE_CP:
		{
			err = cp(cmd.para1,cmd.para2);
		}break;

		/***************************���ɾ����Ŀ¼/�ļ�***********************/
		case TYPE_DL:
		{
			EXEC ord = Confirm();	//ȷ��ִ��ѯ��
			if(ord == EXEC_Y)			//ִ��ɾ������
				err = dl(cmd.para1);
			else		//��ִ��ɾ������
				return ;
		}break;

		/***************************�������***********************/
		case TYPE_NOT_FOUND:
		{
			printf("Command not found! You can type 'help' to help.\n");
		}break;

		/***************************����������Ҫ��***********************/
		case TYPE_INVALID_PARAMETER:
		{
			printf("Invalid parameter! You can type 'help command' to help.\n");
		}break;

		/***************************���������***********************/
		case TYPE_BUFFER_OVERFLOW:
		{
			printf("����Ϊ����ô���������𣡽�ֹ�۸�����־�ҵ�ĳ�����ӣ�����\n");
		}break;

		/***************************δ֪���***********************/
		default:
		{
			printf("δ֪��������ϵ��Ⱥ����־�ҵ�ĳ�����ӣ�Nerv�����飩��youjianghulsk@gmail.com������������ʲô��\n");
		}
	}

	switch(err)		//���ִ�н��
	{
		case ERR_SUCCESS:
			;	break;
		case ERR_TYPE_MISMATCH:	
			printf("Type mismatch��You can type 'help command' to help.\n");	break;
		case ERR_NOT_FOUND:
			printf("File or directory not found.\n");	break;
		case ERR_BUFFER_OVERFLOW:
			printf("Buffer overflow.\n");	break;
		case ERR_READ_ONLY:
			printf("File read only��You can type 'help command' to help.\n");	break;
		case ERR_OUT_OF_RANGE:
			printf("File pointer out of range��\n");	break;
		case ERR_UNKNOWN_FILE_SYSTEM:
			printf("Unkown file system��\n");	break;
		case ERR_INVALID_FILE_NAME:
			printf("Invalid file or dir name.\n");	break;
		case ERR_UNKNOWN_OPEN_MODE:
			printf("Unknown open mode��\n");	break;;	break;
		case ERR_NOT_ENOUGH_DISK_SPACE:
			printf("There is no enough disk space.\n");	break;
		case ERR_NOT_FORMATED:
			printf("System is not formated.\n");	break;
		case ERR_OTHER:
			printf("Unknown error��\n");	break;
		case ERR_DIRECTORY_NOT_EMPTY:
			printf("Directory not empty!\n");	break;
		case ERR_PATH_TOO_LONG:
			printf("Path too long!\n");	break;
		default:
			printf("Unknown error������ϵ��Ⱥ����־�ҵ�ĳ�����ӣ�Nerv�����飩��youjianghulsk@gmail.com������������ʲô��\n");
	}

	return ;
}
