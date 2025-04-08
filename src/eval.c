#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "sisp.h"
#include "eval.h"
#include "funcs.h"
#include "extern.h"
#include "misc.h"
bool lazy_eval = false;
__inline__ static int
compar(const void *p1, const void *p2)
{
	return strcmp(((const funcs *)p1)->name, ((const funcs *)p2)->name);
}

objectp
handsig(const char *str)
{
	fprintf(stderr, "; %s.", str);
	longjmp(je, 1);
	return null;
}

objectp
eval_rat(const struct object *args)
{
	objectp result;
	long int n, d, g;
	short sign = 0;

	n = args->value.r.n;
	d = args->value.r.d;
	_ASSERTP(d != 0L, DIVISION BY ZERO, EVAL RAT, args);
	if (n < 0)
	{
		sign = 1;
		n = -n;
	}
	if (d < 0)
	{
		sign += 1;
		d = -d;
	}
	g = gcd(n, d);

	if (d / g == 1L)
	{
		result = new_object(OBJ_INTEGER);
		result->value.i = n / g;
		if (sign == 1)
			result->value.i = -result->value.i;
	}
	else
	{
		result = new_object(OBJ_RATIONAL);
		result->value.r.n = n / g;
		result->value.r.d = d / g;
		if (sign == 1)
			result->value.r.n = -result->value.r.n;
	}

	return result;
}

objectp
sst_lazy(objectp b, objectp v, objectp body)
{

	objectp p, first, prev, q;
	first = prev = NULL;

	do
	{
		p = car(body);
		q = new_object(OBJ_CONS);
		if (p->type != OBJ_IDENTIFIER && p->type != OBJ_CONS && p->type != OBJ_SET)
			q->vcar = p;
		else if (p->type == OBJ_IDENTIFIER)
			q->vcar = (!strcmp(p->value.id, b->value.id)) ? v : p;
		else if (p->type == OBJ_CONS || p->type == OBJ_SET)
			q->vcar = sst_lazy(b, v, p);
		else
			q->vcar = p;
		if (first == NULL)
			first = q;
		if (prev != NULL)
			prev->vcdr = q;
		prev = q;
	} while ((body = cdr(body)) != nil);
	return first;
}

objectp
sst_lazy_p(objectp args, objectp body)
{

	objectp p, first, prev, q, tmp;
	first = prev = NULL;

	do
	{
		p = car(body);
		q = new_object(OBJ_CONS);
		tmp = args;
		if (p->type != OBJ_IDENTIFIER && p->type != OBJ_CONS && p->type != OBJ_SET)
			q->vcar = p;
		else if (p->type == OBJ_IDENTIFIER)
		{
			do
			{
				if (!strcmp(p->value.id, tmp->vcar->vcar->value.id))
				{
					q->vcar = tmp->vcar->vcdr->vcar;
					break;
				}
				else
				{
					q->vcar = p;
				}
			} while ((tmp = cdr(tmp)) != nil);
		}
		else if (p->type == OBJ_CONS || p->type == OBJ_SET)
			q->vcar = sst_lazy_p(args, p);
		else
			q->vcar = p;
		if (first == NULL)
			first = q;
		if (prev != NULL)
			prev->vcdr = q;
		prev = q;
	} while ((body = cdr(body)) != nil);
	return first;
}

static objectp
eval_func_lazy(objectp p, objectp args)
{
	objectp head_args, b, q, bind_list, body, M;
	q = head_args = b = NULL;
	bind_list = cadr(p);
	body = cddr(p);
	if (bind_list == nil)
	{
		if (!setjmp(je))
		{
			q = eval(car(cdr(cdr(p))));
			return q;
		}
	}
	do
	{
		M = new_object(OBJ_CONS);
		M->vcar = new_object(OBJ_CONS);
		M->vcar->vcar = car(bind_list);
		M->vcar->vcdr = new_object(OBJ_CONS);
		M->vcar->vcdr->vcar = eval(car(args));
		if (head_args == NULL)
			head_args = M;
		if (b != NULL)
			b->vcdr = M;
		b = M;
		args = cdr(args);
	} while ((bind_list = cdr(bind_list)) != nil);
	bind_list = cadr(p);
	args = head_args;
	body = sst_lazy_p(args, body);
	if (!setjmp(je))
	{
		do
		{
			if (cdr(body) == nil)
				break;
			eval(car(body));
		} while ((body = cdr(body)) != nil);
		q = eval(car(body));
	}
	return q;
}

