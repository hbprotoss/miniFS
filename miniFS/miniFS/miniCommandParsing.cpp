#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "miniCommandParsing.h"

/*
* ��������ParsingCommand
*���ܣ������������Ϸ��Լ�飨������ڣ���������������������ȣ����������ͽ����������������
* ��������
* ����ֵ��Cmd�����ڴ洢����������
*/
Cmd ParsingCommand( )
{
	char str[520];
	Cmd cmd;

	memset(str,'#',520);		//��ʼ������

	for( ; ; )		//��������ǰ��Ŀո�
	{
		str[0] = getchar();
		if(str[0] == ' ')
			continue;
		else
			break;
	}
	fgets(&str[1], 517, stdin);	//��������
	int len = strlen(str);		//���������

	/***************************���������*******************************/
	if(len == 517 && str[516] != '\n')		//���������������������������TYPE_BUFFER_OVERFLOW
	{
		fflush(stdin);
		cmd.type = TYPE_BUFFER_OVERFLOW;
		return cmd;
	}
	if(len != 1)
		str[len-1] = '\0';
	
	/***************************�����뻻�з�*******************************/
	if(strcmp(str, "\n") == 0)
	{
		cmd.type = TYPE_NO;
		return cmd;
	}

	/***************************����˳�ϵͳ*******************************/
	if(strcmp(str, "close") == 0 || memcmp(str,"close ",6) == 0)
	{
		if(str[5] == '\0')	//����������������TYPE_CLOSE
		{
			cmd.type = TYPE_CLOSE;
			return cmd;
		}
		else	//������������������������TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
	}

	/***************************��������*******************************/
	if(strcmp(str, "cls") == 0 || memcmp(str,"cls ",4) == 0)
	{
		if(str[3] == '\0')	//����������������TYPE_CLS
		{
			cmd.type = TYPE_CLS;
			return cmd;
		}
		else	//������������������������TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
	}

	/***************************��������*******************************/
	if(strcmp(str, "sys") == 0 || memcmp(str,"sys ",4) == 0)
	{
		if(str[3] == '\0')	//����������������TYPE_SYS
		{
			cmd.type = TYPE_SYS;
			return cmd;
		}
		else	//������������������������TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
	}

	/***************************�����ʾ������Ϣ*******************************/
	if(strcmp(str,"help") == 0 ||  memcmp(str,"help ",5) == 0)
	{
		if(str[4] == '\0')	//�޲���������������������TYPE_HEIP
		{
			cmd.flag = 0;		cmd.type = TYPE_HEIP;
			return cmd;
		}
		else
		{
			int count = 0;
			for(int i=5; i<len; i++)
				if(str[i] == ' ')  ++count;
			if(count == 0 && len-1 > 5)		//��һ������������������������TYPE_HEIP
			{
				strncpy(cmd.para1, &str[5], len-5);
				cmd.flag = 1;		cmd.type = TYPE_HEIP;
				return cmd;
			}
			else	//������������������������TYPE_INVALID_PARAMETER
			{
				cmd.type = TYPE_INVALID_PARAMETER;
				return cmd;
			}
		}
	}

	/***************************�����ʽ��ϵͳ*******************************/
	if(strcmp(str, "fmt") == 0 || memcmp(str,"fmt ",4) == 0)
	{
		if(str[3] == '\0')	//����������������TYPE_FMT
		{
			cmd.type = TYPE_FMT;
			return cmd;
		}
		else	//������������������������TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
	}

	/***************************����Ż��洢�ռ�*******************************/
	if(strcmp(str, "opt") == 0 || memcmp(str,"opt ",4) == 0)
	{
		if(str[3] == '\0')	//����������������TYPE_OPT
		{
			cmd.type = TYPE_OPT;
			return cmd;
		}
		else	//������������������������TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
	}

	/***************************�������Ŀ¼*******************************/
	if(strcmp(str, "mkdir") == 0 || memcmp(str,"mkdir ",6) == 0)
	{
		if(str[5] == '\0')	//�޲�������������������TYPE_INVALID_PARAMETER
		{
			cmd.flag = 0;	cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
		else
		{
			int count = 0;
			for(int i=6; i<len; i++)
				if(str[i] == ' ')  ++count;
			if(count == 0 && len-1 > 6)		//��һ������������������������TYPE_MKDIR
			{
				strncpy(cmd.para1, &str[6], len-6);
				if(strlen(cmd.para1)<257)
				{
					cmd.flag = 1;		cmd.type = TYPE_MKDIR;
					return cmd;
				}
				else	//������������������������TYPE_INVALID_PARAMETER
				{
					cmd.type = TYPE_INVALID_PARAMETER;
					return cmd;
				}
			}
			else	//������������������������TYPE_INVALID_PARAMETER
			{
				cmd.type = TYPE_INVALID_PARAMETER;
				return cmd;
			}
		}
	}

	/***************************����л�Ŀ¼*******************************/
	if(strcmp(str, "cd") == 0 || memcmp(str,"cd ",3) == 0)
	{
		if(str[2] == '\0')	//�޲�������������������TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
		else
		{
			int count = 0;
			for(int i=3; i<len; i++)
				if(str[i] == ' ')  ++count;
			if(count == 0 && len-1 > 3)		//��һ������������������������TYPE_CD
			{
				strncpy(cmd.para1, &str[3], len-3);
				if(strlen(cmd.para1)<257)
				{
					cmd.flag = 1;		cmd.type = TYPE_CD;
					return cmd;
				}
				else	//������������������������TYPE_INVALID_PARAMETER
				{
					cmd.type = TYPE_INVALID_PARAMETER;
					return cmd;
				}
			}
			else	//������������������������TYPE_INVALID_PARAMETER
			{
				cmd.type = TYPE_INVALID_PARAMETER;
				return cmd;
			}
		}
	}

	/***************************�����ʾĿ¼*******************************/
	if(strcmp(str, "dr") == 0 || memcmp(str,"dr ",3) == 0)
	{
		if(str[2] == '\0')	//�޲���������������������TYPE_DR
		{
			cmd.flag = 0;	cmd.type = TYPE_DR;
			return cmd;
		}
		else
		{
			int count = 0;
			for(int i=3; i<len; i++)
				if(str[i] == ' ')  ++count;
			if(count == 0 && len-1 > 3)		//��һ������������������������TYPE_DR
			{
				strncpy(cmd.para1, &str[3], len-3);
				if(strlen(cmd.para1)<257)
				{
					cmd.flag = 1;		cmd.type = TYPE_DR;
					return cmd;
				}
				else	//������������������������TYPE_INVALID_PARAMETER
				{
					cmd.type = TYPE_INVALID_PARAMETER;
					return cmd;
				}
			}
			else	//������������������������TYPE_INVALID_PARAMETER
			{
				cmd.type = TYPE_INVALID_PARAMETER;
				return cmd;
			}
		}
	}

	/***************************�����ʾ��Ŀ¼/�ļ�����*******************************/
	if(strcmp(str, "att") == 0 || memcmp(str,"att ",4) == 0)
	{
		if(str[3] == '\0')	//�޲�������������������TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
		{
			int count = 0;
			for(int i=4; i<len; i++)
				if(str[i] == ' ')  ++count;
			if(count == 0 && len-1 > 4)		//��һ������������������������TYPE_ATT
			{
				strncpy(cmd.para1, &str[4], len-4);
				if(strlen(cmd.para1)<257)
				{
					cmd.flag = 1;		cmd.type = TYPE_ATT;
					return cmd;
				}
				else	//������������������������TYPE_INVALID_PARAMETER
				{
					cmd.type = TYPE_INVALID_PARAMETER;
					return cmd;
				}
			}
			else	//������������������������TYPE_INVALID_PARAMETER
			{
				cmd.type = TYPE_INVALID_PARAMETER;
				return cmd;
			}
		}
	}

	/***************************�����ʾ�ļ�����*******************************/
	if(strcmp(str, "tp") == 0 || memcmp(str,"tp ",3) == 0)
	{
		if(str[2] == '\0')	//�޲�������������������TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
		{
			int count = 0;
			for(int i=3; i<len; i++)
				if(str[i] == ' ')  ++count;
			if(count == 0 && len-1 > 3)		//��һ������������������������TYPE_TP
			{
				strncpy(cmd.para1, &str[3], len-3);
				if(strlen(cmd.para1)<257)
				{
					cmd.flag = 1;		cmd.type = TYPE_TP;
					return cmd;
				}
				else	//������������������������TYPE_INVALID_PARAMETER
				{
					cmd.type = TYPE_INVALID_PARAMETER;
					return cmd;
				}
			}
			else	//������������������������TYPE_INVALID_PARAMETER
			{
				cmd.type = TYPE_INVALID_PARAMETER;
				return cmd;
			}
		}
	}

	/***************************�����ҳ��ʾ�ļ�����*******************************/
	if(strcmp(str, "more") == 0 || memcmp(str,"more ",5) == 0)
	{
		if(str[4] == '\0')	//�޲�������������������TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
		{
			int count = 0;
			for(int i=5; i<len; i++)
				if(str[i] == ' ')  ++count;
			if(count == 0 && len-1 > 5)		//��һ������������������������TYPE_MORE
			{
				strncpy(cmd.para1, &str[5], len-5);
				if(strlen(cmd.para1)<257)
				{
					cmd.flag = 1;		cmd.type = TYPE_MORE;
					return cmd;
				}
				else	//������������������������TYPE_INVALID_PARAMETER
				{
					cmd.type = TYPE_INVALID_PARAMETER;
					return cmd;
				}
			}
			else	//������������������������TYPE_INVALID_PARAMETER
			{
				cmd.type = TYPE_INVALID_PARAMETER;
				return cmd;
			}
		}
	}

	/***************************��������ļ�*******************************/
	if(strcmp(str, "cp") == 0 || memcmp(str,"cp ",3) == 0)
	{
		if(str[2] == '\0')	//�޲�������������������TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
		{
			int count = 0;
			int pos = 0;
			for(int i=3; i<len; i++)
				if(str[i] == ' ')  { ++count;  pos = i; }
			if(count == 1 && str[len-2] != ' ')		//����������������������������TYPE_CP
			{
				strncpy(cmd.para1, &str[3], pos-3);		cmd.para1[pos-3] = '\0';
				strncpy(cmd.para2, &str[pos+1], len-pos-1);

				if(strlen(cmd.para1)<257 && strlen(cmd.para2)<257)
				{
					cmd.flag = 1;		cmd.type = TYPE_CP;
					return cmd;
				}
				else	//������������������������TYPE_INVALID_PARAMETER
				{
					cmd.type = TYPE_INVALID_PARAMETER;
					return cmd;
				}
			}
			else	//������������������������TYPE_INVALID_PARAMETER
			{
				cmd.type = TYPE_INVALID_PARAMETER;
				return cmd;
			}
		}
	}

	/***************************���ɾ����Ŀ¼/�ļ�*******************************/
	if(strcmp(str, "dl") == 0 || memcmp(str,"dl ",3) == 0)
	{
		if(str[2] == '\0')	//�޲�������������������TYPE_INVALID_PARAMETER
		{
			cmd.type = TYPE_INVALID_PARAMETER;
			return cmd;
		}
		{
			int count = 0;
			for(int i=3; i<len; i++)
				if(str[i] == ' ')  ++count;
			if(count == 0 && len-1 > 3)		//��һ������������������������TYPE_DL
			{
				strncpy(cmd.para1, &str[3], len-3);
				if(strlen(cmd.para1)<257)
				{
					cmd.flag = 1;		cmd.type = TYPE_DL;
					return cmd;
				}
				else	//������������������������TYPE_INVALID_PARAMETER
				{
					cmd.type = TYPE_INVALID_PARAMETER;
					return cmd;
				}
			}
			else	//������������������������TYPE_INVALID_PARAMETER
			{
				cmd.type = TYPE_INVALID_PARAMETER;
				return cmd;
			}
		}
	}

	/***************************�������*******************************/
	{
		cmd.type = TYPE_NOT_FOUND;
		return cmd;
	}
}

/*
* ��������Confirm
*���ܣ�����ִ����ѯ��
* ��������
* ����ֵ��int��0Ϊִ�У�1Ϊ��ִ��
*/
EXEC Confirm()
{
	char ch1, ch2;
	for( ;  ;  )
	{
		printf("Are you sure to continue? Y or N: ");	//�����ʾ
		ch1 = getchar();
		if(ch1 == '\n')
			continue;
		else
		{
			ch2 = getchar();
			if( (ch1 == 'Y' || ch1 == 'y') && ch2 == '\n')	//ִ������
				return EXEC_Y;
			if( (ch1 == 'N' || ch1 == 'n') && ch2 == '\n')	//��ִ������
				return EXEC_N;
			fflush(stdin);
		}
	}
}
