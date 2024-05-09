#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

#include "sisp.h"
#include "extern.h"
#include "funcs.h"
#include "eval.h"
#include "misc.h"
int in_set(objectp x, objectp y)
{
	objectp p;
	if (y == nil || y == NULL)
		return 0;
	do
	{
		p = car(y);
		if (x->type != p->type)
			continue;
		switch (x->type)
		{
		case OBJ_NIL:
		case OBJ_T:
			if (x == p)
				return 1;
			break;
		case OBJ_IDENTIFIER:
			if (!strcmp(x->value.id, p->value.id))
				return 1;
			break;
		case OBJ_STRING:
			if (!strcmp(x->value.s.str, p->value.s.str))
				return 1;
			break;
		case OBJ_INTEGER:
			if (x->value.i == p->value.i)
				return 1;
			break;
		case OBJ_RATIONAL:
			if (x->value.r.d == p->value.r.d &&
				x->value.r.n == p->value.r.n)
				return 1;
			break;
		case OBJ_CONS:
			if (eqcons(x, p) == t)
				return 1;
			break;
		default:
			break;
		}
	} while ((y = cdr(y)) != nil);
	return 0;
}
objectp
unionbyextension(const struct object *args)
{
	objectp p, p1, first, prev, ret;
	first = prev = NULL;

	do
	{
		p = eval(car(args));
		p1 = new_object(OBJ_CONS);
		p1->vcar = cdr(p);
		_ASSERTP(COMPSET(p), NOT COMPREHENSION SET, UNION, p);
		if (first == NULL)
			first = p1;
		if (prev != NULL)
			prev->vcdr = p1;
		prev = p1;
	} while ((args = cdr(args)) != nil);
	ret = new_object(OBJ_SET);
	ret->value.c.car = tau;
	ret->value.c.cdr = new_object(OBJ_CONS);
	ret->vcdr->vcar = new_object(OBJ_IDENTIFIER);
	ret->vcdr->vcar->value.id = strdup("or");
	ret->vcdr->vcdr = first;

	return ret;
}
objectp capbyextension(const struct object *args)
{
	objectp p, p1, first, prev, ret;
	first = prev = NULL;

	do
	{
		p = eval(car(args));
		p1 = new_object(OBJ_CONS);
		p1->vcar = cdr(p);
		_ASSERTP(COMPSET(p), NOT COMPREHENSION SET, CAP, p);
		if (first == NULL)
			first = p1;
		if (prev != NULL)
			prev->vcdr = p1;
		prev = p1;
	} while ((args = cdr(args)) != nil);
	ret = new_object(OBJ_SET);
	ret->value.c.car = tau;
	ret->value.c.cdr = new_object(OBJ_CONS);
	ret->vcdr->vcar = new_object(OBJ_IDENTIFIER);
	ret->vcdr->vcar->value.id = strdup("and");
	ret->vcdr->vcdr = first;

	return ret;
}
objectp
F_union(const struct object *args)
{
	objectp p, p1, first, prev;
	a_type type;
	first = prev = NULL;
	p = eval(car(args));
	type = p->type;
	if (COMPSET(p))
		return unionbyextension(args);
	_ASSERTP(((p->type == OBJ_CONS && !CONSP(p)) || p->type == OBJ_SET), NOT CONS, APPEND, p);

	do
	{
		p1 = new_object(type);
		p1->vcar = car(p);
		if (first == NULL)
			first = p1;
		if (prev != NULL)
			prev->vcdr = p1;
		prev = p1;
		p = cdr(p);
	} while (p != nil);
	args = cdr(args);
	if (args == nil)
		return (type == OBJ_SET) ? eval_set(first) : first;
	do
	{
		p = eval(car(args));
		_ASSERTP(((p->type == OBJ_CONS && !CONSP(p)) || (p->type == OBJ_SET && !COMPSET(p))), NOT CONS, APPEND, p);
		_ASSERTP((p->type == type), TYPE MISMATCH, APPEND, p);

		do
		{
			p1 = new_object(type);
			p1->vcar = car(p);
			if (first == NULL)
				first = p1;
			if (prev != NULL)
				prev->vcdr = p1;
			prev = p1;
			p = cdr(p);
		} while (p != nil);
	} while ((args = cdr(args)) != nil);
	return (type == OBJ_SET) ? eval_set(first) : first;
}

