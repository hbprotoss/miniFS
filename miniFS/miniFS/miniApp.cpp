#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <conio.h>

#include <map>
#include <string>
#include <iostream>

#include <Windows.h>

#include "miniApp.h"
#include "miniFile.h"
#include "miniBlock.h"
#include "miniError.h"


extern FILE *g_storage;						// OS�ϵ������ļ���miniBlock.cpp��
extern FILE_SYSTEM_HEADER *g_file_system_header;		// �ļ�ϵͳͷ����
extern Bitmap *g_bitmap;					// λͼ����
extern char g_current_path[MAX_PATH_NAME];			// ȫ�ֵ�ǰĿ¼�ַ���(from miniFile.cpp)

char file_buf[BLOCK_SIZE];					// �ļ�������
std::map<std::string,std::string> maplist;			// ������Ϣmap����

HANDLE hStdout;							// stdout���

/*
* ��������mount
*���ܣ�����ϵͳ�������������øýӿ�����ϵͳ��
* �����������ļ�������ϵͳ����
* ����ֵ��ERROR_CODE����ʾִ�н����
*/
ERROR_CODE mount(char *file_name)
{
	g_storage = fopen(file_name, "rb+");
	if(g_storage == NULL)
	{
		DbgPrint(ERR_NOT_FOUND);
		return ERR_NOT_FOUND;
	}
	setvbuf(g_storage, file_buf, _IOFBF, BLOCK_SIZE);

	ERROR_CODE status = miniInitFileSystem();
	while(status != ERR_SUCCESS)
	{
		if(status == ERR_NOT_FORMATED)
		{
			char ans;
			printf("File system cannot be used before formated! Do you want to format now?(y/n)");
			ans = getchar();
			getchar();
			if(ans == 'y' || ans == 'Y')
			{
				miniFormat();
				status = miniInitFileSystem();
			}
			else
			{
				printf("Fatal error: file system not formated!\n");
				printf("System now exit!\n");
				exit(1);
			}
		}
		else if(status != ERR_SUCCESS)
		{
			DbgPrint(status);
			return status;
		}
	}

	using std::map;
	using std::string;
	using std::pair;
	// �����ַ�����Դ
	maplist.insert(pair<string,string>("create","create wFName [size]\t����mini-uFS�ļ���size��λ��GB��"));
	maplist.insert(pair<string,string>("mount","mount wFName\t\t���� mini-uFS ϵͳ��"));
	maplist.insert(pair<string,string>("fmt","fmt\t\t\t��ʽ��ϵͳ��"));
	maplist.insert(pair<string,string>("dr","dr [drName]\t\t��ʾ��ǰ��ָ��Ŀ¼��"));
	maplist.insert(pair<string,string>("cp","cp FName1 FName2\t���ļ�FName1������FName2��"));
	maplist.insert(pair<string,string>("dl","dl uFName\t\tɾ��ָ���ļ���"));
	maplist.insert(pair<string,string>("tp","tp uFName\t\t��ʾָ���ļ����ݡ�"));
	maplist.insert(pair<string,string>("more","more uFName\t\t��ҳ��ʾָ���ļ����ݡ�"));
	maplist.insert(pair<string,string>("att","att uFName\t\t��ʾ�ļ����ԡ�"));
	maplist.insert(pair<string,string>("close","close\t\t\t�˳�ϵͳ��"));
	maplist.insert(pair<string,string>("opt","opt\t\t\t�Ż�����ʹ�ÿռ䡣"));
	maplist.insert(pair<string,string>("cd","cd drName\t\t�л���ָ��Ŀ¼��"));
	maplist.insert(pair<string,string>("mkdir", "mkdir dir\t\t�½�Ŀ¼"));
	maplist.insert(pair<string,string>("help","help [cmd]\t\t��ʾ����İ�����Ϣ��"));
	maplist.insert(pair<string,string>("cls","cls\t\t\t����"));
	return ERR_SUCCESS;
}

/*
* ��������close
*���ܣ��˳�ϵͳ�������������øýӿ��˳�ϵͳ��
* �������ޡ�
* ����ֵ��ERROR_CODE����ʾִ�н����
*/
ERROR_CODE close(void)
{
	ERROR_CODE err = ERR_OTHER;
	err = miniExitSystem(  );		//�����ļ���miniExitSystem�ӿ�
	return err;
}

/*
* ��������cls
*���ܣ�������
* �������ޡ�
* ����ֵ��ERROR_CODE����ʾִ�н����
*/
ERROR_CODE cls(void)
{
	system("cls");
	return ERR_SUCCESS;
}

/*
* ��������sys
*���ܣ���ʾϵͳ��Ϣ��
* �������ޡ�
* ����ֵ��ERROR_CODE����ʾִ�н����
*/
ERROR_CODE sys(void)
{
	ERROR_CODE err = ERR_OTHER;

	char magic[10];
	int version_major;
	int version_minor;
	int num_free_block;

	err = miniGetSystemInfo(magic, &version_major, &version_minor, &num_free_block);
	if(err == ERR_SUCCESS)
	{
		printf("Magic:			%s\n",magic);
		printf("Version:		%d.%d\n",version_major,version_minor);
		printf("Free space:		%dM\n",num_free_block);
	}

	return err;
}

