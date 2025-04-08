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

__inline__ static objectp F_car(const struct object *args)
{
	return car(eval(car(args)));
}

__inline__ static objectp F_cdr(const struct object *args)
{
	return cdr(eval(car(args)));
}

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
	result->value.s.str = malloc((arg1->value.s.len + arg2->value.s.len + 1) * sizeof(char));
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
		fprintf(stderr, "; SUBSTR: INDEX OUT OF BOUNDS %ld\n", arg1->value.i);
		return nil;
	}
	if (arg1->value.i + arg2->value.i > arg3->value.s.len)
		offset = arg3->value.s.len - arg1->value.i;
	else
		offset = arg2->value.i;
	result->value.s.str = malloc(offset * sizeof(char));
	if (result->value.s.str == NULL)
	{
		fprintf(stderr, "; SUBSTR: UNABLE TO ALLOCATE MEMORY\n");
		return nil;
	}
	strncpy(result->value.s.str, arg3->value.s.str + arg1->value.i, offset);
	return result;
}

static objectp
F_seq(const struct object *args)
{
	objectp arg1, arg2;
	objectp first = NULL, prev = NULL, p1;
	int i;
	arg1 = eval(car(args));
	arg2 = eval(cadr(args));
	_ASSERTP(arg1->type == OBJ_INTEGER, NOT INTEGER, SEQ, arg1);
	_ASSERTP(arg2->type == OBJ_INTEGER, NOT INTEGER, SEQ, arg2);
	_ASSERTP(arg1->value.i < arg2->value.i, INVALID BOUNDS, SEQ, args);
	i = arg1->value.i;
	do
	{
		p1 = new_object(OBJ_CONS);
		p1->vcar = new_object(OBJ_INTEGER);
		p1->vcar->value.i = i;
		i++;
		if (first == NULL)
			first = p1;
		if (prev != NULL)
			prev->vcdr = p1;
		prev = p1;
	} while (i <= arg2->value.i);
	return first;
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
	case OBJ_TAU:
		return t;
	case OBJ_CONS:
	case OBJ_SET:
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
	if (p->type != OBJ_CONS)
		return nil;

	if (p->vcar->type == OBJ_CONS || p->vcdr->type == OBJ_CONS)
		return nil;
	return t;
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
		fprintf(stderr, "; LOADFILE: ALLOCATING MEMORY\n");
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
		p->value.id = strdup("rational");
		break;
	case OBJ_STRING:
		p->value.id = strdup("string");
		break;
	case OBJ_INTEGER:
		p->value.id = strdup("integer");
		break;
	case OBJ_NULL:
		p->value.id = strdup("undefined");
		break;
	case OBJ_NIL:
		p->value.id = strdup("nil");
		break;
	case OBJ_T:
		p->value.id = strdup("t");
		break;
	case OBJ_CONS:
		p->value.id = strdup("cons");
		break;
	case OBJ_SET:
		p->value.id = strdup("set");
		break;
	case OBJ_TAU:
		p->value.id = strdup("tau");
		break;
	case OBJ_IDENTIFIER:
		p->value.id = strdup("identifier");
		break;
	}
	return p;
}

objectp
F_if(const struct object *args)
{
	if (eval(car(args)) != nil)
		return eval(cadr(args));
	do
	{
		if (cdr(args) == nil)
			break;
		eval(car(args));
	} while ((args = cdr(args)) != nil);
	return eval(car(args));
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
	_ASSERTP(p->type == OBJ_CONS || (p->type == OBJ_SET && !COMPSET(p)), NON CONS ARGUMENT, ORD, p);
	do
	{
		i++;
	} while ((p = cdr(p)) != nil);
	q->value.i = (long int)i;
	return q;
}

objectp
F_nth(const struct object *args)
{
	objectp p, n;
	register int i;
	n = eval(car(args));
	p = eval(car(cdr(args)));
	i = 1L;
	_ASSERTP(n->type == OBJ_INTEGER, NOT INTEGER INDEX, nth, n);
	if (p == nil || p == empty)
	{
		return nil;
	}
	_ASSERTP(p->type == OBJ_CONS && !CONSP(p), NON CONS ARGUMENT, ORD, p);
	if (n->value.i == 1)
		return car(p);
	do
	{
		i++;
	} while (((p = cdr(p)) != nil) && i < n->value.i);
	if (p == nil)
	{
		fprintf(stderr, "; NTH: INDEX '%ld' OUT OF BOUNDS", n->value.i);
		return null;
	}
	return car(p);
}

objectp
F_cons(const struct object *args)
{
	objectp p;
	p = new_object(OBJ_CONS);

	p->vcar = eval(car(args));
	p->vcdr = eval(cadr(args));
	return p;
}

objectp
F_list(const struct object *args)
{
	objectp first, prev, p1;
	p1 = new_object(OBJ_CONS);
	p1->vcar = eval(car(args));
	first = p1;
	prev = p1;
	args = cdr(args);
	if (args == nil)
		return first;
	do
	{
		p1 = new_object(OBJ_CONS);
		p1->vcar = eval(car(args));
		prev->vcdr = p1;
		prev = p1;
	} while ((args = cdr(args)) != nil);
	return first;
}

