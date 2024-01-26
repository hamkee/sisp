#ifndef _MISC_H
extern objectp 	sst(objectp, objectp, objectp);
extern objectp 	areeq(objectp, objectp);
extern objectp 	eqcons(objectp, objectp);
extern long int gcd(long int, long int);
extern void 	princ_object(FILE *, objectp);
extern int 		card(objectp);
#define _MISC_H
#endif