/*
* ��������help
*���ܣ���ʾ�����������������øýӿ���ʾ���������
* �������ַ������洢Ҫ��ʾ���������������ַ���Ϊ��ʱ��ʾ�������������
* ����ֵ���ޡ�
*/
void help(char *cmd)
{
	using std::map;
	using std::string;
	using std::pair;
	using std::cout;
	using std::endl;

	map<string,string>::iterator iter;
	if(cmd==NULL)			//����Ϊ�գ����������˵��
	{
		for(iter=maplist.begin();iter!=maplist.end();iter++)
		{
			cout<<iter->second << endl;
		}
	}
	else					//�в��������������˵��
	{
		iter=maplist.find(cmd);
		if(iter!=maplist.end())
		{
			cout << maplist[cmd] << endl;
		}
		else		//������ָ�������
		{
			cout<<"Command type mismatch��You can type 'help command' for help."<<endl;
		}
	}
}

/*
* ��������fmt
*���ܣ���ʽ��ϵͳ�������������øýӿڸ�ʽ��ϵͳ��
* �������ޡ�
* ����ֵ��ERROR_CODE����ʾִ�н����
*/
ERROR_CODE fmt(void)
{
	ERROR_CODE err = ERR_OTHER;
	err = miniFormat( );		//�����ļ���miniFormat�ӿ�
	if(err == ERR_SUCCESS)
		miniChangeCurrentDirectory("/");
	return err;
}

/*
* ��������opt
*���ܣ��Ż����̴洢�ռ䡣�����������øýӿ��Ż��洢�ռ䡣
* �������ޡ�
* ����ֵ��ERROR_CODE����ʾִ�н��
*/
ERROR_CODE opt(void)
{
	ERROR_CODE err = ERR_OTHER;
	//err = miniOptimize();		//�����ļ���miniOptimize�ӿ�
	return err;
}

/*
* ��������mkdir
*���ܣ�����Ŀ¼�������������øýӿڽ���Ŀ¼��
* �������ַ������洢·��������·�������·������
* ����ֵ��ERROR_CODE����ʾִ�н����
*/
ERROR_CODE mkdir(char *path)
{
	ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;

	DIRECTORY_DESCRIPTOR * dDes;
	err_flag = miniCreateDirectory(path, "r", &dDes);	//��Ŀ¼
	if(err_flag == ERR_SUCCESS)		//��Ŀ¼�ɹ�����Ŀ¼����ͻ
	{
		err_mid = miniCloseDirectory(dDes);		//�ر��ļ�
		printf("Directory exists!\n");
		return ERR_SUCCESS;
	}
	if(err_flag == ERR_TYPE_MISMATCH)		//�򿪵�Ŀ��Ŀ¼Ϊ�ļ�����Ŀ¼����ͻ
	{
		printf("\"%s\" is a file. Cannot create directory!\n", path);
		return ERR_SUCCESS;
	}

	if(err_flag == ERR_NOT_FOUND)		//�򿪵�Ŀ��Ŀ¼δ�ҵ����ɴ�����Ŀ¼
	{
		err_ret = miniCreateDirectory(path, "w", &dDes);		//����Ŀ¼
		return  err_ret;
	}
	else
		return err_flag;
}

/*
* ��������cd
*���ܣ��л�Ŀ¼�������������øýӿ��л�Ŀ¼��
* �������ַ������洢·��������·�������·������
* ����ֵ��ERROR_CODE����ʾִ�н����
*/
ERROR_CODE cd(char *path)
{
	ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;

	DIRECTORY_DESCRIPTOR * dDes;
	err_flag = miniCreateDirectory(path, "r", &dDes);	//��Ŀ¼
	if(err_flag == ERR_SUCCESS)		//��Ŀ¼�ɹ�
	{
		err_ret = miniChangeCurrentDirectory(path);	//�л�Ŀ¼
		err_mid = miniCloseDirectory(dDes);		//�ر�Ŀ¼
		return err_ret;		//����״̬����
	}
	else
		return err_flag;
}

