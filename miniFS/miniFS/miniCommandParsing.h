#ifndef PARSING_COMMAND_H
#define PARSING_COMMAND_H

typedef int TYPE;
typedef int EXEC;

#define	TYPE_CLOSE			0x00		//�ر�ϵͳ����
#define	TYPE_HEIP			0x01		//��������
#define TYPE_SYS			0x02		//��ʾϵͳ��Ϣ
#define	TYPE_FMT			0x03		//��ʽ������
#define	TYPE_OPT			0x04		//�Ż��ռ�����
#define TYPE_CLS			0x05		//��������
#define TYPE_MKDIR			0x06		//����Ŀ¼����
#define	TYPE_CD				0x07		//�л�Ŀ¼����
#define	TYPE_DR				0x08		//��ʾĿ¼����
#define TYPE_ATT			0x09		//��ʾ�ļ���������
#define TYPE_TP				0x0a		//��ʾ�ļ�����
#define TYPE_MORE			0x0b		//��ҳ��ʾ�ļ�����
#define TYPE_CP				0x0c		//��������
#define TYPE_DL				0x0d		//ɾ������
#define TYPE_NOT_FOUND		0x0e		//�������
#define TYPE_INVALID_PARAMETER	0x0f	//�����������
#define TYPE_BUFFER_OVERFLOW	0x10	//���������
#define TYPE_NO				0x11		//δ��������


#define EXEC_Y		0x00				//����ִ��
#define EXEC_N		0x01				//���ִ��

typedef struct Command		//�洢����������
{
	TYPE type;		//��������
	int flag;		//�������
	char para1[260];	//����һ
	char para2[260];	//������
}Cmd;

Cmd ParsingCommand( );		//���������
EXEC Confirm();		//����ִ����ѯ��
void ExecuteCommand(Cmd cmd);		//ִ�������

#endif