objectp
F_subset(const struct object *args)
{
	objectp a, b;
	a = eval(car(args));
	b = eval(car(cdr(args)));
	_ASSERTP(a->type == OBJ_SET && !COMPSET(a), NOT EXTENSION SET, SUBSET, a);
	_ASSERTP(b->type == OBJ_SET && !COMPSET(b), NOT EXTENSION SET, SUBSET, b);
	do
	{
		if (!in_set(a->vcar, b))
			return nil;
	} while ((a = cdr(a)) != nil);
	return t;
}
objectp
F_setprod(const struct object *args)
{
	objectp b, a, tmp;
	objectp first = NULL, prev = NULL, p1;
	a = eval(car(args));
	b = eval(car(cdr(args)));
	_ASSERTP(a->type == OBJ_SET && !COMPSET(a), NOT EXTENSION SET, PROD, a);
	_ASSERTP(b->type == OBJ_SET && !COMPSET(b), NOT EXTENSION SET, PROD, b);

	tmp = b;
	do
	{
		b = tmp;
		do
		{

			p1 = new_object(OBJ_SET);
			p1->vcar = new_object(OBJ_CONS);
			p1->vcar->vcar = car(a);
			p1->vcar->vcdr = new_object(OBJ_CONS);
			p1->vcar->vcdr->vcar = car(b);
			if (first == NULL)
				first = p1;
			if (prev != NULL)
				prev->vcdr = p1;
			prev = p1;

		} while ((b = cdr(b)) != nil);
	} while ((a = cdr(a)) != nil);
	return first;
}
objectp
F_cap(const struct object *args)
{
	objectp arg1, arg2, tmp, c;
	objectp first = NULL, prev = NULL, p1;
	a_type type;
	arg1 = eval(car(args));
	arg2 = eval(cadr(args));
	if (COMPSET(arg1) && COMPSET(arg2))
		return capbyextension(args);
	_ASSERTP((arg1->type == OBJ_CONS && arg2->type == OBJ_CONS) ||
				 (arg1->type == OBJ_SET && !COMPSET(arg1) &&
				  !COMPSET(arg2) && arg2->type == OBJ_SET),
			 NOT CONS, CAP, args);
	type = arg1->type;
	do
	{
		tmp = arg2;
		do
		{
			if (arg1->vcar->type == tmp->vcar->type)
			{
				switch (arg1->vcar->type)
				{
				case OBJ_INTEGER:
					if (arg1->vcar->value.i == tmp->vcar->value.i)
					{
						p1 = new_object(type);
						p1->vcar = new_object(OBJ_INTEGER);
						p1->vcar->value.i = arg1->vcar->value.i;
						if (first == NULL)
							first = p1;
						if (prev != NULL)
							prev->vcdr = p1;
						prev = p1;
					}
					break;
				case OBJ_RATIONAL:
					if (arg1->vcar->value.r.n == tmp->vcar->value.r.n &&
						arg1->vcar->value.r.d == tmp->vcar->value.r.d)
					{
						p1 = new_object(type);
						p1->vcar = new_object(OBJ_RATIONAL);
						p1->vcar->value.r.n = arg1->vcar->value.r.n;
						p1->vcar->value.r.d = arg1->vcar->value.r.d;
						if (first == NULL)
							first = p1;
						if (prev != NULL)
							prev->vcdr = p1;
						prev = p1;
					}
					break;
				case OBJ_T:
				case OBJ_NIL:
					p1 = new_object(type);
					p1->vcar = arg1->vcar;
					if (first == NULL)
						first = p1;
					if (prev != NULL)
						prev->vcdr = p1;
					prev = p1;
					break;
				case OBJ_IDENTIFIER:
					if (strcmp(arg1->vcar->value.id, tmp->vcar->value.id) == 0)
					{
						p1 = new_object(type);
						p1->vcar = new_object(OBJ_IDENTIFIER);
						p1->vcar->value.id = strdup(arg1->vcar->value.id);
						if (first == NULL)
							first = p1;
						if (prev != NULL)
							prev->vcdr = p1;
						prev = p1;
					}
					break;
				case OBJ_STRING:
					if (strcmp(arg1->vcar->value.s.str, tmp->vcar->value.s.str) == 0)
					{
						p1 = new_object(type);
						p1->vcar = new_object(OBJ_STRING);
						p1->vcar->value.s.str = strdup(arg1->vcar->value.s.str);
						if (first == NULL)
							first = p1;
						if (prev != NULL)
							prev->vcdr = p1;
						prev = p1;
					}
					break;
				case OBJ_CONS:
				case OBJ_SET:
					c = (tmp->vcar->type == OBJ_SET) ? eqset(arg1->vcar, tmp->vcar) : eqcons(arg1->vcar, tmp->vcar);
					if (c == t)
					{
						p1 = new_object(type);
						p1->vcar = arg1->vcar;
						if (first == NULL)
							first = p1;
						if (prev != NULL)
							prev->vcdr = p1;
						prev = p1;
					}
					break;

				default:
					break;
				}
			}
		} while ((tmp = cdr(tmp)) != nil);
	} while ((arg1 = cdr(arg1)) != nil);

	return first;
}