/*
* ��������cd
*���ܣ���ʾĿ¼�������������øýӿ���ʾĿ¼���ݡ�
* �������ַ������洢·��������·�������·���������ַ���Ϊ��ʱ��ʾ��ǰĿ¼���ݡ�
* ����ֵ��ERROR_CODE����ʾִ�н����
*/
ERROR_CODE dr(char *path)
{
	ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;

	DIRECTORY_DESCRIPTOR * dDes;
	err_flag = miniCreateDirectory(path, "r", &dDes);	//��Ŀ¼
	if(err_flag == ERR_SUCCESS)		//��Ŀ¼�ɹ�
	{
		int size = 0;
		if(miniReadDirectory(dDes, &size, NULL) == ERR_BUFFER_OVERFLOW)		//��̽Ŀ¼����
		{
			DIRECTORY_ENTRY *entry = (DIRECTORY_ENTRY *)calloc(1, size);
			err_ret = miniReadDirectory(dDes, &size, entry);		//��ȡĿ¼��
			for(int i = 0; i < size; i++)		//���Ŀ¼����
			{
				printf("%s\t\t%s\n", entry[i].name, (entry[i].is_file ? "FILE" : "DIR"));
			}
			free(entry);
		}

		err_mid = miniCloseDirectory(dDes);		//�ر�Ŀ¼
		if(size == 0)		//Ŀ¼Ϊ��
			printf("Directory is empty!\n");
		return err_ret;		//����״̬����
	}
	else
		return err_flag;
}

/*
* ��������att
*���ܣ���ʾĿ¼���ļ����ԡ�
* �������ַ������洢Ŀ¼���ļ�����Ϊ����·�������·�����ļ��������ֱ��Ϊ�ļ�����Ĭ�ϵ�ǰĿ¼����
* ����ֵ��ERROR_CODE����ʾִ�н����
*/
ERROR_CODE att(char *file_name)
{
	ERROR_CODE err_ret = ERR_OTHER;

	/*****************�Ƿ���ͨ����ж�*********************/
	int len = strlen(file_name);
	int pos  = 0;
	for(int i=0; i<len; i++)
	{
		if(file_name[i] == '/')
			pos = i;
	}
	int flag = 0;
	for(int i=pos; i<len; i++)
	{
		if(file_name[i] == '*' || file_name[i] == '?')
			flag =1;
	}

	/*****************��ʾĿ¼���ļ�����*********************/
	if(flag == 0)		//����ͨ���
	{
		err_ret = att_print(file_name);
		return err_ret;
	}
	else		//����ͨ���
	{
		if(pos == 0)		//���·����ͨ���
		{
			DIRECTORY_DESCRIPTOR * dDes;
			err_ret = miniCreateDirectory(g_current_path, "r", &dDes);	//��Ŀ¼
			if(err_ret != ERR_SUCCESS)		//�򿪲�Ŀ¼�ɹ�
				return err_ret;
			else		//��Ŀ¼�ɹ�
			{
				int size = 0;
				int flag_have = 0;
				if(miniReadDirectory(dDes, &size, NULL) == ERR_BUFFER_OVERFLOW)		//��̽Ŀ¼����
				{
					DIRECTORY_ENTRY *entry = (DIRECTORY_ENTRY *)calloc(1, size);
					err_ret = miniReadDirectory(dDes, &size, entry);		//��ȡĿ¼��
					for(int i = 0; i < size; i++)		//���Ŀ¼��ƥ�䣬����
					{
						int mark = 0;
						mark = WildcardMatches(file_name, entry[i].name);
						if(mark == 1)		//ƥ��ɹ�
						{
							flag_have = 1;
							err_ret = att_print(entry[i].name);		//�����Ŀ¼������
							printf("\n");
							if(err_ret != ERR_SUCCESS)
								return err_ret;
						}
						else
							continue;
					}
					free(entry);
				}
				else
					return miniReadDirectory(dDes, &size, NULL);

				if(flag_have == 0)		//���κ�һ����ƥ��
					printf("No one is matching\n");
				return ERR_SUCCESS;
			}
		}
		else		//����·����ͨ���
		{
			char path[550];
			char name[260];
			for(int i=0; i<pos; i++)		//��ȡĿ¼
				path[i] = file_name[i];
			for(int i=pos+1; i<len; i++)	//��ȡ��ͨ������ļ���
				name[i-pos-1] = file_name[i];
			path[pos] = '\0';  name[len-pos-1] = '\0';

			DIRECTORY_DESCRIPTOR * dDes;
			char path_cp[520];
			strcpy(path_cp, path);
			err_ret = miniCreateDirectory(path_cp, "r", &dDes);	//��Ŀ¼
			if(err_ret != ERR_SUCCESS)		//�򿪲�Ŀ¼�ɹ�
				return err_ret;
			else		//��Ŀ¼�ɹ�
			{
				int size = 0;
				int flag_have = 0;
				if(miniReadDirectory(dDes, &size, NULL) == ERR_BUFFER_OVERFLOW)		//��̽Ŀ¼����
				{
					DIRECTORY_ENTRY *entry = (DIRECTORY_ENTRY *)calloc(1, size);
					err_ret = miniReadDirectory(dDes, &size, entry);		//��ȡĿ¼��
					for(int i = 0; i < size; i++)		//���Ŀ¼��ƥ�䣬����
					{
						int mark = 0;
						mark = WildcardMatches(name, entry[i].name);
						if(mark == 1)		//ƥ��ɹ�
						{
							flag_have = 1;
							strcpy(path_cp, path);
							strcat(path_cp,"/");
							strcat(path_cp,entry[i].name);
							err_ret = att_print(path_cp);		//�����Ŀ¼������
							printf("\n");
							if(err_ret != ERR_SUCCESS)
								return err_ret;
						}
						else
							continue;
					}
					free(entry);
				}
				else
					return miniReadDirectory(dDes, &size, NULL);

				if(flag_have == 0)
					printf("No one is matching\n");
				return ERR_SUCCESS;
			}
		}
	}
}

