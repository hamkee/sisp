#ifndef _MISC_H
extern objectp sst(objectp, objectp, objectp);
extern objectp eqcons(objectp, objectp);
extern objectp eqset(objectp, objectp);
extern long int gcd(long int, long int);
extern void princ_object(FILE *, const struct object *);
extern int unsafe_sanitize(objectp);
extern unsigned long card(objectp);
extern objectp set_to_array(objectp);
#define _MISC_H
#endif
