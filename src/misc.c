#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sisp.h"
#include "eval.h"
#include "extern.h"

void princ_object(FILE *fout, const struct object *p)
{
	switch (p->type)
	{
	case OBJ_NIL:
		fputs("NIL", fout);
		break;
	case OBJ_T:
		fputc('T', fout);
		break;
	case OBJ_IDENTIFIER:
		if (p->value.id != NULL)
			fputs(p->value.id, fout);
		break;
	case OBJ_STRING:
		if (p->value.s.str != NULL)
			fprintf(fout, "\"%s\"", p->value.s.str);
		break;
	case OBJ_NULL:
		fputc('\0', fout);
		break;
	case OBJ_INTEGER:
		fprintf(fout, "%li", p->value.i);
		break;
	case OBJ_RATIONAL:
		fprintf(fout, "%li/%li", p->value.r.n, p->value.r.d);
		break;
	case OBJ_CONS:
		fputc('(', fout);
		do
		{
			princ_object(fout, p->vcar);
			p = p->vcdr;
			if (p != nil)
			{
				fputc(' ', fout);
				if (p->type != OBJ_CONS)
				{
					fputs(". ", fout);
					princ_object(fout, p);
				}
			}
		} while (p != nil && p->type == OBJ_CONS);
		fputc(')', fout);
		break;
	default:
		return;
	}
}

objectp
eqcons(objectp a, objectp b)
{
	objectp c;

	if (a->type != b->type)
		return nil;
	switch (a->type)
	{
	case OBJ_INTEGER:
		return (a->value.i == b->value.i) ? t : nil;
	case OBJ_RATIONAL:
		return (a->value.r.n == b->value.r.n &&
				a->value.r.d == b->value.r.d)
				   ? t
				   : nil;
	case OBJ_T:
	case OBJ_NIL:
		return t;
	case OBJ_IDENTIFIER:
		return strcmp(a->value.id, b->value.id) == 0 ? t : nil;
	case OBJ_STRING:
		return strcmp(a->value.s.str, b->value.s.str) == 0 ? t : nil;
	case OBJ_CONS:
		c = eqcons(car(a), car(b));
		if (c == nil)
			return nil;
		break;
	default:
		return t;
	}
	return c->type == OBJ_T ? eqcons(cdr(a), cdr(b)) : nil;
}

long int
gcd(long int a, long int b)
{
	long int tmp;
	if (a < b)
	{
		tmp = a;
		a = b;
		b = tmp;
	}
	while (b != 0L)
	{
		tmp = a % b;
		a = b;
		b = tmp;
	}
	return a;
}

unsigned long card(objectp p)
{
	register int i = 0;
	while (p != nil && p->type == OBJ_CONS)
	{
		p = p->vcdr;
		++i;
	}
	return i;
}

objectp
sst(objectp b, objectp v, objectp body)
{
	objectp p, first, prev, q;
	first = prev = NULL;
	do
	{
		p = car(body);
		q = new_object(OBJ_CONS);
		if (b->type != p->type && p->type != OBJ_CONS)
			q->vcar = p;
		else
			switch (p->type)
			{
			case OBJ_NIL:
			case OBJ_T:
				q->vcar = (p->type == b->type) ? v : p;
				break;
			case OBJ_IDENTIFIER:
				q->vcar = (!strcmp(p->value.id, b->value.id)) ? v : p;
				break;
			case OBJ_STRING:
				q->vcar = (!strcmp(p->value.s.str, b->value.s.str)) ? v : p;
				break;
			case OBJ_INTEGER:
				q->vcar = (p->value.i == b->value.i) ? v : p;
				break;
			case OBJ_RATIONAL:
				q->vcar = (p->value.r.d == b->value.r.d &&
						   p->value.r.n == b->value.r.n)
							  ? v
							  : p;
				break;
			case OBJ_CONS:
				q->vcar = (eqcons(b, p) == t) ? v : p;
				if (q->vcar == p)
					q->vcar = sst(b, v, p);
				break;
			default:
				q->vcar = p;
				break;
			}
		if (first == NULL)
			first = q;
		if (prev != NULL)
			prev->vcdr = q;
		prev = q;
	} while ((body = cdr(body)) != nil);
	
	return first;
}
