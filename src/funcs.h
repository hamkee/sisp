#ifndef _FUNCS_H
#define _FUNCS_H

extern objectp F_progn(const struct object *);
#define FUNCS_N 59
extern const funcs functions[FUNCS_N];
#define ISNUMERIC(x) (                                                          \
						 (x)->type == OBJ_INTEGER || (x)->type == OBJ_RATIONAL) \
						 ? true                                                 \
						 : false

#define CONSP(p) (              \
	(p)->type == OBJ_CONS &&    \
	cdr(p)->type != OBJ_CONS && \
	cdr(p) != nil)

#define EXTSET(p) (			\
	(p)->type == OBJ_SET &&	\
	cdr(p)->type != OBJ_SET	\
	)

#define __PROGN(EXPR)         \
	do                        \
	{                         \
		if (cdr(EXPR) == nil) \
			break;            \
		eval(car(EXPR));      \
	} while ((EXPR = cdr(EXPR)) != nil);

#endif
