#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <setjmp.h>
#include <time.h>
#include "sisp.h"
#include "extern.h"
#include "eval.h"
#include "misc.h"

int main(int argc, char **argv)
{
	int fd;
	char buildf[] = "/tmp/sisp.XXXXXXXX";

	init_objects();
	if ((fd = mkstemp(buildf)) > 0)
	{
		if (write_m(fd) != 0)
		{
			unlink(buildf);
			fprintf(stderr, "error creating file\n");
		}
		process_input(buildf);
		unlink(buildf);
	}
	else
		fprintf(stderr, "cannot load functions\n");
	if (argc > 1)
		while (*++argv)
			process_input(*argv);
	process_input(NULL);

	return 0;
}
