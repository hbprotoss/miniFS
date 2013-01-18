#ifndef MINI_ERROR_H
#define MINI_ERROR_H

typedef int ERROR_CODE;

#define	ERR_SUCCESS			0x00
#define	ERR_TYPE_MISMATCH		0x01
#define	ERR_NOT_FOUND			0x02
#define	ERR_BUFFER_OVERFLOW		0x03
#define	ERR_READ_ONLY			0x04
#define	ERR_OTHER			0x05
#define ERR_OUT_OF_RANGE		0x06
#define ERR_UNKNOWN_FILE_SYSTEM		0x07
#define ERR_INVALID_FILE_NAME		0x08
#define ERR_UNKNOWN_OPEN_MODE		0x09
#define ERR_NOT_ENOUGH_DISK_SPACE	0x0a
#define ERR_NOT_FORMATED		0x0b
#define ERR_DIRECTORY_NOT_EMPTY		0x0c
#define ERR_PATH_TOO_LONG		0x0d

#ifdef _DEBUG
#define DbgPrint(status)	//fprintf(stdout, "%s::%s::%d Error:%02x\n", __FILE__, __FUNCTION__, __LINE__, status)
#else
#define DbgPrint(status)
#endif

#define BREAK(exp)	if(exp) __asm int 3;;

#endif
