#ifndef LIBSISP_H
const char *builtin_functions[] = {
	"(define (caar x) (car (car x)))\n",
	"(define (cadr x) (car (cdr x)))\n",
	"(define (cdar x) (cdr (car x)))\n",
	"(define (cddr x) (cdr (cdr x)))\n",
	"(define (caaar x) (car (car (car x))))\n",
	"(define (caadr x) (car (car (cdr x))))\n",
	"(define (cadar x) (car (cdr (car x))))\n",
	"(define (caddr x) (car (cdr (cdr x))))\n",
	"(define (cdaar x) (cdr (car (car x))))\n",
	"(define (cdadr x) (cdr (car (cdr x))))\n",
	"(define (cddar x) (cdr (cdr (car x))))\n",
	"(define (cdddr x) (cdr (cdr (cdr x))))\n",
	"(define (butlast l)\n",
	"  (cond ((eq (cdr l) nil) nil)\n",
	"        (t (cons (car l) (butlast (cdr l))))))\n",
	"(define (get-names x)\n",
	"    (cond ((eq x nil) nil)",
	"           (t (cons (caar x) (get-names (cdr x))))))\n",	
	"(define (get-values x)\n",
	"    (cond ((eq x nil) nil)",
	"           (t (cons (cdar x) (get-values (cdr x))))))\n",	
	"(define (unpair x)\n",
	"    (list (get-names x) (get-values x)))\n",
	"(define (rplaca x y)\n",
	"  (cons y (cdr x)))\n",
	"(define (rplacd x y)\n"
	"  (cons (car x) y))\n",
	"(define (alt x)\n",
	"  (cond ((or (not x)\n",
	"	      (not (cdr x))) x)\n",
	"	 (t (cons (car x) (alt (cddr x))))))\n",
	"(define (succ x y)\n",
	"  (cond ((or (not y) (not (cdr y))) nil)\n",
	"	((eq (car y) x) (cadr y))\n",
	"	(t (succ x (cdr y)))))\n",
	"(define (pred x y)\n",
	"  (cond ((or (not y) (not (cdr y))) nil)\n",
	"	((eq (cadr y) x) (car y))\n",
	"	(t (pred x (cdr y)))))\n"
};
#define LIBSISP_H
#endif