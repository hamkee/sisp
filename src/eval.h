#ifndef _EVAL_H

#define ASSERTP(EXPR,F) 					\
	do { if(EXPR) { 						\
	fprintf(stderr,"; " #F ": " #EXPR ".");	\
	longjmp(je,1); } 						\
	} while(0)

#define eval(p) (                                   \
	(p->type) == OBJ_T ? t :                        \
	(p->type) == OBJ_NIL ? nil :                    \
	(p->type) == OBJ_NULL ? handsig("EVAL ERROR") :	\
	(p->type) == OBJ_INTEGER ? (p) :                \
	(p->type) == OBJ_RATIONAL ? eval_rat(p) :       \
	(p->type) == OBJ_IDENTIFIER ?  get_object(p) :	\
	(p->type) == OBJ_CONS ? eval_cons(p) : (p))

#define try_eval(p) (                               \
	(p->type) == OBJ_T ? t :                        \
	(p->type) == OBJ_NIL ? nil :                    \
	(p->type) == OBJ_NULL ? null :                  \
	(p->type) == OBJ_INTEGER ? (p) :                \
	(p->type) == OBJ_RATIONAL ? eval_rat(p) :       \
	(p->type) == OBJ_IDENTIFIER ?  try_object(p) :	\
	(p->type) == OBJ_CONS ? eval_cons(p) : (p))

#define car(p) (									\
		(p->type) == OBJ_CONS ? p->value.c.car :	\
		(p->type) == OBJ_NIL  ? nil	: handsig("UNDEFINED"))

#define cdr(p) (									\
		(p->type) == OBJ_CONS ? p->value.c.cdr :	\
		(p->type) == OBJ_NIL  ? nil : handsig("UNDEFINED"))

#define cddr(p) cdr(cdr(p))
#define cadr(p) car(cdr(p))
#define caar(p)	car(car(p))
#define cdar(p) cdr(car(p))
extern objectp eval_bquote(objectp);
objectp eval_rat(objectp);
objectp eval_cons(objectp);

#define _EVAL_H

#endif
