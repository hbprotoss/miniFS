#ifndef MINI_BLOCK_H
#define MINI_BLOCK_H

#include "miniError.h"

#define	B	(1)
#define	KB	(1024*B)
#define	MB	(1024*KB)
#define	GB	(1024*MB)

#define	BLOCK_SIZE	(4 * KB)

ERROR_CODE miniReadBlock(int block_number, int buffer_size, void *buf);
ERROR_CODE miniWriteBlock(int block_number, int buffer_size, void *buf);


#endif