objectp
F_diff(const struct object *args)
{
	objectp arg1, arg2, tmp, c;
	objectp first = NULL, prev = NULL, p1;
	int found = 0;
	arg1 = eval(car(args));
	arg2 = eval(cadr(args));
	_ASSERTP((arg1->type == OBJ_CONS && arg2->type == OBJ_CONS) ||
				 (arg1->type == OBJ_SET && !COMPSET(arg1) && arg2->type == OBJ_SET && !COMPSET(arg2)),
			 NOT CONS, DIFF, args);
	do
	{
		tmp = arg2;
		found = 0;
		do
		{
			if (arg1->vcar->type == tmp->vcar->type)
			{
				switch (arg1->vcar->type)
				{
				case OBJ_INTEGER:
					if (arg1->vcar->value.i == tmp->vcar->value.i)
					{
						found = 1;
					}
					break;
				case OBJ_RATIONAL:
					if (arg1->vcar->value.r.n == tmp->vcar->value.r.n &&
						arg1->vcar->value.r.d == tmp->vcar->value.r.d)
					{
						found = 1;
					}
					break;
				case OBJ_T:
				case OBJ_NIL:
					found = 1;
					break;
				case OBJ_IDENTIFIER:
					if (strcmp(arg1->vcar->value.id, tmp->vcar->value.id) == 0)
					{
						found = 1;
					}
					break;
				case OBJ_STRING:
					if (strcmp(arg1->vcar->value.s.str, tmp->vcar->value.s.str) == 0)
					{
						found = 1;
					}
					break;
				case OBJ_CONS:
				case OBJ_SET:
					c = (tmp->vcar->type == OBJ_SET) ? eqset(arg1->vcar, tmp->vcar) : eqcons(arg1->vcar, tmp->vcar);
					if (c == t)
					{
						found = 1;
					}
					break;
				default:
					found = 0;
					break;
				}
			}
		} while ((tmp = cdr(tmp)) != nil);
		if (found == 0)
		{
			p1 = new_object(arg1->type);
			p1->vcar = arg1->vcar;
			if (first == NULL)
				first = p1;
			if (prev != NULL)
				prev->vcdr = p1;
			prev = p1;
		}

	} while ((arg1 = cdr(arg1)) != nil);

	return first == NULL ? nil : first;
}
objectp
F_complement(const struct object *args)
{
	objectp p, ret;
	p = eval(car(args));

	_ASSERTP(p->type == OBJ_SET && COMPSET(p), NOT COMPREHENSION SET, COMPLEMENT, p);
	ret = new_object(OBJ_SET);
	ret->value.c.car = tau;
	ret->value.c.cdr = new_object(OBJ_CONS);
	ret->vcdr->vcar = new_object(OBJ_IDENTIFIER);
	ret->vcdr->vcar->value.id = strdup("not");
	ret->vcdr->vcdr = new_object(OBJ_CONS);
	ret->vcdr->vcdr->vcar = cdr(p);

	return ret;
}
