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
	lseek(fd, FILE_LENGTH + 1, SEEK_SET);
	lseek(fd, 0, SEEK_SET);
	for (i = 0; i < NELEMS; i++)
		if (write(fd, builtin_functions[i], strlen(builtin_functions[i])) == -1)
			return -1;
	close(fd);
	return 0;
}
