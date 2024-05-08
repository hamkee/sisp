#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

#include "sisp.h"
#include "extern.h"
#include "misc.h"
#include "vmm.h"

objectp nil;
objectp t;
objectp null;
objectp u;

static object_pairp setobjs_list = NULL;
static unsigned int gc_id = 0;

__inline__ objectp
new_object(a_type type)
{
	objectp p;

	p = oballoc(type);
	p->type = type;
	p->gc = 0;
	p->next = pool[type].head.u;
	pool[type].head.u = p;
	pool[type].used_size++;
	if (type == OBJ_CONS || type == OBJ_SET)
		p->vcar = p->vcdr = nil;
	if (type == OBJ_STRING)
		p->value.s.len = 0;

	return p;
}

__inline__ objectp
search_object_rational(const long int num, const long int den)
{
	objectp p;

	for (p = pool[OBJ_RATIONAL].head.u; p != NULL; p = p->next)
		if (p->value.r.n == num && p->value.r.d == den)
			return p;

	return (objectp)NULL;
}

__inline__ objectp
search_object_integer(const long int i)
{
	objectp p;

	for (p = pool[OBJ_INTEGER].head.u; p != NULL; p = p->next)
		if (p->value.i == i)
			return p;

	return (objectp)NULL;
}
__inline__ objectp
search_object_string(const char *s)
{
	objectp p;

	for (p = pool[OBJ_STRING].head.u; p != NULL; p = p->next)
		if (!strcmp(p->value.s.str, s))
			return p;

	return (objectp)NULL;
}
__inline__ objectp
search_object_identifier(const char *s)
{
	objectp p;

	for (p = pool[OBJ_IDENTIFIER].head.u; p != NULL; p = p->next)
		if (!strcmp(p->value.id, s))
			return p;

	return (objectp)NULL;
}

void init_objects(void)
{
	int i, j;
	objectp new_heap_list = NULL;

	null = new_object(OBJ_NULL);
	nil = new_object(OBJ_NIL);
	t = new_object(OBJ_T);
	u = (objectp )malloc(OBJ_SIZE);
	for (i = 3; i <= 8; i++)
	{
		pool[i].head.u = NULL;
		pool[i].head.f = NULL;
		pool[i].free_size = 0;
		pool[i].used_size = 0;
	}
	for (i = 3; i <= 4; i++)
	{
		pool[i].head.f = malloc(OBJ_SIZE);
		if (pool[i].head.f == NULL)
			fprintf(stderr, "allocating memory\n");
		pool[i].head.f->next = NULL;
		new_heap_list = pool[i].head.f;
		j = (i == 3 ? 31 : 255);
		pool[i].free_size = (unsigned int)j + 1;
		while (j--)
		{
			pool[i].head.f->next = malloc(OBJ_SIZE);
			if (pool[i].head.f->next == NULL)
				fprintf(stderr, "allocating memory\n");
			pool[i].head.f = pool[i].head.f->next;
		}
		pool[i].head.f->next = NULL;
		pool[i].head.f = new_heap_list;
	}
	pool[OBJ_SET].head.f = malloc(OBJ_SIZE);
	if (pool[OBJ_SET].head.f == NULL)
		fprintf(stderr, "allocating memory\n");
	pool[OBJ_SET].head.f->next = NULL;
	new_heap_list = pool[OBJ_SET].head.f;
	j = 127;
	pool[OBJ_SET].free_size = (unsigned int)j + 1;
	while (j--)
	{
		pool[OBJ_SET].head.f->next = malloc(OBJ_SIZE);
		if (pool[OBJ_SET].head.f->next == NULL)
			fprintf(stderr, "allocating memory\n");
		pool[OBJ_SET].head.f = pool[OBJ_SET].head.f->next;
	}
	pool[OBJ_SET].head.f->next = NULL;
	pool[OBJ_SET].head.f = new_heap_list;
}

void remove_object(objectp name)
{
	object_pairp p, prev, next;
	prev = NULL;

	for (p = setobjs_list; p != NULL; prev = p, p = next)
	{
		next = p->next;
		if (name->type == OBJ_IDENTIFIER &&
			!strcmp(name->value.id, p->name->value.id))
		{
			if (prev == NULL)
				setobjs_list = next;
			else
				prev->next = next;
			free(p);
			break;
		}
	}
}

void set_object(objectp name, objectp value)
{
	object_pairp p, next;

	if (value == NULL || value == null)
	{
		fprintf(stderr, "; SET OBJECT: NULL VALUE.");
		longjmp(je, 1);
	}
	if (name->type != OBJ_IDENTIFIER || !strcmp(name->value.id, "null"))
	{
		fprintf(stderr, "; SET OBJECT: NOT IDENTIFIER NAME.");
		longjmp(je, 1);
	}

	for (p = setobjs_list; p != NULL; p = next)
	{
		next = p->next;
		if (name->type == OBJ_IDENTIFIER &&
			!strcmp(name->value.id, p->name->value.id))
		{
			p->value = value;
			return;
		}
	}
	p = (object_pairp)malloc(sizeof(struct object_pair));
	if (p == NULL)
	{
		fprintf(stderr, "allocating memory\n");
		return;
	}
	p->next = setobjs_list;
	setobjs_list = p;
	p->name = name;
	p->value = value;
}

