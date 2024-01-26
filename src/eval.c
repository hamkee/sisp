#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sisp.h"
#include "eval.h"
#include "funcs.h"
#include "extern.h"
#include "misc.h"

static int 
compar(const void *p1, const void *p2)
{
	return strcmp(((funcs *)p1)->name, ((funcs *)p2)->name);
}

objectp 
handsig(const char *str)
{
	printf("; %s.", str);
	longjmp(je, 1);
	return null;
}

objectp
eval_rat(objectp args)
{
	long int n, d, g;
	bool sign = true;
	n = args->value.r.n;
	d = args->value.r.d;

	ASSERTP(d==0L,RATIONAL);
	if(n<0) {
		sign = false;
		n = -n;
	}
	if(d<0) {
		sign = false;
		d = -d;
	}
	g = gcd(n,d);

	if(d/g == 1L) {
		args = new_object(OBJ_INTEGER);
		args->value.i = n/g;
		if(!sign)
			args->value.i = -args->value.i;
	} else {
		args->value.r.n = n/g;
		args->value.r.d = d/g;
		if(!sign)
			args->value.r.n = -args->value.r.n;
	}

	return args;
}

static objectp 
eval_func(objectp p, objectp args)
{
	objectp head_args, b, q, M, bind_list;
	q = head_args = b = NULL;

	bind_list = cadr(p);
	do {
		M = new_object(OBJ_CONS);
		M->vcar = new_object(OBJ_CONS);
		M->vcar->vcar = eval(car(args));
		M->vcar->vcdr = new_object(OBJ_CONS);
		M->vcar->vcdr->vcar = try_eval(car(bind_list));
		if (head_args == NULL) 
			head_args = M;
		if (b != NULL) 
			b->vcdr = M;
		b = M;
		args = cdr(args);
	} while ((bind_list = cdr(bind_list)) != nil);
	bind_list = cadr(p);
	args = head_args;	
	do {
		set_object(car(bind_list), caar(head_args));
		head_args = cdr(head_args);
	} while ((bind_list = cdr(bind_list)) != nil);
	q = null;
	b = cddr(p);
	if (!setjmp(je)) {
		do {
			if (cdr(b) == nil)
				break;
			eval(car(b));
		} while ((b = cdr(b)) != nil);
		q = eval(car(b));
	}
	bind_list = cadr(p);
	head_args = args;
	do {
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
	
	do { 
		p1 = car(args);
		r = new_object(OBJ_CONS);	
		if (p1->type == OBJ_CONS)
			r->vcar = eval_bquote(p1);
		else if(p1->type == OBJ_IDENTIFIER && 
				!strcmp(p1->value.id, "COMMA")) { 
			r->vcar = eval(args);
			if (first == NULL)
				first = r;
			if (prev != NULL)
				prev->vcdr = r;
			prev = r;
			return car(first);
		} else 
			r->vcar = p1;
		if (first == NULL)
			first = r;
		if (prev != NULL)
			prev->vcdr = r;
		prev = r;
	} while ((args = cdr(args)) != nil);

	return first;
}

objectp 
eval_cons(objectp p)
{
	objectp func_name;
	funcs key, *item;
	int n_args = 0;
	
	ASSERTP(car(p)->type != OBJ_IDENTIFIER, EVAL_CONS);
	if (!strcmp(car(p)->value.id, "LAMBDA")) 
		return p;
	key.name = car(p)->value.id;
	if ((item = bsearch(&key, functions, 
					sizeof(functions)/sizeof(functions[0]), 
					sizeof(functions[0]), compar)) != NULL) 
		return item->func(cdr(p));
	func_name = get_object(car(p));
	if (card(cdr(p)) != (n_args = card(cadr(func_name)))) {
		printf("; %s: EXPECTED %d ARGUMENTS.", car(p)->value.id, n_args);
		longjmp(je,1);
	}
	
	return eval_func(func_name, cdr(p));
}
