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

jmp_buf jb, je;

void clean_buffers(void)
{
	free(token_buffer);
	fclose(input_file);
}

static void
process_file(void)
{
	objectp p = NULL;
	init_lex();
	while (true)
	{
		p = (!setjmp(jb)) ? parse_object(0) : NULL;
		done_lex();
		if (p != NULL)
		{
			if (!setjmp(je))
				eval(p);
		}
		else
			break;
		garbage_collect();
	}
	clean_buffers();
}

static void
process_stdin(void)
{
	objectp p, q;
	p = q = NULL;
	init_lex();

	while (1)
	{
		printf(": ");
		p = (!setjmp(jb)) ? parse_object(0) : NULL;
		done_lex();
		if (p != NULL)
			q = (!setjmp(je)) ? eval(p) : NULL;
		else
			fprintf(stderr, "; PARSER ERROR.");
		if (p != NULL && q != NULL)
			princ_object(stdout, q);
		puts("");
		garbage_collect();
	}
	free(token_buffer);
}

void process_input(const char *filename)
{
	if (filename != NULL && strcmp(filename, "-") != 0)
	{
		if ((input_file = fopen(filename, "r")) == NULL)
		{
			fprintf(stderr, "; %s: FILE NOT FOUND\n", filename);
			return;
		}
	}
	else
		input_file = stdin;
	if (input_file != stdin)
		process_file();
	else
		process_stdin();
}

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