objectp
try_object(const struct object *name)
{
	object_pairp p;

	if (name == null)
		return null;
	for (p = setobjs_list; p != NULL; p = p->next)
		if (!strcmp(name->value.id, p->name->value.id))
			return p->value;
	return null;
}

objectp
get_object(const struct object *name)
{
	object_pairp p;
	if (name == NULL)
	{
		fprintf(stderr, "; GET OBJECT: NULL NAME.");
		longjmp(je, 1);
	}
	for (p = setobjs_list; p != NULL; p = p->next)
		if (!strcmp(name->value.id, p->name->value.id))
			return p->value;

	fprintf(stderr, "; OBJECT '%s' NOT FOUND.", name->value.id);
	longjmp(je, 1);
	return null;
}

void clean_objects(void)
{
	object_pairp p;
	while ((p = setobjs_list) != NULL)
	{
		setobjs_list = setobjs_list->next;
		free(p);
	}
	setobjs_list = NULL;
}

void dump_object(int pool_number)
{
	objectp q;
	object_pairp p;
	int i;

	if (pool_number == 0)
	{
		for (p = setobjs_list; p != NULL; p = p->next)
		{
			princ_object(stdout, p->name);
			printf(": ");
			princ_object(stdout, p->value);
			printf("\n");
		}
		printf("| USED\t  FREE|\n");
		for (i = 0; i <= 8; i++)
		{
			if (pool[i].used_size > 0)
				printf("|%6zu %6zu|\n", pool[i].used_size, pool[i].free_size);
		}
	}
	else if (pool_number >= 3 && pool_number <= 8)
	{

		for (q = pool[pool_number].head.u; q != NULL; q = q->next)
		{
			princ_object(stdout, q);
			printf(" ");
		}
		printf("\n");
		printf("| USED\t  FREE|\n");
		printf("|%6zu %6zu|\n", pool[pool_number].used_size, pool[pool_number].free_size);
	}
	else
	{
		fprintf(stderr, "; NO SUCH POOL %d", pool_number);
	}
}
#ifdef GCTHREADS
static void *tt(void *tree)
{
	objectp *t_ptr = (objectp *)tree;
	if ((*t_ptr)->gc == gc_id)
		return NULL;
	(*t_ptr)->gc = gc_id;
	if ((*t_ptr)->type == OBJ_CONS || (*t_ptr)->type == OBJ_SET)
	{
		tt(&((*t_ptr)->vcar));
		tt(&((*t_ptr)->vcdr));
	}
	return NULL;
}
__inline__ static void
tag_whole_tree(void)
{
	object_pairp p;
	pthread_t tag_name, tag_value;
	for (p = setobjs_list; p != NULL; p = p->next)
	{
		pthread_create(&tag_name, NULL, tt, &(p->name));
		pthread_create(&tag_value, NULL, tt, &(p->value));
		pthread_join(tag_name, NULL);
		pthread_join(tag_value, NULL);
	}
	return;
}
#else
static void tag_tree(objectp p)
{
	if (p->gc == gc_id)
		return;
	p->gc = gc_id;
	if (p->type == OBJ_CONS)
	{
		tag_tree(p->vcar);
		tag_tree(p->vcdr);
	}
}

static void tag_whole_tree(void)
{
	object_pairp p;

	for (p = setobjs_list; p != NULL; p = p->next)
	{
		tag_tree(p->name);
		tag_tree(p->value);
	}
}

#endif
void garbage_collect(void)
{
	objectp p, prev, next;
	objectp new_used_objs_list;
	a_type i;

	if (++gc_id == UINT_MAX - 1)
		gc_id = 1;

	tag_whole_tree();

	for (i = 3; i <= 8; i++)
	{
		prev = NULL;
		new_used_objs_list = NULL;
		for (p = pool[i].head.u; p != NULL; p = next)
		{
			next = p->next;
			prev = p;
			if (p->gc != gc_id && p != null)
			{
				// if (prev == NULL) {
				// 	printf("not redundant\n");
				// 	pool[i].head.u = next;
				// } else
				prev->next = next;
				p->next = pool[i].head.f;
				pool[i].head.f = p;
				pool[i].free_size++;
				pool[i].used_size--;
			}
			else
			{
				p->next = new_used_objs_list;
				new_used_objs_list = p;
			}
		}
		pool[i].head.u = new_used_objs_list;
		recycle_pool(i);
	}
}

