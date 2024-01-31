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

#define CONSP(p) (              \
	p->type == OBJ_CONS &&      \
	cdr(p)->type != OBJ_CONS && \
	cdr(p) != nil)

#define __ASSERTP(COND, ARG, F)                     \
	do                                              \
	{                                               \
		if (COND)                                   \
		{                                           \
			fprintf(stderr, "; " #F ": " #ARG "."); \
			longjmp(je, 1);                         \
		}                                           \
	} while (0)

#define __PROGN(EXPR)         \
	do                        \
	{                         \
		if (cdr(EXPR) == nil) \
			break;            \
		eval(car(EXPR));      \
	} while ((EXPR = cdr(EXPR)) != nil);

static objectp
F_less(const struct object *args)
{
	objectp arg1, arg2;
	arg1 = eval(car(args));
	arg2 = eval(cadr(args));
	__ASSERTP(arg1->type + arg2->type < 10, NON NUMERIC ARGUMENT, <=);

	if (arg1->type == OBJ_INTEGER)
	{
		if (arg2->type == OBJ_INTEGER)
			return (arg1->value.i < arg2->value.i) ? t : nil;
		if (arg2->type == OBJ_RATIONAL)
			return (arg1->value.i * arg2->value.r.d < arg2->value.r.n) ? t : nil;
	}
	else if (arg1->type == OBJ_RATIONAL)
	{
		if (arg2->type == OBJ_INTEGER)
			return (arg1->value.r.n < arg1->value.r.d * arg2->value.i) ? t : nil;
		if (arg2->type == OBJ_RATIONAL)
			return (arg1->value.r.n * arg2->value.r.d < arg2->value.r.n * arg1->value.r.d) ? t : nil;
	}
	return null;
}

static objectp
F_lesseq(const struct object *args)
{
	objectp arg1, arg2;
	arg1 = eval(car(args));
	arg2 = eval(cadr(args));
	__ASSERTP(arg1->type + arg2->type < 10, NON NUMERIC ARGUMENT, <=);

	if (arg1->type == OBJ_INTEGER)
	{
		if (arg2->type == OBJ_INTEGER)
			return (arg1->value.i <= arg2->value.i) ? t : nil;
		if (arg2->type == OBJ_RATIONAL)
			return (arg1->value.i * arg2->value.r.d <= arg2->value.r.n) ? t : nil;
	}
	else if (arg1->type == OBJ_RATIONAL)
	{
		if (arg2->type == OBJ_INTEGER)
			return (arg1->value.r.n <= arg1->value.r.d * arg2->value.i) ? t : nil;
		if (arg2->type == OBJ_RATIONAL)
			return (arg1->value.r.n * arg2->value.r.d <= arg2->value.r.n * arg1->value.r.d) ? t : nil;
	}
	return null;
}

static objectp
F_great(const struct object *args)
{
	objectp arg1, arg2;
	arg1 = eval(car(args));
	arg2 = eval(cadr(args));
	__ASSERTP(arg1->type + arg2->type<10, NON NUMERIC ARGUMENT, >);

	if (arg1->type == OBJ_INTEGER)
	{
		if (arg2->type == OBJ_INTEGER)
			return (arg1->value.i > arg2->value.i) ? t : nil;
		if (arg2->type == OBJ_RATIONAL)
			return (arg1->value.i * arg2->value.r.d > arg2->value.r.n) ? t : nil;
	}
	else if (arg1->type == OBJ_RATIONAL)
	{
		if (arg2->type == OBJ_INTEGER)
			return (arg1->value.r.n > arg1->value.r.d * arg2->value.i) ? t : nil;
		if (arg2->type == OBJ_RATIONAL)
			return (arg1->value.r.n * arg2->value.r.d > arg2->value.r.n * arg1->value.r.d) ? t : nil;
	}
	return null;
}

static objectp
F_greateq(const struct object *args)
{
	objectp arg1, arg2;
	arg1 = eval(car(args));
	arg2 = eval(cadr(args));
	__ASSERTP(arg1->type + arg2->type < 10, NON NUMERIC ARGUMENT, >=);

	if (arg1->type == OBJ_INTEGER)
	{
		if (arg2->type == OBJ_INTEGER)
			return (arg1->value.i >= arg2->value.i) ? t : nil;
		if (arg2->type == OBJ_RATIONAL)
			return (arg1->value.i * arg2->value.r.d >= arg2->value.r.n) ? t : nil;
	}
	else if (arg1->type == OBJ_RATIONAL)
	{
		if (arg2->type == OBJ_INTEGER)
			return (arg1->value.r.n >= arg1->value.r.d * arg2->value.i) ? t : nil;
		if (arg2->type == OBJ_RATIONAL)
			return (arg1->value.r.n * arg2->value.r.d >= arg2->value.r.n * arg1->value.r.d) ? t : nil;
	}
	return null;
}

static objectp
F_add(const struct object *args)
{
	long int i, d, n, g;
	objectp p;

	i = n = 0L;
	d = 1L;
	do
	{
		p = eval(car(args));
		if (p->type == OBJ_INTEGER)
			i += p->value.i;
		else if (p->type == OBJ_RATIONAL)
		{
			n = (n * p->value.r.d) + (d * p->value.r.n);
			d *= p->value.r.d;
		}
		else
			__ASSERTP(p->type, NON NUMERIC ARGUMENT, ADD);
	} while ((args = cdr(args)) != nil);

	if (n == 0L)
	{
		p = new_object(OBJ_INTEGER);
		p->value.i = i;
		return p;
	}
	else
	{
		p = new_object(OBJ_RATIONAL);
		if (i != 0L)
			n += d * i;
		g = gcd(n, d);
		p->value.r.n = n / g;
		p->value.r.d = d / g;
	}
	return eval(p);
}

static objectp
F_prod(const struct object *args)
{
	long int i, d, n, g;
	objectp p;

	i = d = n = 1L;
	do
	{
		p = eval(car(args));
		if (p->type == OBJ_INTEGER)
			i *= p->value.i;
		else if (p->type == OBJ_RATIONAL)
		{
			d *= p->value.r.d;
			n *= p->value.r.n;
		}
		else
			__ASSERTP(p->type, NON NUMERIC ARGUMENT, PROD);
	} while ((args = cdr(args)) != nil);

	if (d == 1L)
	{
		p = new_object(OBJ_INTEGER);
		p->value.i = i;
		return p;
	}
	else
	{
		p = new_object(OBJ_RATIONAL);
		n = n * i;
		g = gcd(n, d);
		p->value.r.n = n / g;
		p->value.r.d = d / g;
	}
	return eval(p);
}

static objectp
F_div(const struct object *args)
{
	long int g, u, v;
	objectp d, n, rat;

	n = eval(car(args));
	d = eval(car(cdr(args)));
	__ASSERTP(n->type + d->type < 10, NON NUMERIC ARGUMENT, DIV);
	if (n->type == OBJ_INTEGER)
	{
		g = n->value.i;
		n = new_object(OBJ_RATIONAL);
		n->value.r.n = g;
		n->value.r.d = 1L;
	}
	if (d->type == OBJ_INTEGER)
	{
		if (d->value.i == 0)
			return null;
		g = d->value.i;
		d = new_object(OBJ_RATIONAL);
		d->value.r.n = g;
		d->value.r.d = 1L;
	}
	__ASSERTP(d->value.r.d == 0 || n->value.r.d == 0,
			  DENOMINATOR IS ZERO, DIV);
	u = n->value.r.n * d->value.r.d;
	v = n->value.r.d * d->value.r.n;
	g = gcd(u, v);
	u = u / g;
	v = v / g;
	if (v == 1L)
	{
		rat = new_object(OBJ_INTEGER);
		rat->value.i = u;
		return rat;
	}
	rat = new_object(OBJ_RATIONAL);
	if (v < 0L)
	{
		rat->value.r.n = -u;
		rat->value.r.d = -v;
	}
	else
	{
		rat->value.r.n = u;
		rat->value.r.d = v;
	}
	return rat;
}

__inline__ static objectp F_car(const struct object *args)
{
	return car(eval(car(args)));
}

__inline__ static objectp F_cdr(const struct object *args)
{
	return cdr(eval(car(args)));
}

objectp
F_atom(const struct object *args)
{
	switch (eval(car(args))->type)
	{
	case OBJ_T:
	case OBJ_NIL:
	case OBJ_IDENTIFIER:
	case OBJ_INTEGER:
	case OBJ_RATIONAL:
		return t;
	case OBJ_CONS:
		return nil;
	default:
		return null;
	}
}

objectp
F_consp(const struct object *args)
{
	objectp p;
	p = eval(car(args));
	if (p->vcar->type == OBJ_CONS && cdr(p->vcar)->type != OBJ_CONS && cdr(p->vcar) != nil)
		return t;
	return nil;
}
objectp
F_loadfile(const struct object *args)
{
	objectp p;
	size_t i;
	char *f_name;
	p = eval(car(args));
	if (p->type != OBJ_IDENTIFIER)
		return null;
	f_name = malloc(strlen(p->value.id) + 4);
	if (f_name == NULL)
	{
		fprintf(stderr, "allocating memory\n");
		return nil;
	}
	strncpy(f_name, p->value.id, strlen(p->value.id));
	strcat(f_name, ".LSP");
	for (i = 0; i < strlen(f_name); i++)
	{
		f_name[i] = tolower(f_name[i]);
	}
	process_input(f_name);
	process_input(NULL);
	free(f_name);
	free(p->value.id);
	free(p);
	return nil;
}

objectp
F_typeof(const struct object *args)
{
	objectp p;
	p = new_object(OBJ_IDENTIFIER);
	switch (eval(car(args))->type)
	{
	case OBJ_RATIONAL:
		p->value.id = strdup("RATIONAL");
		break;
	case OBJ_INTEGER:
		p->value.id = strdup("INTEGER");
		break;
	case OBJ_NULL:
		p->value.id = strdup("UNDEFINED");
		break;
	case OBJ_NIL:
		p->value.id = strdup("NIL");
		break;
	case OBJ_T:
		p->value.id = strdup("T");
		break;
	case OBJ_CONS:
		p->value.id = strdup("CONS");
		break;
	case OBJ_IDENTIFIER:
		p->value.id = strdup("IDENTIFIER");
		break;
	}
	return p;
}

objectp
F_if(const struct object *args)
{
	return eval(car(args)) != nil ? eval(cadr(args)) : F_progn(cddr(args));
}

objectp
F_cond(const struct object *args)
{
	do
	{
		if (eval(car(car(args))) != nil)
			return F_progn(cdar(args));
	} while ((args = cdr(args)) != nil);
	return nil;
}

objectp
F_ord(const struct object *args)
{
	objectp q, p;
	register int i;

	p = eval(car(args));
	i = 0L;
	q = new_object(OBJ_INTEGER);
	if (p == nil)
	{
		q->value.i = 0L;
		return q;
	}
	else if (CONSP(p))
	{
		q->value.i = 1L;
		return q;
	}
	__ASSERTP(p->type != OBJ_CONS, NON CONS ARGUMENT, ORD);
	do
	{
		i++;
	} while ((p = cdr(p)) != nil);
	q->value.i = (long int)i;
	return q;
}

objectp
F_cons(const struct object *args)
{
	objectp p;
	p = new_object(OBJ_CONS);
	p->vcar = eval(car(args));
	p->vcdr = eval(cadr(args));
	return p; // args;
}

objectp
F_list(const struct object *args)
{
	objectp first = NULL, prev = NULL, p1;
	do
	{
		p1 = new_object(OBJ_CONS);
		p1->vcar = eval(car(args));
		if (first == NULL)
			first = p1;
		if (prev != NULL)
			prev->vcdr = p1;
		prev = p1;
	} while ((args = cdr(args)) != nil);
	return first;
}

objectp
F_map(const struct object *args)
{
	objectp p, p1, first, prev;

	first = prev = NULL;
	p1 = eval(cadr(args));
	p = car(args);
	__ASSERTP(p1->type != OBJ_CONS, NOT CONS ARGUMENT, MAP);
	__ASSERTP(p->type != OBJ_IDENTIFIER, NOT IDENTIFER, MAP);
	do
	{
		p = new_object(OBJ_CONS);
		p->vcar = new_object(OBJ_CONS);
		p->vcar->vcar = car(args);
		if (car(p1)->type == OBJ_CONS)
		{
			p->vcar->vcdr = car(p1);
		}
		else
		{
			p->vcar->vcdr = new_object(OBJ_CONS);
			p->vcar->vcdr->vcar = car(p1);
		}
		if (first == NULL)
			first = p;
		if (prev != NULL)
			prev->vcdr = p;
		prev = p;
	} while ((p1 = cdr(p1)) != nil);
	p = first;
	first = prev = NULL;
	do
	{
		p1 = new_object(OBJ_CONS);
		p1->vcar = eval(p->vcar);
		if (first == NULL)
			first = p1;
		if (prev != NULL)
			prev->vcdr = p1;
		prev = p1;
	} while ((p = cdr(p)) != nil);
	return first;
}

objectp
F_quit(const struct object *args)
{
	clean_pools();
	clean_objects();
	clean_buffers();
	exit(0);
	return NULL;
}

objectp
F_quote(const struct object *args)
{
	return car(args);
}

objectp
F_and(const struct object *args)
{
	objectp p1;
	do
	{
		p1 = eval(car(args));
		if (p1 == nil)
			return nil;
	} while ((args = cdr(args)) != nil);
	return p1;
}

objectp
F_or(const struct object *args)
{
	objectp p1;
	do
	{
		p1 = eval(car(args));
		if (p1 != nil)
			return p1;
	} while ((args = cdr(args)) != nil);
	return nil;
}

objectp
F_not(const struct object *args)
{
	return eval(car(args)) != nil ? nil : t;
}

objectp
F_xor(const struct object *args)
{
	objectp first;
	first = eval(car(args));
	do
	{
		if (eval(car(args)) != first)
			return nil;
	} while ((args = cdr(args)) != nil);
	return t;
}

objectp
F_assoc(const struct object *args)
{
	objectp var, val;
	objectp assoc;
	var = eval(car(args));
	assoc = eval(car(cdr(args)));
	do
	{
		val = caar(assoc);
		if (var->type == val->type)
		{
			if (var->type == OBJ_CONS)
				return car(assoc);
			if (var->type == OBJ_IDENTIFIER)
				if (!strcmp(var->value.id, val->value.id))
					return car(assoc);
			if (var->type == OBJ_INTEGER)
				if (var->value.i == val->value.i)
					return car(assoc);
			if (var->type == OBJ_RATIONAL)
				if ((var->value.r.n == val->value.r.n) &&
					(var->value.r.d == val->value.r.d))
					return car(assoc);
			if (var->type == OBJ_T || var->type == OBJ_NIL)
				return car(assoc);
		}
	} while ((assoc = cdr(assoc)) != nil);
	return nil;
}

__inline__ objectp
F_progn(const struct object *args)
{
	do
	{
		if (cdr(args) == nil)
			break;
		eval(car(args));
	} while ((args = cdr(args)) != nil);
	return eval(car(args));
}

objectp
F_prog1(const struct object *args)
{
	objectp p1;
	p1 = eval(car(args));
	args = cdr(args);
	if (args == nil)
		return p1;
	do
	{
		eval(car(args));
	} while ((args = cdr(args)) != nil);
	return p1;
}

objectp
F_prog2(const struct object *args)
{
	objectp p1;
	eval(car(args));
	args = cdr(args);
	if (args == nil)
		return nil;
	p1 = eval(car(args));
	args = cdr(args);
	if (args == nil)
		return p1;
	do
	{
		eval(car(args));
	} while ((args = cdr(args)) != nil);
	return p1;
}

objectp
F_eq(const struct object *args)
{
	objectp a, b;
	a = eval(car(args));
	b = eval(cadr(args));
	// ASSERTP(a == NULL || b == NULL, EQ);
	if (a->type != b->type)
		return nil;
	switch (a->type)
	{
	case OBJ_IDENTIFIER:
		return strcmp(a->value.id, b->value.id) == 0 ? t : nil;
	case OBJ_CONS:
		return eqcons(a, b);
	case OBJ_RATIONAL:
		return ((a->value.r.n == b->value.r.n) &&
				(a->value.r.d == b->value.r.d))
				   ? t
				   : nil;
	case OBJ_INTEGER:
		return (a->value.i == b->value.i) ? t : nil;
	default:
		return t;
	}
	return null;
}

objectp
F_member(const struct object *args)
{
	objectp m, x, set;
	m = eval(car(args));
	set = eval(cadr(args));
	ASSERTP(set->type != OBJ_CONS, MEMBERP);
	do
	{
		x = car(set);
		ASSERTP(x->type == OBJ_NULL, MEMBERP);
		switch (m->type)
		{
		case OBJ_IDENTIFIER:
			if (x->type == OBJ_IDENTIFIER &&
				!strcmp(m->value.id, x->value.id))
				return t;
			break;
		case OBJ_CONS:
			if (x->type == OBJ_CONS && eqcons(m, x) == t)
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

objectp
F_defun(const struct object *args)
{
	objectp body;
	body = new_object(OBJ_CONS);
	body->vcar = new_object(OBJ_IDENTIFIER);
	body->vcar->value.id = strdup("LAMBDA");
	body->vcdr = new_object(OBJ_CONS);
	if (cdr(car(args)) == nil)
	{
		body->vcdr->vcar = nil;
		body->vcdr->vcdr = cdr(args);
	}
	else
	{
		body->vcdr->vcar = cdr(car(args));
		body->vcdr->vcdr = cdr(args);
	}
	set_object(car(car(args)), body);
	return body;
}

objectp
F_setq(const struct object *args)
{
	objectp p2;
	if (car(args)->type == OBJ_CONS)
		return F_defun(args);
	do
	{
		p2 = eval(cadr(args));
		set_object(car(args), p2);
	} while ((args = cddr(args)) != nil);
	return p2;
}

objectp
F_lambda(const struct object *args)
{
	return cdr(args);
}

objectp
F_pair(const struct object *args)
{
	objectp p, p1, p2, first = NULL, prev = NULL;
	p1 = eval(car(args));
	p2 = eval(cadr(args));
	ASSERTP(p1->type != OBJ_CONS || p2->type != OBJ_CONS ||
				CONSP(p1) || CONSP(p2),
			PAIR);
	do
	{
		p = new_object(OBJ_CONS);
		p->vcar = new_object(OBJ_CONS);
		p->vcar->vcar = car(p1);
		p->vcar->vcdr = new_object(OBJ_CONS);
		p->vcar->vcdr->vcar = car(p2);
		if (first == NULL)
			first = p;
		if (prev != NULL)
			prev->vcdr = p;
		prev = p;
	} while ((p1 = cdr(p1)) != nil && (p2 = cdr(p2)) != nil);
	return first;
}

objectp
F_append(const struct object *args)
{
	objectp p, p1, first, prev;
	first = prev = NULL;
	do
	{
		p = eval(car(args));
		ASSERTP((p->type != OBJ_CONS || CONSP(p)), APPEND);
		do
		{
			p1 = new_object(OBJ_CONS);
			p1->vcar = car(p);
			if (first == NULL)
				first = p1;
			if (prev != NULL)
				prev->vcdr = p1;
			prev = p1;
			p = cdr(p);
		} while (p != nil);
	} while ((args = cdr(args)) != nil);
	return first;
}

objectp
F_bquote(const struct object *args)
{
	objectp p;
	if (car(args)->type == OBJ_IDENTIFIER)
		return car(args);

	p = new_object(OBJ_CONS);
	p->value.c.car = args->value.c.car;
	p->value.c.cdr = args->value.c.cdr;
	return car(eval_bquote(p));
}

objectp
F_comma(const struct object *args)
{
	return eval(car(args));
}

objectp
F_let(const struct object *args)
{
	objectp var, bind, bind_list, body, r, q, first, prev;
	first = prev = NULL;
	bind_list = car(args);
	if (bind_list->type != OBJ_CONS)
		return F_progn(cdr(args));
	body = cdr(args);
	do
	{
		bind = caar(bind_list);
		var = eval(cadr(car(bind_list)));
		r = new_object(OBJ_CONS);
		r->vcar = try_eval(caar(bind_list));
		if (first == NULL)
			first = r;
		if (prev != NULL)
			prev->vcdr = r;
		prev = r;
		set_object(bind, var);
	} while ((bind_list = cdr(bind_list)) != nil);

	__PROGN(body);
	q = eval(car(body));

	bind_list = car(args);
	do
	{
		bind = caar(bind_list);
		var = car(first);
		if (var->type == OBJ_NULL)
			remove_object(bind);
		else
			set_object(bind, var);
		first = cdr(first);
	} while ((bind_list = cdr(bind_list)) != nil);

	return q;
}

objectp
F_subst(const struct object *args)
{
	objectp sym, val, body;
	val = eval(car(args));
	if (val->type == OBJ_CONS)
	{
		body = eval(car(cdr(args)));
		ASSERTP(body->type != OBJ_CONS, SUBST);
		do
		{
			body = sst(car(car(val)), cdr(car(val)), body);
		} while ((val = cdr(val)) != nil);
		return body;
	}
	else
	{
		sym = eval(car(cdr(args)));
		body = eval(car(cddr(args)));
		ASSERTP(body->type != OBJ_CONS, SUBST);
		return sst(val, sym, body);
	}
}

objectp
F_labels(const struct object *args)
{
	objectp var, bind, bind_list, body, r, q, s, first, prev;
	first = prev = NULL;
	bind_list = car(args);
	ASSERTP((bind_list->type != OBJ_CONS || bind_list == nil), LABELS);
	body = cdr(args);
	ASSERTP((body->type != OBJ_CONS || body == nil), LABELS);

	do
	{
		bind = car(caar(bind_list));
		var = new_object(OBJ_CONS);
		var->vcar = cdr(caar(bind_list));
		var->vcdr = cdar(bind_list);
		s = new_object(OBJ_CONS);
		s->vcar = new_object(OBJ_IDENTIFIER);
		s->vcar->value.id = strdup("LAMBDA");
		s->vcdr = var;
		r = new_object(OBJ_CONS);
		r->vcar = try_eval(bind);
		if (first == NULL)
			first = r;
		if (prev != NULL)
			prev->vcdr = r;
		prev = r;
		set_object(bind, s);
	} while ((bind_list = cdr(bind_list)) != nil);

	__PROGN(body);
	q = eval(car(body));

	bind_list = car(args);
	do
	{
		bind = car(caar(bind_list));
		var = car(first);
		if (var->type == OBJ_NULL)
			remove_object(bind);
		else
			set_object(bind, var);
		first = cdr(first);
	} while ((bind_list = cdr(bind_list)) != nil);

	return q;
}

objectp
F_eval(const struct object *args)
{
	return eval(eval(car(args)));
}

objectp
F_defmacro(const struct object *args)
{
	objectp func_name, aux, body;

	func_name = car(car(args));
	aux = new_object(OBJ_CONS);
	aux->vcar = new_object(OBJ_IDENTIFIER);
	aux->vcar->value.id = strdup("BQUOTE");
	aux->vcdr = cdr(args);
	body = new_object(OBJ_CONS);
	body->vcar = new_object(OBJ_IDENTIFIER);
	body->vcar->value.id = strdup("LAMBDA");
	body->vcdr = new_object(OBJ_CONS);
	body->vcdr->vcar = cdr(car(args));
	body->vcdr->vcdr = new_object(OBJ_CONS);
	body->vcdr->vcdr->vcar = aux;
	set_object(func_name, body);

	return body;
}

objectp
F_evlis(const struct object *args)
{
	objectp p, first, prev;
	first = prev = NULL;
	args = eval(car(args));
	ASSERTP(args->type != OBJ_CONS, EVLIS);
	do
	{
		p = new_object(OBJ_CONS);
		p->vcar = eval(car(args));
		if (first == NULL)
			first = p;
		if (prev != NULL)
			prev->vcdr = p;
		prev = p;
	} while ((args = cdr(args)) != nil);
	return first;
}

objectp
F_pop(const struct object *stack)
{
	objectp p, q, r;
	ASSERTP(car(stack)->type != OBJ_IDENTIFIER, POP);
	r = eval(car(stack));
	ASSERTP(r->type != OBJ_CONS, POP);
	q = car(r);
	p = cdr(r);
	set_object(car(stack), p);
	return q;
}

objectp
F_push(const struct object *args)
{
	objectp s, e, r, first, prev;
	first = prev = NULL;
	ASSERTP(car(cdr(args))->type != OBJ_IDENTIFIER, PUSH);
	e = eval(car(args));
	s = eval(car(cdr(args)));
	ASSERTP(s->type != OBJ_CONS, PUSH);
	r = new_object(OBJ_CONS);
	r->vcar = e;
	first = r;
	prev = r;
	do
	{
		r = new_object(OBJ_CONS);
		r->vcar = car(s);
		if (first == NULL)
			first = r;
		if (prev != NULL)
			prev->vcdr = r;
		prev = r;
	} while ((s = cdr(s)) != nil);
	set_object(car(cdr(args)), first);
	return first;
}

objectp
F_dump(const struct object *args)
{
	objectp pn;
	pn = eval(car(args));
	if (pn == nil)
		dump_object(0);
	else if (pn->type == OBJ_INTEGER && pn->value.i >= 3 && pn->value.i <= 7)
		dump_object((int)pn->value.i);
	return nil;
}

funcs functions[FUNCS_N] = {
	{"*", F_prod},
	{"+", F_add},
	{"/", F_div},
	{"<", F_less},
	{"<=", F_lesseq},
	{"=", F_eq},
	{">", F_great},
	{">=", F_greateq},
	{"AND", F_and},
	{"APPEND", F_append},
	{"ASSOC", F_assoc},
	{"ATOMP", F_atom},
	{"BQUOTE", F_bquote},
	{"CAR", F_car},
	{"CDR", F_cdr},
	{"COMMA", F_comma},
	{"COND", F_cond},
	{"CONS", F_cons},
	{"CONSP", F_consp},
	{"DEFINE", F_setq},
	{"DEFMACRO", F_defmacro},
	{"DUMP", F_dump},
	{"EQ", F_eq},
	{"EVAL", F_eval},
	{"EVLIS", F_evlis},
	{"IF", F_if},
	{"LABELS", F_labels},
	{"LET", F_let},
	{"LIST", F_list},
	{"LOAD", F_loadfile},
	{"MAP", F_map},
	{"MEMBERP", F_member},
	{"NOT", F_not},
	{"OR", F_or},
	{"ORD", F_ord},
	{"PAIR", F_pair},
	{"POP", F_pop},
	{"PROG1", F_prog1},
	{"PROG2", F_prog2},
	{"PROGN", F_progn},
	{"PUSH", F_push},
	{"QUIT", F_quit},
	{"QUOTE", F_quote},
	{"SUBST", F_subst},
	{"TYPEOF", F_typeof},
	{"XOR", F_xor}};
