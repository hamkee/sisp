#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "sisp.h"
#include "extern.h"

#define PARSE_COMMA                              \
	p = new_object(OBJ_CONS);                    \
	p->value.c.car = new_object(OBJ_IDENTIFIER); \
	p->value.c.car->value.id = strdup("COMMA");  \
	p->value.c.cdr = new_object(OBJ_CONS);       \
	p->value.c.cdr->value.c.car = parse_object(0)

#define PARSE_QUOTE                              \
	p = new_object(OBJ_CONS);                    \
	p->value.c.car = new_object(OBJ_IDENTIFIER); \
	p->value.c.car->value.id = strdup("QUOTE");  \
	p->value.c.cdr = new_object(OBJ_CONS);       \
	p->value.c.cdr->value.c.car = parse_object(0)

#define PARSE_BACKQUOTE                          \
	p = new_object(OBJ_CONS);                    \
	p->value.c.car = new_object(OBJ_IDENTIFIER); \
	p->value.c.car->value.id = strdup("BQUOTE"); \
	p->value.c.cdr = new_object(OBJ_CONS);       \
	p->value.c.cdr->value.c.car = parse_object(0)

static int thistoken;

__inline__ static objectp parse_form(void)
{
	objectp p, first, prev;
	first = prev = NULL;

	while ((thistoken = gettoken()) != ')' && thistoken != EOF)
	{
		if (thistoken == '.')
		{
			thistoken = gettoken();
			if (prev == NULL)
				longjmp(jb, 1);
			prev->value.c.cdr = parse_object(1);
			if ((thistoken = gettoken()) != ')')
				longjmp(jb, 1);
			break;
		}
		p = new_object(OBJ_CONS);
		if (first == NULL)
			first = p;
		if (prev != NULL)
			prev->value.c.cdr = p;
		p->value.c.car = parse_object(1);
		prev = p;
	}

	return (first == NULL) ? null : first;
}

objectp
parse_object(int havetoken)
{
	objectp p = NULL;
	long int d, n;
	char *delim;

	if (!havetoken)
	{
		if (!setjmp(jl))
			thistoken = gettoken();
		else
		{
			while ((havetoken = getchar()) != '\n' && havetoken != EOF)
				;
			longjmp(jb, 1);
		}
	}
	switch (thistoken)
	{
	case EOF:
		return NULL;
	case '`':
		PARSE_BACKQUOTE;
		break;
	case ',':
		PARSE_COMMA;
		break;
	case '(':
		p = parse_form();
		break;
	case '\'':
		PARSE_QUOTE;
		break;
	case IDENTIFIER:
		if (!strcmp(token_buffer, "T"))
			p = t;
		else if (!strcmp(token_buffer, "NIL"))
			p = nil;
		else if ((p = search_object_identifier(token_buffer)) == NULL)
		{
			p = new_object(OBJ_IDENTIFIER);
			p->value.id = strdup(token_buffer);
		}
		break;
	case INTEGER:
		n = strtol(token_buffer, NULL, 10);
		if ((p = search_object_integer(n)) == NULL)
		{
			p = new_object(OBJ_INTEGER);
			p->value.i = n;
		}
		break;
	case RATIONAL:
		n = strtol(token_buffer, &delim, 10);
		delim++;
		d = strtol(delim, NULL, 10);
		if ((p = search_object_rational(n, d)) == NULL)
		{
			p = new_object(OBJ_RATIONAL);
			p->value.r.n = n;
			p->value.r.d = d;
		}
		break;
	default:
		while ((havetoken = getchar()) != '\n' && havetoken != EOF)
			;
		longjmp(jb, 1);
	}

	return p;
}
