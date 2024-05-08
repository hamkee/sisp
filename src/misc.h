#ifndef _MISC_H
extern objectp sst(objectp, objectp, objectp);
extern objectp eqcons(objectp, objectp);
extern objectp eqset(objectp, objectp);
extern long int gcd(long int, long int);
extern void princ_object(FILE *, const struct object *);
extern unsigned long card(objectp);
#define _MISC_H
#endif