/*
* ��������tp
*���ܣ���ʾָ���ļ������ݡ�
* �������ַ������洢�ļ�����Ϊ����·�������·�����ļ��������ֱ��Ϊ�ļ�����Ĭ�ϵ�ǰĿ¼����
* ����ֵ��ERROR_CODE����ʾִ�н����
*/
ERROR_CODE tp(char *file_name)
{
	ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;

	FILE_DESCRIPTOR * fDes;
	err_flag = miniCreateFile(file_name, 0, "r", &fDes);		//���ļ�
	
	if(err_flag == ERR_SUCCESS)		//���ļ��ɹ�
	{
		char buf[BLOCK_SIZE];
		__int64 read;
		do		//���ļ�
		{
			err_ret = miniReadFile(fDes, BLOCK_SIZE, BLOCK_SIZE, buf, &read);
			if(err_ret == ERR_SUCCESS)		//���ļ��ɹ�
				fwrite(buf, read, 1, stdout);
			else		//���ļ�ʧ��
				break;
		}while(read == BLOCK_SIZE);
		printf("\n");

		err_mid = miniCloseFile(fDes);		//�ر��ļ�
		return err_ret;		//����״̬����
	}
	else
		return err_flag;
}

/*
* ��������more
*���ܣ���ҳ��ʾָ���ļ������ݡ�
* �������ַ������洢�ļ�����Ϊ����·�������·�����ļ��������ֱ��Ϊ�ļ�����Ĭ�ϵ�ǰĿ¼����
* ����ֵ��ERROR_CODE����ʾִ�н����
*/
ERROR_CODE more(char *file_name)
{
	ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;

	FILE_DESCRIPTOR * fDes;
	err_flag = miniCreateFile(file_name, 0, "r", &fDes);		//���ļ�
	
	if(err_flag == ERR_SUCCESS)		//���ļ��ɹ�
	{
		char buf[BLOCK_SIZE];
		__int64 read, write;
		int row, col;
		int num_row = 0,  num_bite = 0;
		int flag = 0;	//�Ƿ��һ�����
		int mark = 1;	//һ�η�ҳ����Ƿ����

		CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 
		SMALL_RECT *srctWindow; 
		hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 

		do		//���ļ�
		{
			err_ret = miniReadFile(fDes, BLOCK_SIZE, BLOCK_SIZE, buf, &read);
			if(err_ret == ERR_SUCCESS)		//���ļ��ɹ�,��ҳ���
			{
				write = 0;
				char ch;
				char *read_in = (char*)buf;
				for(  ;  ;  )		//ѭ����ҳ�������
				{
					/*****************ȷ���������������������*********************/
					if(flag == 0)		//��һ��������ʾ
					{
						if (! GetConsoleScreenBufferInfo(hStdout, &csbiInfo))		//��ȡ����̨�߶�����
						{
							printf("GetConsoleScreenBufferInfo (%d)\n", GetLastError()); 
							return FALSE;
						}
						srctWindow = &csbiInfo.srWindow;
						row = srctWindow->Bottom - srctWindow->Top;		col = srctWindow->Right+1;		//ȷ���������������
						flag = 1;
					}
					else		//�ǵ�һ�����
					{
						if(mark == 1)	//��һ�η�ҳ��ʾ�Ѿ����
						{
							for(char c  ;  ;  )
							{
								c = getch();	//�Ƿ�����
								if(c != ' ' && c != '\r' && c !='q')
									continue;
								else
								{
									if (! GetConsoleScreenBufferInfo(hStdout, &csbiInfo))		//��ȡ����̨�߶�����
									{
										printf("GetConsoleScreenBufferInfo (%d)\n", GetLastError()); 
										return FALSE;
									}
									srctWindow = &csbiInfo.srWindow;
									if(c == ' ')	//��ʾһ��
									{
										row = 1;		col = srctWindow->Right+1;		//ȷ���������������
									}
									else
									{
										if(c == '\r')	//��ʾһ��
										{
											row = srctWindow->Bottom - srctWindow->Top;		col = srctWindow->Right+1;		//ȷ���������������
										}
										else
											return ERR_SUCCESS;
									}
									break;
								}
							}
						}
					}

					/*****************���һ�η�ҳ������*********************/
					for( ; num_row<row; )
					{
						if(write < read)
						{
							ch = read_in[write];
							++write;
						}
						else
							break;
						if(ch == '\r')		//���뻻�з�,Ϊ'\r'��'\n'
						{
							if(write < read)
							{
								ch = read_in[write];
								write++;
								if(ch == '\n')	// ����\n
								{
									putchar(ch);
									++ num_row;		num_bite = 0;
								}
								else		// ����\r
									continue;
							}
							else
								break;

						}	//
						else		//δ��������
						{
							if( num_bite+1 == col)		//�������һ��
							{
								putchar(ch);
								++ num_row;		num_bite = 0;
							}
							else		//���������δ��һ��&&δ��������
							{
								putchar(ch);
								if(ch == '\t')
									num_bite = num_bite + 4;
								else
									++num_bite;
							}
						}
					}
					/*****************���ֱ�����ĸ���*********************/
					if(num_row >= row)		//���һ�η�ҳ���
					{
						num_row = 0;		num_bite = 0;
						mark = 1;
					}
					else		//δ���һ�η�ҳ���
						mark = 0;
					if(write >= read)		//һ�����������Ѿ�ȫ�����
						break;
				}
			}
			else		//���ļ�ʧ��
				break;
		}while(read == BLOCK_SIZE);
		printf("\n");

		//CloseHandle(hStdout);
		err_mid = miniCloseFile(fDes);		//�ر��ļ�

		return err_ret;		//����״̬����
	}
	else
		return err_flag;
}

