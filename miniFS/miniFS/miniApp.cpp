#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstdio>

#include <map>
#include <string>
#include <iostream>


#include "util.h"
#include "miniApp.h"
#include "miniFile.h"
#include "miniBlock.h"
#include "miniError.h"


extern FILE *g_storage;                        // OS上的物理文件（miniBlock.cpp）
extern FILE_SYSTEM_HEADER *g_file_system_header;        // 文件系统头缓存
extern Bitmap *g_bitmap;                    // 位图缓存
extern char g_current_path[MAX_PATH_NAME];            // 全局当前目录字符串(from miniFile.cpp)

char file_buf[BLOCK_SIZE];                    // 文件缓冲区
std::map<std::string, std::string> maplist;            // 帮助信息map容器

#define OUT_FILE_PREFIX "file://"
#define OUT_FILE_PREFIX_LEN 7

/*
* 函数名：mount
*功能：启动系统。命令解析层调用该接口启动系统。
* 参数：磁盘文件名，即系统名。
* 返回值：ERROR_CODE，表示执行结果。
*/
ERROR_CODE mount(char *file_name) {
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
		} else if (status != ERR_SUCCESS) {
            DbgPrint(status);
            return status;
        }
    }

    using std::map;
    using std::string;
    using std::pair;
    // 加载字符串资源
    maplist.insert(pair<string, string>("create", "create wFName [size]\t建立mini-uFS文件。size单位：GB。"));
    maplist.insert(pair<string, string>("mount", "mount wFName\t\t进入 mini-uFS 系统。"));
    maplist.insert(pair<string, string>("fmt", "fmt\t\t\t格式化系统。"));
    maplist.insert(pair<string, string>("dr", "dr [drName]\t\t显示当前或指定目录。"));
    maplist.insert(pair<string, string>("cp", "cp FName1 FName2\t将文件FName1复制至FName2。"));
    maplist.insert(pair<string, string>("dl", "dl uFName\t\t删除指定文件。"));
    maplist.insert(pair<string, string>("tp", "tp uFName\t\t显示指定文件内容。"));
    maplist.insert(pair<string, string>("more", "more uFName\t\t分页显示指定文件内容。"));
    maplist.insert(pair<string, string>("att", "att uFName\t\t显示文件属性。"));
    maplist.insert(pair<string, string>("close", "close\t\t\t退出系统。"));
    maplist.insert(pair<string, string>("opt", "opt\t\t\t优化磁盘使用空间。"));
    maplist.insert(pair<string, string>("cd", "cd drName\t\t切换到指定目录。"));
    maplist.insert(pair<string, string>("mkdir", "mkdir dir\t\t新建目录"));
    maplist.insert(pair<string, string>("help", "help [cmd]\t\t显示命令的帮助信息。"));
    maplist.insert(pair<string, string>("cls", "cls\t\t\t清屏"));
    return ERR_SUCCESS;
}

/*
* 函数名：close
*功能：退出系统。命令解析层调用该接口退出系统。
* 参数：无。
* 返回值：ERROR_CODE，表示执行结果。
*/
ERROR_CODE close(void) {
    ERROR_CODE err = ERR_OTHER;
    err = miniExitSystem();        //调用文件层miniExitSystem接口
    return err;
}

/*
* 函数名：cls
*功能：清屏。
* 参数：无。
* 返回值：ERROR_CODE，表示执行结果。
*/
ERROR_CODE cls(void)
{
	system("cls");
	return ERR_SUCCESS;
}

/*
* 函数名：sys
*功能：显示系统信息。
* 参数：无。
* 返回值：ERROR_CODE，表示执行结果。
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
* 函数名：help
*功能：显示帮助。命令解析层调用该接口显示命令帮助。
* 参数：字符串。存储要显示帮助的命令名，字符串为空时显示所有命令帮助。
* 返回值：无。
*/
void help(char *cmd) {
    using std::map;
    using std::string;
    using std::pair;
    using std::cout;
    using std::endl;

    map<string, string>::iterator iter;
    if (cmd == NULL)            //参数为空：所有命令的说明
    {
        for (iter = maplist.begin(); iter != maplist.end(); iter++) {
            cout << iter->second << endl;
        }
    } else                    //有参数：具体命令的说明
    {
        iter = maplist.find(cmd);
        if (iter != maplist.end()) {
            cout << maplist[cmd] << endl;
        } else        //参数所指命令不存在
        {
            cout << "Command type mismatch！You can type 'help command' for help." << endl;
        }
	}
}

