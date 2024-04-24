#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <setjmp.h>
#include <time.h>
#include "sisp.h"
#include "extern.h"
#include "eval.h"
#include "misc.h"
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
int main(int argc, char **argv)
{
    objectp p, q;
    p = new_object(OBJ_CONS);
    	p->value.c.car = new_object(OBJ_STRING);
        p->value.c.car->value.s.str = (char *)malloc(4 * sizeof(char));
        p->value.c.car->value.s.str = strdup("HOLA");
        p->value.c.car->value.s.len = 4;
        p->value.c.cdr = new_object(OBJ_CONS);
        p->value.c.cdr->value.c.car = new_object(OBJ_STRING);
        p->value.c.cdr->value.c.car->value.s.str = (char *)malloc(4 * sizeof(char));
        p->value.c.cdr->value.c.car->value.s.str = strdup("ALOH");
        p->value.c.cdr->value.c.car->value.s.len = 4;
    princ_object(stdout, F_list(p));
    printf("\n");

    return 0;
}