/*
* ��������cp
*���ܣ������ļ���Ŀ¼��֧��ϵͳ�⿽����ϵͳ�ڣ�ϵͳ�ڿ�����ϵͳ���Լ�ϵͳ���໥��������
* �������ַ������洢�ļ�����Ŀ¼����Ϊ����·�������·�����ļ�����Ŀ¼���������ֱ��Ϊ�ļ�����Ŀ¼����Ĭ�ϵ�ǰĿ¼����
* ����ֵ��ERROR_CODE����ʾִ�н����
*/
ERROR_CODE cp(char *src, char *dst)
{
	if(src[1] == ':' && src[2] == '\\')		//ϵͳ����ϵͳ�ڸ���
	{
		FILE * fp = NULL;
		fp = fopen(src, "rb");		//��ԭ�ļ�
		if(fp == NULL)		//��ԭ�ļ�ʧ��
			return ERR_NOT_FOUND;
		else		//��ԭ�ļ��ɹ�
		{
			ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;
			FILE_DESCRIPTOR * fDes;
			err_flag = miniCreateFile(dst, 0, "r", &fDes);		//��Ŀ���ļ�
			if(err_flag == ERR_SUCCESS)		//��Ŀ���ļ��ɹ������ļ�����ͻ
			{
				err_mid = miniCloseFile(fDes);		//�ر��ļ�
				printf("File exists!\n");
				return ERR_SUCCESS;
			}
			if(err_flag == ERR_TYPE_MISMATCH)		//�򿪵�Ŀ���ļ�ΪĿ¼�����ļ�����ͻ
			{
				printf("\"%s\" is a directory. Cannot create file!\n", dst);
				return ERR_SUCCESS;
			}

			if(err_flag == ERR_NOT_FOUND)		//�򿪵�Ŀ���ļ�δ�ҵ����ɽ�һ�����и���
			{
				if(miniEnoughSpace(fp) == true)		//ϵͳ�ռ��㹻����ʼ����
				{
					err_flag = miniCreateFile(dst, GetFileSize(fp), "w", &fDes);		//д��ʽ��Ŀ���ļ�
					if(err_flag == ERR_SUCCESS)		//��Ŀ���ļ��ɹ����������и���
					{
						hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
						CONSOLE_SCREEN_BUFFER_INFO csb;
						GetConsoleScreenBufferInfo(hStdout, &csb);

						COORD cr = csb.dwCursorPosition;
						cr.X = 0;

						CONSOLE_CURSOR_INFO cc;
						GetConsoleCursorInfo(hStdout, &cc);
						cc.bVisible = 0;
						SetConsoleCursorInfo(hStdout, &cc);

						BAR_INFO bi;
						InitBar(&bi, csb.srWindow.Right, 0.0);
						PrintBar(&bi);

						int how_many_blocks = fDes->self->file_size / BLOCK_SIZE + 1;
						int i_blocks = 0;

						char buf[BLOCK_SIZE];
						__int64 read, write;
						do		//����
						{
							if(i_blocks % BAR_UPDATE == 0)
							{
								SetConsoleCursorPosition(hStdout, cr);
								SetBarPos(&bi, 1.0 * i_blocks / how_many_blocks);
								PrintBar(&bi);
							}
							read = fread(buf, 1, BLOCK_SIZE, fp);
							err_ret = miniWriteFile(fDes, read, BLOCK_SIZE, buf, &write);
							if(err_ret != ERR_SUCCESS)		//д���ݲ��ɹ�
								break;
							i_blocks++;
						}while(read == BLOCK_SIZE);
						SetConsoleCursorPosition(hStdout, cr);
						SetBarPos(&bi, 1.0);
						PrintBar(&bi);
						cc.bVisible = 1;
						SetConsoleCursorInfo(hStdout, &cc);
						fclose(fp);		err_mid = miniCloseFile(fDes);
						return err_ret;
					}
					else 
						return err_flag;
				}
				else
					return ERR_NOT_ENOUGH_DISK_SPACE;
			}
			else		//�������󣬷���״̬����
				return err_flag;
		}
	}
	else
	{
		if(dst[1] == ':' && dst[2] == '\\')		//ϵͳ����ϵͳ�⸴��
		{
			ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;
			FILE_DESCRIPTOR * fDes;
			err_flag = miniCreateFile(src, 0, "r", &fDes);		//��ԭ�ļ�
			if(err_flag != ERR_SUCCESS)		//��ԭ�ļ����ɹ�
				return err_flag;
			else		//��ԭ�ļ��ɹ�
			{
				FILE * fp = NULL;
				fp = fopen(dst, "r");		//��Ŀ���ļ�
				if(fp != NULL)		//��ԭ�ļ��ɹ������ļ�����ͻ
				{
					printf("File exists!\n");
					return ERR_SUCCESS;
				}
				else		//�ɽ�һ�����и���
				{
					//�˴�����ϵͳ��ռ��㹻���ʲ��ж�ϵͳ��ռ��Ƿ��㹻
					fp = fopen(dst, "wb");		//д��ʽ��Ŀ���ļ�
					if(fp != NULL)		//��Ŀ���ļ��ɹ����������и���
					{
						hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
						CONSOLE_SCREEN_BUFFER_INFO csb;
						GetConsoleScreenBufferInfo(hStdout, &csb);

						COORD cr = csb.dwCursorPosition;
						cr.X = 0;

						CONSOLE_CURSOR_INFO cc;
						GetConsoleCursorInfo(hStdout, &cc);
						cc.bVisible = 0;
						SetConsoleCursorInfo(hStdout, &cc);

						BAR_INFO bi;
						InitBar(&bi, csb.srWindow.Right, 0.0);
						PrintBar(&bi);

						int how_many_blocks = fDes->self->file_size / BLOCK_SIZE + 1;
						int i_blocks = 0;

						char buf[BLOCK_SIZE];
						__int64 read;
						do		//���и���
						{
							if(i_blocks % BAR_UPDATE == 0)
							{
								SetConsoleCursorPosition(hStdout, cr);
								SetBarPos(&bi, 1.0 * i_blocks / how_many_blocks);
								PrintBar(&bi);
							}
							err_ret = miniReadFile(fDes, BLOCK_SIZE, BLOCK_SIZE, buf, &read);
							if(err_ret == ERR_SUCCESS)
								fwrite(buf, 1, read, fp);
							else		//���ļ�ʧ��
								break;
							i_blocks++;
						}while(read == BLOCK_SIZE);
						SetConsoleCursorPosition(hStdout, cr);
						SetBarPos(&bi, 1.0);
						PrintBar(&bi);
						cc.bVisible = 1;
						SetConsoleCursorInfo(hStdout, &cc);
						fclose(fp);		err_mid = miniCloseFile(fDes);
						return err_ret;
					}
					else
						return ERR_OTHER;
				}
			}
		}
		else		//ϵͳ����ϵͳ�ڸ���
		{
			ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;
			FILE_DESCRIPTOR * fDes_src;
			err_flag = miniCreateFile(src, 0, "r", &fDes_src);		//��ԭ�ļ�
			if(err_flag != ERR_SUCCESS)		//��ԭ�ļ����ɹ�
				return err_flag;
			else		//��ԭ�ļ��ɹ�
			{
				FILE_DESCRIPTOR * fDes_dst;
				err_flag = miniCreateFile(dst, 0, "r", &fDes_dst);		//��Ŀ���ļ�
				if(err_flag == ERR_SUCCESS)		//��Ŀ���ļ��ɹ������ļ�����ͻ
				{
					err_mid = miniCloseFile(fDes_dst);		//�ر��ļ�
					printf("File exists!\n");
					return ERR_SUCCESS;
				}
				if(err_flag == ERR_TYPE_MISMATCH)		//�򿪵�Ŀ���ļ�ΪĿ¼�����ļ�����ͻ
				{
					printf("\"%s\" is a directory. Cannot create file!\n", dst);
					return ERR_SUCCESS;
				}

				if(err_flag == ERR_NOT_FOUND)		//�򿪵�Ŀ���ļ�δ�ҵ����ɽ�һ�����и���
				{
					if(miniEnoughSpace(fDes_src) == true)		//ϵͳ�ռ��㹻����ʼ����
					{
						err_flag = miniCreateFile(dst, fDes_src->self->file_size, "w", &fDes_dst);		//д��ʽ��Ŀ���ļ�
						if(err_flag == ERR_SUCCESS)		//��Ŀ���ļ��ɹ����������и���
						{
							hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
							CONSOLE_SCREEN_BUFFER_INFO csb;
							GetConsoleScreenBufferInfo(hStdout, &csb);

							COORD cr = csb.dwCursorPosition;
							cr.X = 0;

							CONSOLE_CURSOR_INFO cc;
							GetConsoleCursorInfo(hStdout, &cc);
							cc.bVisible = 0;
							SetConsoleCursorInfo(hStdout, &cc);

							BAR_INFO bi;
							InitBar(&bi, csb.srWindow.Right, 0.0);
							PrintBar(&bi);

							int how_many_blocks = fDes_dst->self->file_size / BLOCK_SIZE + 1;
							int i_blocks = 0;

							char buf[BLOCK_SIZE];
							__int64 read, write;
							do		//����
							{
								if(i_blocks % BAR_UPDATE == 0)
								{
									SetConsoleCursorPosition(hStdout, cr);
									SetBarPos(&bi, 1.0 * i_blocks / how_many_blocks);
									PrintBar(&bi);
								}
								err_ret = miniReadFile(fDes_src, BLOCK_SIZE, BLOCK_SIZE, buf, &read);
								if(err_ret == ERR_SUCCESS)
								{
									err_ret = miniWriteFile(fDes_dst, read, BLOCK_SIZE, buf, &write);
									if(err_ret != ERR_SUCCESS)		//д���ݲ��ɹ�
										break;
								}
								else		//�����ݲ��ɹ�
									break;
								i_blocks++;
							}while(read == BLOCK_SIZE);
							SetConsoleCursorPosition(hStdout, cr);
							SetBarPos(&bi, 1.0);
							PrintBar(&bi);
							cc.bVisible = 1;
							SetConsoleCursorInfo(hStdout, &cc);
							err_mid = miniCloseFile(fDes_src);		err_mid = miniCloseFile(fDes_dst);
							return err_ret;
						}
						else 
							return err_flag;
					}
					else
						return ERR_NOT_ENOUGH_DISK_SPACE;
				}
				else		//�������󣬷���״̬����
					return err_flag;
			}
		}
	}
}

