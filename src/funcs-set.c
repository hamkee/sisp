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
		case OBJ_SET:
			if (eqset(x, p) == t)
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
// (and (in tau p1) (not (in tau p2)))
objectp diffbyextension(objectp p1, objectp p2)
{
	objectp ret;

	ret = new_object(OBJ_SET);
	ret->value.c.car = tau;
	ret->value.c.cdr = new_object(OBJ_CONS);
	ret->vcdr->vcar = new_object(OBJ_IDENTIFIER);
	ret->vcdr->vcar->value.id = strdup("and");
	ret->vcdr->vcdr = new_object(OBJ_CONS);
	ret->vcdr->vcdr->vcar = cdr(p1);
	ret->vcdr->vcdr->vcdr = new_object(OBJ_CONS);
	ret->vcdr->vcdr->vcdr->vcar = new_object(OBJ_CONS);
	ret->vcdr->vcdr->vcdr->vcar->vcar = new_object(OBJ_IDENTIFIER);
	ret->vcdr->vcdr->vcdr->vcar->vcar->value.id = strdup("not");
	ret->vcdr->vcdr->vcdr->vcar->vcdr = new_object(OBJ_CONS);
	ret->vcdr->vcdr->vcdr->vcar->vcdr->vcar = cdr(p2);

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
	objectp a, b, tmp, ret;
	a = eval(car(args));
	b = eval(car(cdr(args)));
	if (a == empty)
		return t;

	_ASSERTP(a->type == OBJ_SET && !COMPSET(a), NOT EXTENSION SET, SUBSET, a);
	_ASSERTP(b->type == OBJ_SET, NOT SET, SUBSET, b);
	if (COMPSET(b))
	{
		do
		{
			tmp = sst(tau, a->vcar, cdr(b));
			ret = eval(tmp);
			if (ret == nil)
				return ret;
		} while ((a = cdr(a)) != nil);
	}
	else
	{
		do
		{
			if (!in_set(a->vcar, b))
				return nil;
		} while ((a = cdr(a)) != nil);
	}
	return t;
}
objectp F_notin(const struct object *arg)
{
	return (F_member(arg) == nil) ? t : nil;
}
objectp
sanitize(objectp a)
{
	objectp f1, f2, f3;
	objectp ctau, cdtau;

	ctau = new_object(OBJ_CONS);
	ctau->vcar = new_object(OBJ_IDENTIFIER);
	ctau->vcar->value.id = strdup("car");
	ctau->vcdr = new_object(OBJ_CONS);
	ctau->vcdr->vcar = tau;

	cdtau = new_object(OBJ_CONS);
	cdtau->vcar = new_object(OBJ_IDENTIFIER);
	cdtau->vcar->value.id = strdup("cdr");
	cdtau->vcdr = new_object(OBJ_CONS);
	cdtau->vcdr->vcar = tau;

	f1 = car(cdr(a));
	if (f1->type != OBJ_IDENTIFIER)
		return t;
	if (strcmp(f1->value.id, "and"))
		return t;
	if (car(cdr(cdr(a)))->type != OBJ_CONS)
		return t;
	f2 = car(car(cdr(cdr(a))));
	if (f2->type != OBJ_IDENTIFIER)
		return t;
	if (strcmp(f2->value.id, "in"))
		return t;
	if (cdr(cdr(cdr(a)))->type != OBJ_CONS)
		return t;
	if (car(cdr(cdr(cdr(a))))->type != OBJ_CONS)
		return t;
	f3 = car(car(cdr(cdr(cdr(a)))));
	if (f3->type != OBJ_IDENTIFIER)
		return t;
	if (strcmp(f3->value.id, "in"))
		return t;
	if (cdr(car(cdr(cdr(a))))->type != OBJ_CONS)
		return t;
	if (!eqcons(ctau, car(cdr(car(cdr(cdr(a)))))))
		return t;
	if (cdr(car(cdr(cdr(cdr(a)))))->type != OBJ_CONS)
		return t;
	if (!eqcons(cdtau, car(cdr(car(cdr(cdr(cdr(a))))))))
	return t;
	if(cdr(cdr(car(cdr(cdr(cdr(a))))))->type != OBJ_CONS)
	return t;
	if(!COMPSET(car(cdr(cdr(car(cdr(cdr(cdr(a)))))))))
		return t;
	if (cdr(cdr(car(cdr(cdr(cdr(a))))))->type != OBJ_CONS)
		return t;
	if (!COMPSET(car(cdr(cdr(car(cdr(cdr(cdr(a)))))))))
		return t;

	return nil;
}
objectp
prodbyextension(objectp a, objectp b)
{
	objectp ret, cons, in, inb;
	if (sanitize(a) == nil)
	{
		fprintf(stderr, "; PROD: MALFORMED FIRST OPERAND: ");
		princ_object(stderr, a);
		fprintf(stderr, "\n; FIRST ARGUMENT IS AN EXTENSION SET OF THE FORM"
						" (AND (IN EXPR) (IN EXPR)) AND THIS LEAD TO LEFT ASSOCIATIVE LISTS.\n"
						"; THIS IS NOT SUPPORTED.\n");
		longjmp(je, 1);
	}
	in = new_object(OBJ_CONS);
	in->vcar = new_object(OBJ_IDENTIFIER);
	in->vcar->value.id = strdup("in");
	in->vcdr = new_object(OBJ_CONS);
	in->vcdr->vcar = new_object(OBJ_CONS);
	in->vcdr->vcar->vcar = new_object(OBJ_IDENTIFIER);
	in->vcdr->vcar->vcar->value.id = strdup("car");
	in->vcdr->vcar->vcdr = new_object(OBJ_CONS);
	in->vcdr->vcar->vcdr->vcar = tau;
	in->vcdr->vcar->vcdr->vcdr = nil;
	in->vcdr->vcdr = new_object(OBJ_CONS);
	in->vcdr->vcdr->vcar = a;

	inb = new_object(OBJ_CONS);
	inb->vcar = new_object(OBJ_IDENTIFIER);
	inb->vcar->value.id = strdup("in");
	inb->vcdr = new_object(OBJ_CONS);
	inb->vcdr->vcar = new_object(OBJ_CONS);
	inb->vcdr->vcar->vcar = new_object(OBJ_IDENTIFIER);
	inb->vcdr->vcar->vcar->value.id = strdup("cdr");
	inb->vcdr->vcar->vcdr = new_object(OBJ_CONS);
	inb->vcdr->vcar->vcdr->vcar = tau;
	inb->vcdr->vcar->vcdr->vcdr = nil;
	inb->vcdr->vcdr = new_object(OBJ_CONS);
	inb->vcdr->vcdr->vcar = b;

	cons = new_object(OBJ_CONS);
	cons->vcar = new_object(OBJ_IDENTIFIER);
	cons->vcar->value.id = strdup("consp");
	cons->vcdr = new_object(OBJ_CONS);
	cons->vcdr->vcar = tau;

	ret = new_object(OBJ_SET);
	ret->value.c.car = tau;
	ret->value.c.cdr = new_object(OBJ_CONS);
	ret->vcdr->vcar = new_object(OBJ_IDENTIFIER);
	ret->vcdr->vcar->value.id = strdup("and");
	ret->vcdr->vcdr = new_object(OBJ_CONS);
	ret->vcdr->vcdr->vcar = in;
	ret->vcdr->vcdr->vcdr = new_object(OBJ_CONS);
	ret->vcdr->vcdr->vcdr->vcar = inb;

	// ret->vcdr->vcdr->vcar = cons;
	// ret->vcdr->vcdr->vcdr = new_object(OBJ_CONS);
	// ret->vcdr->vcdr->vcdr->vcar = in;
	// ret->vcdr->vcdr->vcdr->vcdr = new_object(OBJ_CONS);
	// ret->vcdr->vcdr->vcdr->vcdr->vcar = inb;
	return ret;
}
objectp
flatten(objectp p)
{
	objectp first = NULL, prev = NULL, p1, p2, p3;
	p = car(p);

	do
	{
		p1 = car(p);
		if (p1->type == OBJ_CONS)
		{
			do
			{
				p2 = car(p1);

				p3 = new_object(OBJ_CONS);
				p3->value.c.car = p2;
				if (first == NULL)
					first = p3;
				if (prev != NULL)
					prev->vcdr = p3;
				prev = p3;
			} while ((p1 = cdr(p1)) != nil);
		}
		else
		{
			p3 = new_object(OBJ_CONS);
			p3->value.c.car = p1;
			if (first == NULL)
				first = p3;
			if (prev != NULL)
				prev->vcdr = p3;
			prev = p3;
		}
	} while ((p = cdr(p)) != nil);

	return first;
}
objectp
F_setprod(const struct object *args)
{
	objectp b, a, tmp;
	objectp first = NULL, prev = NULL, p1, p2;
	a = eval(car(args));
	b = eval(car(cdr(args)));
	_ASSERTP(a->type == OBJ_SET, NOT SET, PROD, a);
	_ASSERTP(b->type == OBJ_SET, NOT SET, PROD, b);
	if (COMPSET(a) && COMPSET(b))
		return prodbyextension(a, b);
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
			p2 = new_object(OBJ_SET);
			p2->vcar = flatten(p1);
			if (first == NULL)
				first = p2;
			if (prev != NULL)
				prev->vcdr = p2;
			prev = p2;

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
				 (arg1->type == OBJ_SET && arg2->type == OBJ_SET),
			 NOT CONS, DIFF, args);
	if (COMPSET(arg1))
		return diffbyextension(arg1, arg2);
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

	_ASSERTP(p->type == OBJ_SET, NOT SET, COMPLEMENT, p);

	ret = new_object(OBJ_SET);
	ret->value.c.car = tau;
	ret->value.c.cdr = new_object(OBJ_CONS);
	ret->vcdr->vcar = new_object(OBJ_IDENTIFIER);
	ret->vcdr->vcar->value.id = strdup("not");
	ret->vcdr->vcdr = new_object(OBJ_CONS);
	if (COMPSET(p))
	{
		ret->vcdr->vcdr->vcar = cdr(p);
	}
	else
	{
		ret->vcdr->vcdr->vcar = new_object(OBJ_CONS);
		ret->vcdr->vcdr->vcar->vcar = new_object(OBJ_IDENTIFIER);
		ret->vcdr->vcdr->vcar->vcar->value.id = strdup("in");
		ret->vcdr->vcdr->vcar->vcdr = new_object(OBJ_CONS);
		ret->vcdr->vcdr->vcar->vcdr->vcar = tau;
		ret->vcdr->vcdr->vcar->vcdr->vcdr = new_object(OBJ_CONS);
		ret->vcdr->vcdr->vcar->vcdr->vcdr->vcar = p;
	}
	return ret;
}
objectp
F_member(const struct object *args)
{
	objectp m, x, set, ret, tmp, tv;
	m = eval(car(args));
	set = eval(cadr(args));
	if (COMPSET(set))
	{
		if (m->type == OBJ_CONS)
		{
			if (m->vcdr != nil)
			{
				tv = new_object(OBJ_CONS);
				tv->vcar = new_object(OBJ_IDENTIFIER);
				tv->vcar->value.id = strdup("quote");
				tv->vcdr = new_object(OBJ_CONS);
				tv->vcdr->vcar = m;
			}
			else
			{
				tv = m->vcar;
			}
		}
		else
		{
			tv = m;
		}
		tmp = sst(tau, tv, cdr(set));
		ret = eval(tmp);
		_ASSERTP(ret == nil || ret == t, NOT LOGIC EXPRESSION, MEMBERP, cdr(set));
		return ret;
	}
	_ASSERTP(set->type == OBJ_CONS || set->type == OBJ_SET, NOT CONS, MEMBERP, set);
	do
	{
		x = car(set);
		_ASSERTP(x->type != OBJ_NULL, IS NULL, MEMBERP, x);
		switch (m->type)
		{
		case OBJ_IDENTIFIER:
			if (x->type == OBJ_IDENTIFIER &&
				!strcmp(m->value.id, x->value.id))
				return t;
			break;
		case OBJ_STRING:
			if (x->type == OBJ_STRING &&
				!strcmp(m->value.s.str, x->value.s.str))
				return t;
			break;
		case OBJ_CONS:
			if (x->type == OBJ_CONS && eqcons(m, x) == t)
				return t;
			break;
		case OBJ_SET:
			if (x->type == OBJ_SET && eqset(m, x) == t)
				return t;
			break;
		case OBJ_T:
			if (x->type == OBJ_T)
				return t;
			break;
		case OBJ_NIL:
			if (x->type == OBJ_NIL)
				return t;
			break;
		case OBJ_INTEGER:
			if (x->type == OBJ_INTEGER)
				if (x->value.i == m->value.i)
					return t;
			break;
		case OBJ_RATIONAL:
			if (x->type == OBJ_RATIONAL)
				if ((x->value.r.d == m->value.r.d) &&
					(x->value.r.n == m->value.r.n))
					return t;
			break;
		default:
			break;
		}
		set = cdr(set);
	} while (set->type != OBJ_NIL);
	return nil;
}
