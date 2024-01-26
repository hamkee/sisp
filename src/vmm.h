#ifndef _VMM_H
#define _VMM_H

extern objectp oballoc(a_type);
extern void obfree(objectp);
extern void recycle_pool(a_type);

struct t_pool
{
	size_t used_size;
	size_t free_size;
	struct
	{
		objectp u;
		objectp f;
	} head;
} pool[8];

#endif