/*
* ��������dl
*���ܣ�ɾ��ָ���ļ���
* �������ַ������洢�ļ�����Ϊ����·�������·�����ļ��������ֱ��Ϊ�ļ�����Ĭ�ϵ�ǰĿ¼����
* ����ֵ��ERROR_CODE����ʾִ�н����
*/
ERROR_CODE dl(char *file_name)
{
	ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;

	FILE_DESCRIPTOR * fDes;
	err_flag = miniCreateFile(file_name, 0, "r", &fDes);		//���ļ�
	if(err_flag == ERR_SUCCESS)		//���ļ��ɹ�
	{
		err_ret = miniDeleteFile(fDes);		//ɾ���ļ�
		err_mid = miniCloseFile(fDes);		//�ر��ļ�
		return err_ret;		//����״̬����
	}
	else		//���ļ�ʧ��
	{
		if(err_flag = ERR_TYPE_MISMATCH)		//·����ָΪһ��Ŀ¼
		{
			DIRECTORY_DESCRIPTOR * dDes;
			err_flag = miniCreateDirectory(file_name, "r", &dDes);	//��Ŀ¼
			if(err_flag == ERR_SUCCESS)		//��Ŀ¼�ɹ�
			{
				err_ret = miniDeleteDirectory(dDes);	//ɾ��Ŀ¼
				err_mid = miniCloseDirectory(dDes);		//�ر�Ŀ¼
				return err_ret;		//����״̬����
			}
			else
				return err_flag;	//��Ŀ¼ʧ��
		}
		else
			return err_flag;	//��������
	}	
}

