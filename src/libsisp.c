#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "libsisp.h"

#define FILE_LENGTH sizeof(builtin_functions)

int write_m(int fd)
{
	int i;
	static int NELEMS;
	NELEMS = sizeof(builtin_functions) / sizeof(builtin_functions[0]);
	if (lseek(fd, FILE_LENGTH + 1, SEEK_SET) == -1)
		return -1;
	if (lseek(fd, 0, SEEK_SET) == -1)
		return -1;
	for (i = 0; i < NELEMS; i++)
		if (write(fd, builtin_functions[i], strlen(builtin_functions[i])) == -1)
			return -1;
	close(fd);
	return 0;
}