objectp
F_evlis(const struct object *args)
{
	objectp first = NULL, prev = NULL, p1;
	args = car(args);
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
	int exit_code = 0;
	if (car(args)->type == OBJ_INTEGER)
		exit_code = car(args)->value.i;
	clean_pools();
	clean_objects();
	clean_buffers();
	exit(exit_code);
	return NULL;
}

objectp
F_quote(const struct object *args)
{
	return car(args);
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
			if (var->type == OBJ_CONS || var->type == OBJ_SET)
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
	case OBJ_SET:
		return eqset(a, b);
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
F_defun(const struct object *args)
{
	objectp body;
	body = new_object(OBJ_CONS);
	body->vcar = new_object(OBJ_IDENTIFIER);
	body->vcar->value.id = strdup("lambda");
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
	if (car(cdr(args))->type == OBJ_SET && COMPSET(car(cdr(args))))
	{
		args->vcdr->vcar = eval_set(car(cdr(args)));
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
	_ASSERTP((bind_list->type == OBJ_CONS && bind_list != nil), EMPTY CONS, LABELS, bind_list);
	body = cdr(args);
	_ASSERTP((body->type == OBJ_CONS && body != nil), EMPTY CONS, LABELS, body);

	do
	{
		bind = car(caar(bind_list));
		var = new_object(OBJ_CONS);
		var->vcar = cdr(caar(bind_list));
		var->vcdr = cdar(bind_list);
		s = new_object(OBJ_CONS);
		s->vcar = new_object(OBJ_IDENTIFIER);
		s->vcar->value.id = strdup("lambda");
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
	aux->vcar->value.id = strdup("bquote");
	aux->vcdr = cdr(args);
	body = new_object(OBJ_CONS);
	body->vcar = new_object(OBJ_IDENTIFIER);
	body->vcar->value.id = strdup("lambda");
	body->vcdr = new_object(OBJ_CONS);
	body->vcdr->vcar = cdr(car(args));
	body->vcdr->vcdr = new_object(OBJ_CONS);
	body->vcdr->vcdr->vcar = aux;
	set_object(func_name, body);

	return body;
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
	else if (pn->type == OBJ_INTEGER && pn->value.i >= 3 && pn->value.i <= 8)
		dump_object((int)pn->value.i);
	return null;
}

objectp
F_undef(const struct object *args)
{
	objectp p;
	do
	{
		p = car(args);

		if (p->type != OBJ_IDENTIFIER)
			return nil;
		remove_object(p);
	} while ((args = cdr(args)) != nil);
	return t;
}

objectp
F_print(const struct object *arg)
{
	do
	{
		princ_object(stdout, eval(car(arg)));
		fputc(' ', stdout);
	} while ((arg = cdr(arg)) != nil);
	return null;
}

objectp
F_numberp(const struct object *arg)
{
	if (ISNUMERIC(eval(car(arg))))
		return t;
	return nil;
}
objectp
F_gc(const struct object *arg)
{
	if (arg->type != OBJ_NULL)
		garbage_collect();
	return t;
}
objectp
F_lazy(const struct object *arg)
{
	objectp p;
	p = eval(car(arg));
	if (p == t)
	{
		printf("; LAZY EVAL: ENABLED\n");
		lazy_eval = true;
	}
	else
	{
		printf("; LAZY EVAL: DISABLED\n");
		lazy_eval = false;
	}
	return t;
}

const funcs functions[] = {
	{"*", F_prod},
	{"+", F_add},
	{"/", F_div},
	{"<", F_less},
	{"<=", F_lesseq},
	{"<=>", F_iff},
	{"=", F_eq},
	{"=>", F_imply},
	{">", F_great},
	{">=", F_greateq},
	{"\\", F_diff},
	{"^", F_pow},
	{"and", F_and},
	{"append", F_union},
	{"assoc", F_assoc},
	{"atomp", F_atom},
	{"bquote", F_bquote},
	{"cap", F_cap},
	{"car", F_car},
	{"cat", F_cat},
	{"cdr", F_cdr},
	{"comma", F_comma},
	{"comp", F_complement},
	{"cond", F_cond},
	{"cons", F_cons},
	{"consp", F_consp},
	{"define", F_setq},
	{"defmacro", F_defmacro},
	{"diff", F_diff},
	{"dump", F_dump},
	{"eq", F_eq},
	{"eval", F_eval},
	{"evlis", F_evlis},
	{"gc", F_gc},
	{"if", F_if},
	{"in", F_member},
	{"labels", F_labels},
	{"lazy", F_lazy},
	{"let", F_let},
	{"list", F_list},
	{"load", F_loadfile},
	{"map", F_map},
	{"memberp", F_member},
	{"mod", F_mod},
	{"not", F_not},
	{"notin", F_notin},
	{"nth", F_nth},
	{"numberp", F_numberp},
	{"or", F_or},
	{"ord", F_ord},
	{"par", F_pair},
	{"pop", F_pop},
	{"pow", F_powerset},
	{"print", F_print},
	{"prod", F_setprod},
	{"prog1", F_prog1},
	{"prog2", F_prog2},
	{"progn", F_progn},
	{"push", F_push},
	{"quit", F_quit},
	{"quote", F_quote},
	{"seq", F_seq},
	{"strlen", F_strlen},
	{"subset", F_subset},
	{"subst", F_subst},
	{"substr", F_substr},
	{"symdiff", F_symdiff},
	{"typeof", F_typeof},
	{"undef", F_undef},
	{"union", F_union},
	{"xor", F_xor},
};