/*
* ��������GetFileSize
*���ܣ�����ļ���ϵͳ�⣩��С��
* �������ļ���������
* ����ֵ��__int64����ʾ�ļ���С����λ���ֽڡ�
*/
__int64 GetFileSize(FILE *fp)
{
	__int64 size = 0;
	_fseeki64(fp, 0, SEEK_END);
	size = _ftelli64(fp);
	fseek(fp, 0, SEEK_SET);

	// ��ֹ���ļ�������ΪĿ¼
	if(size == 0)
		return 1;
	return size;
}

/*
* ��������WildcardMatches
*���ܣ�ͨ���ƥ�䡣
* �����������ַ�������һ��������ͨ������ڶ���Ϊ��ƥ���ַ�����
* ����ֵ��ƥ������
*/
int WildcardMatches(char *wildcard, char *str) 
{  
    for (  ; *wildcard;  ++wildcard, ++str)	//ѭ������
	{
		if (*wildcard == '*')    //��� wildcard �ĵ�ǰ�ַ��� '*'ʱ�������
		{
			while(*wildcard=='*') 
			{
				wildcard++;
			}
	    for (; *str; ++str)
			{ 
		if (WildcardMatches(wildcard, str))  
		    return 1;  
			}
	    return *wildcard == '\0';  
	}
		else	 //��� wildcard �ĵ�ǰ�ַ��� '*'ʱ�������
		{
			if ((*wildcard != '?') && (*wildcard != *str))    
		return 0;  
		}
	}

    return *str == '\0';	//����ƥ����
}

