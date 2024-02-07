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

#define ISNUMERIC(x) ((x)->type == OBJ_INTEGER || (x)->type == OBJ_RATIONAL) ? true : false
#define CONSP(p) (              \
	p->type == OBJ_CONS &&      \
	cdr(p)->type != OBJ_CONS && \
	cdr(p) != nil)

#define __PROGN(EXPR)         \
	do                        \
	{                         \
		if (cdr(EXPR) == nil) \
			break;            \
		eval(car(EXPR));      \
	} while ((EXPR = cdr(EXPR)) != nil);

static objectp
F_strlen(const struct object *args)
{
	objectp arg1, result;
	arg1 = eval(car(args));
	_ASSERTP(arg1->type == OBJ_STRING, NOT STRING, STRLEN, arg1);
	result = new_object(OBJ_INTEGER);
	result->value.i = arg1->value.s.len;
	return result;
}
static objectp
F_cat(const struct object *args)
{
	objectp arg1, arg2, result;
	int i, j;
	arg1 = eval(car(args));
	arg2 = eval(cadr(args));
	_ASSERTP(arg1->type == OBJ_STRING, NOT STRING, CAT, arg1);
	_ASSERTP(arg2->type == OBJ_STRING, NOT STRING, CAT, arg2);

	result = new_object(OBJ_STRING);
	result->value.s.str = (char *)malloc((arg1->value.s.len + arg2->value.s.len) * sizeof(char));
	for (i = 0; i < arg1->value.s.len; i++)
	{
		result->value.s.str[i] = arg1->value.s.str[i];
	}
	for (j = 0; j < arg2->value.s.len; j++)
	{
		result->value.s.str[i] = arg2->value.s.str[j];
		i++;
	}
	result->value.s.str[i] = '\0';
	result->value.s.len = arg1->value.s.len + arg2->value.s.len;
	return result;
}
static objectp
F_less(const struct object *args)
{
	objectp arg1, arg2;
	arg1 = eval(car(args));
	arg2 = eval(cadr(args));
	_ASSERTP(ISNUMERIC(arg1), NOT NUMERIC, <, arg1);
	_ASSERTP(ISNUMERIC(arg2), NOT NUMERIC, <, arg2);

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
	_ASSERTP(ISNUMERIC(arg1), NOT NUMERIC, <=, arg1);
	_ASSERTP(ISNUMERIC(arg2), NOT NUMERIC, <=, arg2);

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
	_ASSERTP(ISNUMERIC(arg1), NOT NUMERIC, >, arg1);
	_ASSERTP(ISNUMERIC(arg2), NOT NUMERIC, >, arg2);

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
	_ASSERTP(ISNUMERIC(arg1), NOT NUMERIC, >=, arg1);
	_ASSERTP(ISNUMERIC(arg2), NOT NUMERIC, >=, arg2);

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
			_ASSERTP(false, NOT NUMERIC, ADD, p);
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
			_ASSERTP(false, NOT NUMERIC ARGUMENT, PROD, p);
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
	_ASSERTP(ISNUMERIC(n), NOT NUMERIC, DIV, n);
	_ASSERTP(ISNUMERIC(d), NOT NUMERIC, DIV, d);

	if (n->type == OBJ_INTEGER)
	{
		g = n->value.i;
		n = new_object(OBJ_RATIONAL);
		n->value.r.n = g;
		n->value.r.d = 1L;
	}
	if (d->type == OBJ_INTEGER)
	{
		_ASSERTP(d->value.i != 0, DIVISION BY ZERO, DIV, d);
		g = d->value.i;
		d = new_object(OBJ_RATIONAL);
		d->value.r.n = g;
		d->value.r.d = 1L;
	}
	_ASSERTP(d->value.r.d != 0, ZERO DENOMINATOR, DIV, d);
	_ASSERTP(n->value.r.d != 0, ZERO DENOMINATOR, DIV, n);
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
	case OBJ_STRING:
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
	case OBJ_STRING:
		p->value.id = strdup("STRING");
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
	_ASSERTP(p->type == OBJ_CONS, NON CONS ARGUMENT, ORD, p);
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
	_ASSERTP(p1->type == OBJ_CONS, NOT CONS, MAP, p1);
	_ASSERTP(p->type == OBJ_IDENTIFIER, NOT IDENTIFER, MAP, p);
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
			if (var->type == OBJ_STRING)
				if (!strcmp(var->value.s.str, val->value.s.str))
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
	case OBJ_STRING:
		return strcmp(a->value.s.str, b->value.s.str) == 0 ? t : nil;
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
	_ASSERTP(set->type == OBJ_CONS, NOT CONS, MEMBERP, set);
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
F_pair(const struct object *args)
{
	objectp p, p1, p2, first = NULL, prev = NULL;
	p1 = eval(car(args));
	p2 = eval(cadr(args));
	_ASSERTP(p1->type == OBJ_CONS, NOT CONS, PAIR, p1);
	_ASSERTP(p2->type == OBJ_CONS, NOT CONS, PAIR, p2);
	_ASSERTP(!CONSP(p1), IS PAIR, PAIR, p1);
	_ASSERTP(!CONSP(p2), IS PAIR, PAIR, p2);
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
		_ASSERTP((p->type == OBJ_CONS && !CONSP(p)), NOT CONS, APPEND,p);
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
		_ASSERTP(body->type == OBJ_CONS, NOT CONS, SUBST, body);
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
		_ASSERTP(body->type == OBJ_CONS, NOT CONS, SUBST, body);
		return sst(val, sym, body);
	}
}

