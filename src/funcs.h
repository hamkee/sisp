#ifndef _FUNCS_H
#define _FUNCS_H
// funcs-math
extern objectp F_pow(const struct object *);
extern objectp F_less(const struct object *);
extern objectp F_lesseq(const struct object *);
extern objectp F_great(const struct object *);
extern objectp F_greateq(const struct object *);
extern objectp F_add(const struct object *);
extern objectp F_prod(const struct object *);
extern objectp F_div(const struct object *);
extern objectp F_mod(const struct object *);
extern objectp F_and(const struct object *);
extern objectp F_or(const struct object *);
extern objectp F_not(const struct object *);
extern objectp F_xor(const struct object *);

// funcs-set
extern objectp F_cap(const struct object *);
extern objectp F_union(const struct object *);
extern objectp F_diff(const struct object *);
extern objectp F_subset(const struct object *);
extern objectp F_setprod(const struct object *);
extern objectp F_complement(const struct object *);
extern objectp F_member(const struct object *);
extern objectp F_notin(const struct object *);


extern objectp F_progn(const struct object *);
#define FUNCS_N 65
extern const funcs functions[FUNCS_N];
#define ISNUMERIC(x) ((x)->type == OBJ_INTEGER || (x)->type == OBJ_RATIONAL) \
						 ? true                                              \
						 : false

#define CONSP(p) (              \
	(p)->type == OBJ_CONS &&    \
	cdr(p)->type != OBJ_CONS && \
	cdr(p) != nil)

#define COMPSET(p) (			\
	(p)->type == OBJ_SET &&		\
	cdr(p)->type != OBJ_SET &&	\
	cdr(p) != nil)

#define __PROGN(EXPR)         \
	do                        \
	{                         \
		if (cdr(EXPR) == nil) \
			break;            \
		eval(car(EXPR));      \
	} while ((EXPR = cdr(EXPR)) != nil);

#endif