/*
* 函数名：fmt
*功能：格式化系统。命令解析层调用该接口格式化系统。
* 参数：无。
* 返回值：ERROR_CODE，表示执行结果。
*/
ERROR_CODE fmt(void) {
    ERROR_CODE err = ERR_OTHER;
    err = miniFormat();        //调用文件层miniFormat接口
    if (err == ERR_SUCCESS)
        miniChangeCurrentDirectory("/");
    return err;
}

/*
* 函数名：opt
*功能：优化磁盘存储空间。命令解析层调用该接口优化存储空间。
* 参数：无。
* 返回值：ERROR_CODE，表示执行结果
*/
ERROR_CODE opt(void) {
    ERROR_CODE err = ERR_OTHER;
    //err = miniOptimize();		//调用文件层miniOptimize接口
    return err;
}

/*
* 函数名：mkdir
*功能：建立目录。命令解析层调用该接口建立目录。
* 参数：字符串。存储路径（绝对路径或相对路径）。
* 返回值：ERROR_CODE，表示执行结果。
*/
ERROR_CODE mkdir(char *path) {
    ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;

    DIRECTORY_DESCRIPTOR *dDes;
    err_flag = miniCreateDirectory(path, "r", &dDes);    //打开目录
    if (err_flag == ERR_SUCCESS)        //打开目录成功，则目录名冲突
    {
        err_mid = miniCloseDirectory(dDes);        //关闭文件
        printf("Directory exists!\n");
        return ERR_SUCCESS;
    }
    if (err_flag == ERR_TYPE_MISMATCH)        //打开的目标目录为文件，则目录名冲突
    {
        printf("\"%s\" is a file. Cannot create directory!\n", path);
        return ERR_SUCCESS;
    }

    if (err_flag == ERR_NOT_FOUND)        //打开的目标目录未找到，可创建该目录
    {
        err_ret = miniCreateDirectory(path, "w", &dDes);        //创建目录
        return err_ret;
    } else
        return err_flag;
}

/*
* 函数名：cd
*功能：切换目录。命令解析层调用该接口切换目录。
* 参数：字符串。存储路径（绝对路径或相对路径）。
* 返回值：ERROR_CODE，表示执行结果。
*/
ERROR_CODE cd(char *path) {
    ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;

    DIRECTORY_DESCRIPTOR *dDes;
    err_flag = miniCreateDirectory(path, "r", &dDes);    //打开目录
    if (err_flag == ERR_SUCCESS)        //打开目录成功
    {
        err_ret = miniChangeCurrentDirectory(path);    //切换目录
        err_mid = miniCloseDirectory(dDes);        //关闭目录
        return err_ret;        //返回状态代码
    } else
        return err_flag;
}

/*
* 函数名：cd
*功能：显示目录。命令解析层调用该接口显示目录内容。
* 参数：字符串。存储路径（绝对路径或相对路径），当字符串为空时显示当前目录内容。
* 返回值：ERROR_CODE，表示执行结果。
*/
ERROR_CODE dr(char *path) {
    ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;

    DIRECTORY_DESCRIPTOR *dDes;
    err_flag = miniCreateDirectory(path, "r", &dDes);    //打开目录
    if (err_flag == ERR_SUCCESS)        //打开目录成功
    {
        int size = 0;
        if (miniReadDirectory(dDes, &size, NULL) == ERR_BUFFER_OVERFLOW)        //试探目录项数
        {
            DIRECTORY_ENTRY *entry = (DIRECTORY_ENTRY *) calloc(1, size);
            err_ret = miniReadDirectory(dDes, &size, entry);        //读取目录项
            for (int i = 0; i < size; i++)        //输出目录内容
            {
                printf("%s\t\t%s\n", entry[i].name, (entry[i].is_file ? "FILE" : "DIR"));
            }
            free(entry);
        }

        err_mid = miniCloseDirectory(dDes);        //关闭目录
        if (size == 0)        //目录为空
            printf("Directory is empty!\n");
        return err_ret;        //返回状态代码
    }
	else
		return err_flag;
}