objectp
F_labels(const struct object *args)
{
	objectp var, bind, bind_list, body, r, q, s, first, prev;
	first = prev = NULL;
	bind_list = car(args);
	_ASSERTP((bind_list->type == OBJ_CONS && bind_list != nil), NOT NON-EMPTY CONS, LABELS, bind_list);
	body = cdr(args);
	_ASSERTP((body->type == OBJ_CONS && body != nil), NOT NON-EMPTY CONS, LABELS, body);

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
	objectp p, first, prev, arg1;
	first = prev = NULL;
	arg1 = eval(car(args));
	_ASSERTP(arg1->type == OBJ_CONS, NOT CONS, EVLIS, arg1);
	do
	{
		p = new_object(OBJ_CONS);
		p->vcar = eval(car(arg1));
		if (first == NULL)
			first = p;
		if (prev != NULL)
			prev->vcdr = p;
		prev = p;
	} while ((arg1 = cdr(arg1)) != nil);
	return first;
}

objectp
F_pop(const struct object *stack)
{
	objectp p, q, r;
	_ASSERTP(car(stack)->type == OBJ_IDENTIFIER, NOT IDENTIFIER, POP, car(stack));
	r = eval(car(stack));
	_ASSERTP(r->type == OBJ_CONS, NOT CONS, POP, r);
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
	_ASSERTP(car(cdr(args))->type == OBJ_IDENTIFIER, NOT IDENTIFIER, PUSH, car(cdr(args)));
	e = eval(car(args));
	s = eval(car(cdr(args)));
	_ASSERTP(s->type == OBJ_CONS, NOT CONS, PUSH, s);
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
objectp
F_undef(const struct object *args)
{
	objectp p;
	p = car(args);
	if (p->type != OBJ_IDENTIFIER)
		return nil;
	remove_object(p);
	return t;
}
objectp
F_substr(const struct object *args)
{
	objectp arg1, arg2, arg3;
	objectp result;
	int offset = 0;
	arg1 = eval(car(args));
	arg2 = eval(car(cdr(args)));
	arg3 = eval(car(cdr(cdr(args))));
	_ASSERTP(arg1->type == OBJ_INTEGER, NON INTEGER, SUBSTR, arg1);
	_ASSERTP(arg2->type == OBJ_INTEGER, NON INTEGER, SUBSTR, arg2);
	_ASSERTP(arg3->type == OBJ_STRING, NON STRING, SUBSTR, arg3);

	result = new_object(OBJ_STRING);
	if (arg1->value.i > arg3->value.s.len)
	{
		fprintf(stderr, "; INDEX OUT OF BOUNDS\n");
		return nil;
	}
	if (arg1->value.i + arg2->value.i > arg3->value.s.len)
	{
		offset = arg3->value.s.len - arg1->value.i;
	}
	else
	{
		offset = arg2->value.i;
	}
	result->value.s.str = (char *)malloc(offset * sizeof(char));
	_ASSERTP(result->value.s.str, ALLOCATE MEMORY, SUBSTR, result);
	strncpy(result->value.s.str, arg3->value.s.str + arg1->value.i, offset);
	return result;
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
	{"CAT", F_cat},
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
	{"STRLEN", F_strlen},
	{"SUBST", F_subst},
	{"SUBSTR", F_substr},
	{"TYPEOF", F_typeof},
	{"UNDEF", F_undef},
	{"XOR", F_xor}};
