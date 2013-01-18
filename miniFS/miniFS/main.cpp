#include <cstdio>
#include <cstdlib>

#include "miniCommandParsing.h"
#include "miniAPP.h"

extern char g_current_path[MAX_PATH_NAME];			// ȫ�ֵ�ǰĿ¼�ַ���(from miniFile.cpp)
int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("������ʽ������������ʽ��mount �ļ���\n");
		exit(1);
	}
	if(mount(argv[1]) == ERR_NOT_FOUND)
	{
		printf("�������δ�ҵ�!\n");
		exit(1);
	}

	Cmd cmd;		//�洢����������

	for(  ;  ;   )	//ѭ�����롢������ִ������
	{
		printf("[%s ]:  ", g_current_path);
		cmd = ParsingCommand();		//���벢��������

		if(cmd.type == TYPE_CLOSE)			//�˳�����
		{
			EXEC ord = Confirm();	//ȷ��ִ��ѯ��
			if(ord == EXEC_Y)			//ִ���˳�����
			{
				ExecuteCommand(cmd);
				break;
			}
			else		//��ִ���˳�����
				continue;
		}
		else			//��������
		{
				ExecuteCommand(cmd);	//ִ������
		}
	}

	return 0;
}