/*
* ��������att_print
*���ܣ�����ļ�/Ŀ¼���ԡ�
* ������һ���ַ�����ΪҪ������ļ�/Ŀ¼�ľ��Ի����·����
* ����ֵ������ֵ��ERROR_CODE����ʾִ�н����
*/
ERROR_CODE att_print(char *file_name)
{
	ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;

	FILE_DESCRIPTOR * fDes;
	err_flag = miniCreateFile(file_name, 0, "r", &fDes);		//���ļ�
	
	if(err_flag == ERR_SUCCESS)		//���ļ��ɹ�
	{
		printf("Name:			%s\n",fDes->self->name);		//�������
		printf("Size:			%lld Bytes\n",fDes->self->file_size);
		printf("File or Dir:		%s\n",(fDes->self->is_file ? "File" : "Dir"));
		struct tm *t_c = _localtime64(&fDes->self->create_time);
		printf("Create time:		%d/%d/%d  %02d:%02d:%02d\n",t_c->tm_year+1900, t_c->tm_mon+1, t_c->tm_mday,  t_c->tm_hour, t_c->tm_min, t_c->tm_sec);
		struct tm *t_m = _localtime64(&fDes->self->modified_time);
		printf("Modified time:		%d/%d/%d  %02d:%02d:%02d\n",t_m->tm_year+1900, t_m->tm_mon+1, t_m->tm_mday,  t_m->tm_hour, t_m->tm_min, t_m->tm_sec);
		struct tm *t_a = _localtime64(&fDes->self->access_time);
		printf("Access time:		%d/%d/%d  %02d:%02d:%02d\n",t_a->tm_year+1900, t_a->tm_mon+1, t_a->tm_mday,  t_a->tm_hour, t_a->tm_min, t_a->tm_sec);		
		
		err_mid = miniCloseFile(fDes);
		return ERR_SUCCESS;
	}

	if(err_flag == ERR_TYPE_MISMATCH)		//�򿪵�ΪĿ¼
	{
		DIRECTORY_DESCRIPTOR * dDes;
		err_flag = miniCreateDirectory(file_name, "r", &dDes);	//��Ŀ¼
		if(err_flag == ERR_SUCCESS)		//��Ŀ¼�ɹ�
		{
			printf("Name:			%s\n",dDes->self->name);		//�������
			printf("Size:			%lld Bytes\n",dDes->self->file_size);
			printf("File or Dir:		%s\n",(dDes->self->is_file ? "File" : "Dir"));
			struct tm *t_c = _localtime64(&dDes->self->create_time);
			printf("Create time:		%d/%d/%d  %02d:%02d:%02d\n",t_c->tm_year+1900, t_c->tm_mon+1, t_c->tm_mday,  t_c->tm_hour, t_c->tm_min, t_c->tm_sec);
			struct tm *t_m = _localtime64(&dDes->self->modified_time);
			printf("Modified time:		%d/%d/%d  %02d:%02d:%02d\n",t_m->tm_year+1900, t_m->tm_mon+1, t_m->tm_mday,  t_m->tm_hour, t_m->tm_min, t_m->tm_sec);
			struct tm *t_a = _localtime64(&dDes->self->access_time);		
			printf("Access time:		%d/%d/%d  %02d:%02d:%02d\n",t_a->tm_year+1900, t_a->tm_mon+1, t_a->tm_mday,  t_a->tm_hour, t_a->tm_min, t_a->tm_sec);

			err_mid = miniCloseDirectory(dDes);
			return ERR_SUCCESS;
		}
		else
			return err_flag;
	}

	return err_flag;
}

// width: ��[��%���ܳ���
void InitBar(BAR_INFO *bi, int width, float init_ratio)
{
	memset(bi->bar, 0, sizeof(bi->bar));
	memset(bi->bar, ' ', width);
	bi->progress_chr = '=';
	bi->arrow_chr = '>';
	// 9������'[', '>', ']', " XX.X%"
	bi->width = width - 9;
	bi->ratio = init_ratio;
	bi->str_ratio_offset = bi->width + 4;
	bi->last_pos = 0;

	bi->bar[0] = '[';
	bi->bar[bi->width + 2] = ']';
	SetBarPos(bi, init_ratio);
}

void SetBarPos(BAR_INFO *bi, float new_ratio)
{
	int new_pos = (int)(bi->width * new_ratio);
	for(int i = bi->last_pos; i <= new_pos; i++)
	{
		bi->bar[1 + i] = bi->progress_chr;
	}
	bi->bar[new_pos + 1] = bi->arrow_chr;
	bi->ratio = new_ratio;

	sprintf(&bi->bar[bi->str_ratio_offset], "%3.1f%%", bi->ratio * 100);
}

void PrintBar(BAR_INFO *bi)
{
	fwrite(bi->bar, strlen(bi->bar), 1, stdout);
}