#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sisp.h"
#include "eval.h"
#include "extern.h"

objectp eqcons(objectp, objectp);

void princ_object(FILE *fout, const struct object *p)
{
	switch (p->type)
	{
	case OBJ_NIL:
		fputs("nil", fout);
		break;
	case OBJ_T:
		fputc('t', fout);
		break;
	case OBJ_TAU:
		fputs("tau", fout);
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
	case OBJ_SET:
		fputc('{', fout);
		do
		{
			princ_object(fout, p->vcar);
			p = p->vcdr;
			if (p != nil)
			{
				fputc(' ', fout);
				if (p->type != OBJ_SET)
				{
					fputs(": ", fout);
					princ_object(fout, p);
				}
			}
		} while (p != nil && p->type == OBJ_SET);
		fputc('}', fout);
		break;
	default:
		return;
	}
}
objectp
eqset(objectp a, objectp b)
{
	objectp c, tmp, count;
	int found = -1;
	count = b;
	if ((cdr(a)->type != OBJ_SET && cdr(a) != nil) ||
		(cdr(b)->type != OBJ_SET && cdr(b) != nil))
	{
		fprintf(stderr, "; EQSET: SET NOT BY EXTENSION\n");
		return nil;
	}
	do
	{
		tmp = b;
		found = -1;
		do
		{
			if (a->vcar->type == tmp->vcar->type)
			{
				switch (a->vcar->type)
				{
				case OBJ_INTEGER:
					if (a->vcar->value.i == tmp->vcar->value.i)
					{
						found = 1;
					}
					break;
				case OBJ_RATIONAL:
					if (a->vcar->value.r.n == tmp->vcar->value.r.n &&
						a->vcar->value.r.d == tmp->vcar->value.r.d)
					{
						found = 1;
					}
					break;
				case OBJ_T:
				case OBJ_NIL:
					found = 1;
					break;
				case OBJ_IDENTIFIER:
					if (strcmp(a->vcar->value.id, tmp->vcar->value.id) == 0)
					{
						found = 1;
					}
					break;
				case OBJ_STRING:
					if (strcmp(a->vcar->value.s.str, tmp->vcar->value.s.str) == 0)
					{
						found = 1;
					}
					break;
				case OBJ_CONS:
					c = eqcons(a->vcar, tmp->vcar);
					if (c == t)
					{
						found = 1;
					}
					break;
				case OBJ_SET:
					c = eqset(a->vcar, tmp->vcar);
					if (c == t)
					{
						found = 1;
					}
					break;
				case OBJ_NULL:
					return nil;
				default:
					found = -1;
					break;
				}
			}
			if (found == 1)
				break;
		} while ((tmp = cdr(tmp)) != nil);
		if (found == -1)
		{
			return nil;
		}
		count = cdr(count);
	} while (((a = cdr(a)) != nil));
	if (count != nil)
		return nil;

	return t;
}

objectp
eqcons(objectp a, objectp b)
{
	objectp c;

	if (a->type != b->type)
		return nil;
	switch (a->type)
	{
	case OBJ_NULL:
		return nil;
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
	register unsigned long i = 0;
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
			case OBJ_TAU:
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
			case OBJ_SET:
				q->vcar = (eqset(b, p) == t) ? v : p;
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
