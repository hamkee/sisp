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
#include "libsisp.h"

int main(int argc, char **argv)
{
	init_objects();
	input_file = fmemopen(builtin_functions, strlen(builtin_functions), "r");
	process_file();
	fclose(input_file);
	if (argc > 1)
		while (*++argv)
			process_input(*argv);

	process_input(NULL);

	return 0;
}
