#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

#include "sisp.h"
#include "extern.h"

#define LEX_BUF_MAX 16
#define XGETC() ((lex_bufp > lex_buf) ? *--lex_bufp : toupper(fgetc(input_file)))
#define XUNGETC(c) *lex_bufp++ = c
#define CLEAN_BUFFER                                \
	do                                              \
	{                                               \
		XUNGETC(c);                                 \
		while (((c = XGETC()) == '\n') && c != EOF) \
			;                                       \
		longjmp(jl, 1);                             \
	} while (0)

FILE *input_file;
static int lex_buf[LEX_BUF_MAX];
static int *lex_bufp;
char *token_buffer;
static int token_buffer_max;
static int par = 0;
jmp_buf jl;

void init_lex(void)
{
	token_buffer_max = 128;
	token_buffer = (char *)malloc(token_buffer_max);
	lex_bufp = lex_buf;
}

void done_lex(void)
{
	free(token_buffer);
}

static char *
extend_buf(char *p)
{
	int offset;

	offset = p - token_buffer;
	token_buffer_max = token_buffer_max + 64;
	token_buffer = (char *)realloc(token_buffer, token_buffer_max);

	return token_buffer + offset;
}

int gettoken(void)
{
	char *p;
	int c;

	while (true)
	{
		c = XGETC();
		switch (c)
		{
		case ',':
			c = XGETC();
			if (c == ')')
				CLEAN_BUFFER;
			else
			{
				XUNGETC(c);
				c = ',';
				return c;
			}
		case '`':
			c = XGETC();
			if (c == ')')
				CLEAN_BUFFER;
			else
			{
				XUNGETC(c);
				c = '`';
				return c;
			}
		case '(':
			par++;
			return c;
		case ')': // no match
			par--;
			if (par < 0)
			{
				par = 0;
				CLEAN_BUFFER;
			}
			c = ')';
			return c;
		case ' ':
			while (isspace(c = XGETC()))
				;
			XUNGETC(c);
		case '\f':
		case '\t':
		case '\v':
		case '\r':
		case '\n':
			break;
		case ';':
			while ((c = XGETC()) != '\n')
				;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '-':
			p = token_buffer;
			do
			{
				if (p - token_buffer >= token_buffer_max)
					p = extend_buf(p);
				*p++ = c;
				c = XGETC();
			} while (isdigit(c));
			if (c == ' ' || c == ')' || c == '(' || c == '\n' || c == EOF)
			{
				XUNGETC(c);
				*p = '\0';
				return INTEGER;
			}
			else if (c == '/')
			{
				do
				{
					if (p - token_buffer >= token_buffer_max)
						p = extend_buf(p);
					*p++ = c;
					c = XGETC();
				} while (isdigit(c));
				if (c == ' ' || c == ')' || c == '(' || c == '\n' || c == EOF)
				{
					XUNGETC(c);
					*p = '\0';
					return RATIONAL;
				}
			}
			else
				CLEAN_BUFFER;
		case '*':
		case '+':
		case '/':
		case '<':
		case '=':
		case '>':
		case '_':
		case '#':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
			p = token_buffer;
			do
			{
				if (p - token_buffer >= token_buffer_max)
					p = extend_buf(p);
				*p++ = c;
				c = XGETC();
			} while (isalnum(c) || strchr("*+/<=>-_#", c) != NULL);
			XUNGETC(c);
			*p = '\0';
			return IDENTIFIER;
		default:
			return c;
		}
	}
}