static objectp
eval_func(objectp p, objectp args)
{
	objectp head_args, b, q, M, bind_list;
	q = head_args = b = NULL;
	bind_list = cadr(p);

	if (bind_list == nil)
	{
		if (!setjmp(je))
		{
			q = eval(car(cdr(cdr(p))));
			return q;
		}
	}
	do
	{
		M = new_object(OBJ_CONS);
		M->vcar = new_object(OBJ_CONS);
		M->vcar->vcar = eval(car(args));
		M->vcar->vcdr = new_object(OBJ_CONS);
		M->vcar->vcdr->vcar = try_object(car(bind_list));
		if (head_args == NULL)
			head_args = M;
		if (b != NULL)
			b->vcdr = M;
		b = M;
		args = cdr(args);
	} while ((bind_list = cdr(bind_list)) != nil);
	bind_list = cadr(p);
	args = head_args;
	do
	{
		set_object(car(bind_list), caar(head_args));
		head_args = cdr(head_args);
	} while ((bind_list = cdr(bind_list)) != nil);
	q = null;
	b = cddr(p);
	if (!setjmp(je))
	{
		do
		{
			if (cdr(b) == nil)
				break;
			eval(car(b));
		} while ((b = cdr(b)) != nil);
		q = eval(car(b));
	}
	bind_list = cadr(p);
	head_args = args;
	do
	{
		if (cadr(car(head_args))->type == OBJ_NULL)
			remove_object(car(bind_list));
		else
			set_object(car(bind_list), cadr(car(head_args)));
		head_args = cdr(head_args);
	} while ((bind_list = cdr(bind_list)) != nil);
	return q;
}

objectp
eval_bquote(objectp args)
{
	objectp p1, r, first, prev;
	first = prev = NULL;
	do
	{
		p1 = car(args);
		r = new_object(OBJ_CONS);
		if (p1->type == OBJ_CONS)
			r->vcar = eval_bquote(p1);
		else if (p1->type == OBJ_IDENTIFIER && !strcmp(p1->value.id, "comma"))
		{
			r->vcar = eval(args);
			if (first == NULL)
				first = r;
			return car(first);
		}
		else
			r->vcar = p1;
		if (first == NULL)
			first = r;
		if (prev != NULL)
			prev->vcdr = r;
		prev = r;
	} while ((args = cdr(args)) != nil);

	return first;
}

static unsigned int rbp = 0;
objectp
eval_cons(const struct object *p)
{

	objectp func_name, q;
	funcs key, *item;
	unsigned long n_args = 0;
	_ASSERTP(car(p)->type == OBJ_IDENTIFIER, NOT A FUNCTION, EVAL, car(p));

	for (unsigned int i = 0; i < CACHE_SIZE; i++)
	{
		if (function_cache[i].name != NULL &&
			!strcmp(p->vcar->value.id, function_cache[i].name))
		{
			return (lazy_eval == true) ? eval_func_lazy(function_cache[i].func, p->vcdr)
									   : eval_func(function_cache[i].func, p->vcdr);
		}
	}

	if (!strcmp(p->vcar->value.id, "lambda"))
	{
		q = new_object(OBJ_IDENTIFIER);
		q->value.id = strdup("lambda");
		return q;
	}

	key.name = p->vcar->value.id;
	if ((item = bsearch(&key, functions,
						sizeof(functions) / sizeof(functions[0]),
						sizeof(functions[0]), compar)) != NULL)
		return item->func(cdr(p));

	func_name = get_object(p->vcar);

	if (card(p->vcdr) != (n_args = card(cadr(func_name))))
	{
		fprintf(stderr, "; %s: EXPECTED %lu ARGUMENTS.", car(p)->value.id, n_args);
		longjmp(je, 1);
	}
	function_cache[rbp].name = strdup(p->vcar->value.id);
	function_cache[rbp].func = func_name;
	rbp++;
	if (rbp == CACHE_SIZE)
		rbp = 0;

	return (lazy_eval == true) ? eval_func_lazy(func_name, p->vcdr)
							   : eval_func(func_name, p->vcdr);
}

objectp
eval_set(const struct object *p)
{
	objectp p1, r, first, prev, tmp = NULL;
	first = prev = NULL;
	r = nil;
	if (p == empty)
	{
		return empty;
	}
	if (COMPSET(p))
	{
		p1 = car(p);
		first = new_object(OBJ_SET);
		first->value.c.car = tau;
		first->value.c.cdr = sst(car(p), tau, cdr(p));
		tmp = cdr(p);
		do
		{
			if (car(tmp)->type == OBJ_IDENTIFIER)
			{
				r = try_object(car(tmp));
				if (r != null)
					first->value.c.cdr = sst(car(tmp), r, cdr(p));
			}
		} while ((tmp = cdr(tmp)) != nil);
		return first;
	}

	do
	{
		p1 = p->value.c.car;
		if (!in_set(p1, first))
		{
			r = new_object(OBJ_SET);
			r->value.c.car = p1;
		}
		else
		{
			continue;
		}
		if (first == NULL)
			first = r;
		if (prev != NULL)
			prev->vcdr = r;
		prev = r;
	} while ((p = cdr(p)) != nil);

	return first;
}