/*
* 函数名：att
*功能：显示目录或文件属性。
* 参数：字符串。存储目录或文件名，为绝对路径或相对路径加文件名，异或直接为文件名（默认当前目录）。
* 返回值：ERROR_CODE，表示执行结果。
*/
ERROR_CODE att(char *file_name) {
    ERROR_CODE err_ret = ERR_OTHER;

    /*****************是否含有通配符判断*********************/
    int len = strlen(file_name);
    int pos = 0;
    for (int i = 0; i < len; i++) {
        if (file_name[i] == '/')
            pos = i;
    }
    int flag = 0;
    for (int i = pos; i < len; i++) {
        if (file_name[i] == '*' || file_name[i] == '?')
            flag = 1;
    }

    /*****************显示目录或文件属性*********************/
    if (flag == 0)        //不含通配符
    {
        err_ret = att_print(file_name);
        return err_ret;
    } else        //含有通配符
    {
        if (pos == 0)        //相对路径含通配符
        {
            DIRECTORY_DESCRIPTOR *dDes;
            err_ret = miniCreateDirectory(g_current_path, "r", &dDes);    //打开目录
            if (err_ret != ERR_SUCCESS)        //打开不目录成功
                return err_ret;
            else        //打开目录成功
            {
                int size = 0;
                int flag_have = 0;
                if (miniReadDirectory(dDes, &size, NULL) == ERR_BUFFER_OVERFLOW)        //试探目录项数
                {
                    DIRECTORY_ENTRY *entry = (DIRECTORY_ENTRY *) calloc(1, size);
                    err_ret = miniReadDirectory(dDes, &size, entry);        //读取目录项
                    for (int i = 0; i < size; i++)        //逐个目录项匹配，处理
                    {
                        int mark = 0;
                        mark = WildcardMatches(file_name, entry[i].name);
                        if (mark == 1)        //匹配成功
                        {
                            flag_have = 1;
                            err_ret = att_print(entry[i].name);        //输出该目录项内容
                            printf("\n");
                            if (err_ret != ERR_SUCCESS)
                                return err_ret;
                        } else
                            continue;
                    }
                    free(entry);
                } else
                    return miniReadDirectory(dDes, &size, NULL);

                if (flag_have == 0)        //无任何一个项匹配
                    printf("No one is matching\n");
                return ERR_SUCCESS;
            }
        } else        //绝对路径含通配符
        {
            char path[550];
            char name[260];
            for (int i = 0; i < pos; i++)        //提取目录
                path[i] = file_name[i];
            for (int i = pos + 1; i < len; i++)    //提取含通配符的文件名
                name[i - pos - 1] = file_name[i];
            path[pos] = '\0';
            name[len - pos - 1] = '\0';

            DIRECTORY_DESCRIPTOR *dDes;
            char path_cp[520];
            strcpy(path_cp, path);
            err_ret = miniCreateDirectory(path_cp, "r", &dDes);    //打开目录
            if (err_ret != ERR_SUCCESS)        //打开不目录成功
                return err_ret;
            else        //打开目录成功
            {
                int size = 0;
                int flag_have = 0;
                if (miniReadDirectory(dDes, &size, NULL) == ERR_BUFFER_OVERFLOW)        //试探目录项数
                {
                    DIRECTORY_ENTRY *entry = (DIRECTORY_ENTRY *) calloc(1, size);
                    err_ret = miniReadDirectory(dDes, &size, entry);        //读取目录项
                    for (int i = 0; i < size; i++)        //逐个目录项匹配，处理
                    {
                        int mark = 0;
                        mark = WildcardMatches(name, entry[i].name);
                        if (mark == 1)        //匹配成功
                        {
                            flag_have = 1;
                            strcpy(path_cp, path);
                            strcat(path_cp, "/");
                            strcat(path_cp, entry[i].name);
                            err_ret = att_print(path_cp);        //输出该目录项内容
                            printf("\n");
                            if (err_ret != ERR_SUCCESS)
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
* 函数名：tp
*功能：显示指定文件的内容。
* 参数：字符串。存储文件名，为绝对路径或相对路径加文件名，异或直接为文件名（默认当前目录）。
* 返回值：ERROR_CODE，表示执行结果。
*/
ERROR_CODE tp(char *file_name) {
    ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;

    FILE_DESCRIPTOR *fDes;
    err_flag = miniCreateFile(file_name, 0, "r", &fDes);        //打开文件

    if (err_flag == ERR_SUCCESS)        //打开文件成功
    {
        char buf[BLOCK_SIZE];
        long long read;
        do        //读文件
        {
            err_ret = miniReadFile(fDes, BLOCK_SIZE, BLOCK_SIZE, buf, &read);
            if (err_ret == ERR_SUCCESS)        //读文件成功
                fwrite(buf, read, 1, stdout);
            else        //读文件失败
                break;
        } while (read == BLOCK_SIZE);
        printf("\n");

        err_mid = miniCloseFile(fDes);        //关闭文件
        return err_ret;        //返回状态代码
    }
	else
		return err_flag;
}

/*
* 函数名：more
*功能：分页显示指定文件的内容。
* 参数：字符串。存储文件名，为绝对路径或相对路径加文件名，异或直接为文件名（默认当前目录）。
* 返回值：ERROR_CODE，表示执行结果。
*/
ERROR_CODE more(char *file_name) {
    ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;

    FILE_DESCRIPTOR *fDes;
    err_flag = miniCreateFile(file_name, 0, "r", &fDes);        //打开文件

    if (err_flag == ERR_SUCCESS)        //打开文件成功
    {
        char buf[BLOCK_SIZE];
        long long read;
        do        //读文件
        {
            err_ret = miniReadFile(fDes, BLOCK_SIZE, BLOCK_SIZE, buf, &read);
            if (err_ret == ERR_SUCCESS)        //读文件成功,分页输出
            {
                fwrite(buf, sizeof(char), read, stdout);
            } else        //读文件失败
                break;
        } while (read == BLOCK_SIZE);
        printf("\n");

        //CloseHandle(hStdout);
        err_mid = miniCloseFile(fDes);        //关闭文件

        return err_ret;        //返回状态代码
    }
	else
		return err_flag;
}

/*
* 函数名：cp
*功能：拷贝文件或目录（支持系统外拷贝至系统内，系统内拷贝到系统外以及系统内相互拷贝）。
* 参数：字符串。存储文件名或目录名，为绝对路径或相对路径加文件名（目录名），异或直接为文件名或目录名（默认当前目录）。
* 返回值：ERROR_CODE，表示执行结果。
*/
ERROR_CODE cp(char *src, char *dst) {
    if (startsWith(OUT_FILE_PREFIX, src))        //系统外向系统内复制
    {
        src = src + OUT_FILE_PREFIX_LEN;
        FILE *fp = NULL;
        fp = fopen(src, "rb");        //打开原文件
        if (fp == NULL)        //打开原文件失败
            return ERR_NOT_FOUND;
        else        //打开原文件成功
        {
            ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;
            FILE_DESCRIPTOR *fDes;
            err_flag = miniCreateFile(dst, 0, "r", &fDes);        //打开目标文件
            if (err_flag == ERR_SUCCESS)        //打开目标文件成功，则文件名冲突
            {
                err_mid = miniCloseFile(fDes);        //关闭文件
                printf("File exists!\n");
                return ERR_SUCCESS;
            }
            if (err_flag == ERR_TYPE_MISMATCH)        //打开的目标文件为目录，则文件名冲突
            {
                printf("\"%s\" is a directory. Cannot create file!\n", dst);
                return ERR_SUCCESS;
            }

            if (err_flag == ERR_NOT_FOUND)        //打开的目标文件未找到，可进一步进行复制
            {
                if (miniEnoughSpace(fp) == true)        //系统空间足够，开始复制
                {
                    err_flag = miniCreateFile(dst, GetFileSize(fp), "w", &fDes);        //写方式打开目标文件
                    if (err_flag == ERR_SUCCESS)        //打开目标文件成功，继续进行复制
                    {
                        int how_many_blocks = fDes->self->file_size / BLOCK_SIZE + 1;
                        int i_blocks = 0;

                        char buf[BLOCK_SIZE];
                        long long read, write;
                        do        //复制
                        {
                            read = fread(buf, 1, BLOCK_SIZE, fp);
                            err_ret = miniWriteFile(fDes, read, BLOCK_SIZE, buf, &write);
                            if (err_ret != ERR_SUCCESS)        //写数据不成功
                                break;
                            i_blocks++;
                        } while (read == BLOCK_SIZE);
                        fclose(fp);
                        err_mid = miniCloseFile(fDes);
                        return err_ret;
                    } else
                        return err_flag;
                } else
                    return ERR_NOT_ENOUGH_DISK_SPACE;
            } else        //其他错误，返回状态代码
                return err_flag;
		}
	}
	else {
        if (startsWith(OUT_FILE_PREFIX, dst))        //系统内向系统外复制
        {
            dst = dst + OUT_FILE_PREFIX_LEN;
            ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;
            FILE_DESCRIPTOR *fDes;
            err_flag = miniCreateFile(src, 0, "r", &fDes);        //打开原文件
            if (err_flag != ERR_SUCCESS)        //打开原文件不成功
                return err_flag;
            else        //打开原文件成功
            {
                FILE *fp = NULL;
                fp = fopen(dst, "r");        //打开目标文件
                if (fp != NULL)        //打开原文件成功，即文件名冲突
                {
                    printf("File exists!\n");
                    return ERR_SUCCESS;
                } else        //可进一步进行复制
                {
                    //此处假设系统外空间足够，故不判断系统外空间是否足够
                    fp = fopen(dst, "wb");        //写方式打开目标文件
                    if (fp != NULL)        //打开目标文件成功，继续进行复制
                    {
                        int how_many_blocks = fDes->self->file_size / BLOCK_SIZE + 1;
                        int i_blocks = 0;

                        char buf[BLOCK_SIZE];
                        long long read;
                        do        //进行复制
                        {
                            err_ret = miniReadFile(fDes, BLOCK_SIZE, BLOCK_SIZE, buf, &read);
                            if (err_ret == ERR_SUCCESS)
                                fwrite(buf, 1, read, fp);
                            else        //读文件失败
                                break;
                            i_blocks++;
                        } while (read == BLOCK_SIZE);
                        fclose(fp);
                        err_mid = miniCloseFile(fDes);
                        return err_ret;
                    } else
                        return ERR_OTHER;
                }
            }
        } else        //系统内向系统内复制
        {
            ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;
            FILE_DESCRIPTOR *fDes_src;
            err_flag = miniCreateFile(src, 0, "r", &fDes_src);        //打开原文件
            if (err_flag != ERR_SUCCESS)        //打开原文件不成功
                return err_flag;
            else        //打开原文件成功
            {
                FILE_DESCRIPTOR *fDes_dst;
                err_flag = miniCreateFile(dst, 0, "r", &fDes_dst);        //打开目标文件
                if (err_flag == ERR_SUCCESS)        //打开目标文件成功，则文件名冲突
                {
                    err_mid = miniCloseFile(fDes_dst);        //关闭文件
                    printf("File exists!\n");
                    return ERR_SUCCESS;
                }
                if (err_flag == ERR_TYPE_MISMATCH)        //打开的目标文件为目录，则文件名冲突
                {
                    printf("\"%s\" is a directory. Cannot create file!\n", dst);
                    return ERR_SUCCESS;
                }

                if (err_flag == ERR_NOT_FOUND)        //打开的目标文件未找到，可进一步进行复制
                {
                    if (miniEnoughSpace(fDes_src) == true)        //系统空间足够，开始复制
                    {
                        err_flag = miniCreateFile(dst, fDes_src->self->file_size, "w", &fDes_dst);        //写方式打开目标文件
                        if (err_flag == ERR_SUCCESS)        //打开目标文件成功，继续进行复制
                        {
                            int how_many_blocks = fDes_dst->self->file_size / BLOCK_SIZE + 1;
                            int i_blocks = 0;

                            char buf[BLOCK_SIZE];
                            long long read, write;
                            do        //复制
                            {
                                err_ret = miniReadFile(fDes_src, BLOCK_SIZE, BLOCK_SIZE, buf, &read);
                                if (err_ret == ERR_SUCCESS) {
                                    err_ret = miniWriteFile(fDes_dst, read, BLOCK_SIZE, buf, &write);
                                    if (err_ret != ERR_SUCCESS)        //写数据不成功
                                        break;
                                } else        //读数据不成功
                                    break;
                                i_blocks++;
                            } while (read == BLOCK_SIZE);
                            err_mid = miniCloseFile(fDes_src);
                            err_mid = miniCloseFile(fDes_dst);
                            return err_ret;
                        } else
                            return err_flag;
                    } else
                        return ERR_NOT_ENOUGH_DISK_SPACE;
                } else        //其他错误，返回状态代码
                    return err_flag;
			}
		}
	}
}

/*
* 函数名：dl
*功能：删除指定文件。
* 参数：字符串。存储文件名，为绝对路径或相对路径加文件名，异或直接为文件名（默认当前目录）。
* 返回值：ERROR_CODE，表示执行结果。
*/
ERROR_CODE dl(char *file_name) {
    ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;

    FILE_DESCRIPTOR *fDes;
    err_flag = miniCreateFile(file_name, 0, "r", &fDes);        //打开文件
    if (err_flag == ERR_SUCCESS)        //打开文件成功
    {
        err_ret = miniDeleteFile(fDes);        //删除文件
        err_mid = miniCloseFile(fDes);        //关闭文件
        return err_ret;        //返回状态代码
    } else        //打开文件失败
    {
        if (err_flag == ERR_TYPE_MISMATCH)        //路径所指为一个目录
        {
            DIRECTORY_DESCRIPTOR *dDes;
            err_flag = miniCreateDirectory(file_name, "r", &dDes);    //打开目录
            if (err_flag == ERR_SUCCESS)        //打开目录成功
            {
                err_ret = miniDeleteDirectory(dDes);    //删除目录
                err_mid = miniCloseDirectory(dDes);        //关闭目录
                return err_ret;        //返回状态代码
            } else
                return err_flag;    //打开目录失败
        } else
            return err_flag;    //其他错误
    }
}

/*
* 函数名：GetFileSize
*功能：获得文件（系统外）大小。
* 参数：文件描述符。
* 返回值：long long，表示文件大小，单位：字节。
*/
long long GetFileSize(FILE *fp) {
    long long size = 0;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // 防止空文件被误认为目录
    if (size == 0)
        return 1;
    return size;
}

/*
* 函数名：WildcardMatches
*功能：通配符匹配。
* 参数：两个字符串，第一个喊含有通配符，第二个为待匹配字符串。
* 返回值：匹配结果。
*/
int WildcardMatches(char *wildcard, char *str) 
{
    for (; *wildcard; ++wildcard, ++str)    //循环处理
    {
        if (*wildcard == '*')    //如果 wildcard 的当前字符是 '*'时处理代码
        {
            while (*wildcard == '*') {
                wildcard++;
            }
            for (; *str; ++str) {
                if (WildcardMatches(wildcard, str))
                    return 1;
            }
            return *wildcard == '\0';
        } else     //如果 wildcard 的当前字符非 '*'时处理代码
        {
            if ((*wildcard != '?') && (*wildcard != *str))
                return 0;
        }
    }

    return *str == '\0';    //返回匹配结果
}

/*
* 函数名：att_print
*功能：输出文件/目录属性。
* 参数：一个字符串，为要输出的文件/目录的绝对或相对路径。
* 返回值：返回值：ERROR_CODE，表示执行结果。
*/
ERROR_CODE att_print(char *file_name) {
    ERROR_CODE err_ret = ERR_OTHER, err_mid = ERR_OTHER, err_flag = ERR_OTHER;

    FILE_DESCRIPTOR *fDes;
    err_flag = miniCreateFile(file_name, 0, "r", &fDes);        //打开文件

    if (err_flag == ERR_SUCCESS)        //打开文件成功
    {
        printf("Name:			%s\n", fDes->self->name);        //输出属性
        printf("Size:			%lld Bytes\n", fDes->self->file_size);
        printf("File or Dir:		%s\n", (fDes->self->is_file ? "File" : "Dir"));
        struct tm *t_c = localtime(&fDes->self->create_time);
        printf("Create time:		%d/%d/%d  %02d:%02d:%02d\n", t_c->tm_year + 1900, t_c->tm_mon + 1, t_c->tm_mday,
               t_c->tm_hour, t_c->tm_min, t_c->tm_sec);
        struct tm *t_m = localtime(&fDes->self->modified_time);
        printf("Modified time:		%d/%d/%d  %02d:%02d:%02d\n", t_m->tm_year + 1900, t_m->tm_mon + 1, t_m->tm_mday,
               t_m->tm_hour, t_m->tm_min, t_m->tm_sec);
        struct tm *t_a = localtime(&fDes->self->access_time);
        printf("Access time:		%d/%d/%d  %02d:%02d:%02d\n", t_a->tm_year + 1900, t_a->tm_mon + 1, t_a->tm_mday,
               t_a->tm_hour, t_a->tm_min, t_a->tm_sec);

        err_mid = miniCloseFile(fDes);
        return ERR_SUCCESS;
    }

    if (err_flag == ERR_TYPE_MISMATCH)        //打开的为目录
    {
        DIRECTORY_DESCRIPTOR *dDes;
        err_flag = miniCreateDirectory(file_name, "r", &dDes);    //打开目录
        if (err_flag == ERR_SUCCESS)        //打开目录成功
        {
            printf("Name:			%s\n", dDes->self->name);        //输出属性
            printf("Size:			%lld Bytes\n", dDes->self->file_size);
            printf("File or Dir:		%s\n", (dDes->self->is_file ? "File" : "Dir"));
            struct tm *t_c = localtime(&dDes->self->create_time);
            printf("Create time:		%d/%d/%d  %02d:%02d:%02d\n", t_c->tm_year + 1900, t_c->tm_mon + 1, t_c->tm_mday,
                   t_c->tm_hour, t_c->tm_min, t_c->tm_sec);
            struct tm *t_m = localtime(&dDes->self->modified_time);
            printf("Modified time:		%d/%d/%d  %02d:%02d:%02d\n", t_m->tm_year + 1900, t_m->tm_mon + 1,
                   t_m->tm_mday, t_m->tm_hour, t_m->tm_min, t_m->tm_sec);
            struct tm *t_a = localtime(&dDes->self->access_time);
            printf("Access time:		%d/%d/%d  %02d:%02d:%02d\n", t_a->tm_year + 1900, t_a->tm_mon + 1, t_a->tm_mday,
                   t_a->tm_hour, t_a->tm_min, t_a->tm_sec);

            err_mid = miniCloseDirectory(dDes);
            return ERR_SUCCESS;
        }
		else
			return err_flag;
	}

	return err_flag;
}

// width: 从[到%的总长度
void InitBar(BAR_INFO *bi, int width, float init_ratio) {
    memset(bi->bar, 0, sizeof(bi->bar));
    memset(bi->bar, ' ', width);
    bi->progress_chr = '=';
    bi->arrow_chr = '>';
    // 9包括：'[', '>', ']', " XX.X%"
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