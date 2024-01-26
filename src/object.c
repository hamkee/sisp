#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>

#include "sisp.h"
#include "extern.h"
#include "misc.h"
#include "vmm.h"

#define HANDSIG(ARG, EXP)                   \
	do                                      \
	{                                       \
		if (ARG == NULL)                    \
		{                                   \
			fprintf(stderr, "; " #EXP "."); \
			longjmp(je, 1);                 \
		}                                   \
	} while (0)

objectp nil;
objectp t;
objectp null;

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
	if (type == OBJ_CONS)
		p->vcar = p->vcdr = nil;

	return p;
}

__inline__ objectp
search_object_rational(long int num, long int den)
{
	objectp p;

	for (p = pool[OBJ_RATIONAL].head.u; p != NULL; p = p->next)
		if (p->value.r.n == num && p->value.r.d == den)
			return p;

	return (objectp)NULL;
}

__inline__ objectp
search_object_integer(long int i)
{
	objectp p;

	for (p = pool[OBJ_INTEGER].head.u; p != NULL; p = p->next)
		if (p->value.i == i)
			return p;

	return (objectp)NULL;
}

__inline__ objectp
search_object_identifier(char *s)
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

	for (i = 3; i < 7; i++)
	{
		pool[i].head.u = NULL;
		pool[i].head.f = NULL;
		pool[i].free_size = 0;
		pool[i].used_size = 0;
	}
	for (i = 3; i <= 4; i++)
	{
		pool[i].head.f = malloc(OBJ_SIZE);
		pool[i].head.f->next = NULL;
		new_heap_list = pool[i].head.f;
		j = (i == 3 ? 63 : 511);
		pool[i].free_size = j + 1;
		while (j--)
		{
			pool[i].head.f->next = malloc(OBJ_SIZE);
			pool[i].head.f = pool[i].head.f->next;
		}
		pool[i].head.f->next = NULL;
		pool[i].head.f = new_heap_list;
	}
	null = new_object(OBJ_NULL);
	nil = new_object(OBJ_NIL);
	t = new_object(OBJ_T);
}

void remove_object(objectp name)
{
	object_pairp p, prev, next;
	prev = NULL;

	for (p = setobjs_list; p != NULL; prev = p, p = next)
	{
		next = p->next;
		if (name->type == OBJ_IDENTIFIER &&
			p->name->type == OBJ_IDENTIFIER &&
			!strcmp(name->value.id, p->name->value.id))
		{
			if (prev == NULL)
				setobjs_list = next;
			else
				prev->next = next;
			if (p->value->type == OBJ_IDENTIFIER)
				free(p->value->value.id);
			free(p);
			break;
		}
	}
}

void set_object(objectp name, objectp value)
{
	object_pairp p, next;

	HANDSIG(value, SET OBJECT);
	for (p = setobjs_list; p != NULL; p = next)
	{
		next = p->next;
		if (name->type == OBJ_IDENTIFIER &&
			p->name->type == OBJ_IDENTIFIER &&
			!strcmp(name->value.id, p->name->value.id))
		{
			p->value = value;
			return;
		}
	}
	p = (object_pairp)malloc(sizeof(struct object_pair));
	p->next = setobjs_list;
	setobjs_list = p;
	p->name = name;
	p->value = value;
}

__inline__ objectp
try_object(objectp name)
{
	object_pairp p;

	if (name == null)
		return null;
	for (p = setobjs_list; p != NULL; p = p->next)
		if (p->name->type == OBJ_IDENTIFIER &&
			!strcmp(name->value.id, p->name->value.id))
			return p->value;
	return null;
}

__inline__ objectp
get_object(objectp name)
{
	object_pairp p;

	HANDSIG(name, GETOBJECT);
	for (p = setobjs_list; p != NULL; p = p->next)
		if (p->name->type == OBJ_IDENTIFIER &&
			!strcmp(name->value.id, p->name->value.id))
			return p->value;

	HANDSIG(NULL, OBJECT NOT FOUND);
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

void dump_objects(void)
{
	objectp q;
	object_pairp p;
	int i;

	for (p = setobjs_list; p != NULL; p = p->next)
	{
		princ_object(stdout, p->name);
		printf(":");
		princ_object(stdout, p->value);
		printf("\n");
	}
	for (i = 3; i < 7; i++)
		printf("|%6zd %6zd|\n", pool[i].used_size, pool[i].free_size);
	printf("want to see a pool? a magic pool??\nTell me a number, a magic number!: ");
	scanf("%d", &i);
	if (i >= 3 && i < 7)
	{
		for (q = pool[i].head.u; q != NULL; q = q->next)
		{
			princ_object(stdout, q);
			printf(" --- ");
		}
		printf("\n");
	}
}

__inline__ static void
tag_tree(objectp p)
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

__inline__ static void
tag_whole_tree(void)
{
	object_pairp p;

	for (p = setobjs_list; p != NULL; p = p->next)
	{
		tag_tree(p->name);
		tag_tree(p->value);
	}
}

__inline__ void
garbage_collect(void)
{
	objectp p, prev, next;
	objectp new_used_objs_list = t;
	int i;

	if (++gc_id == UINT_MAX - 1)
		gc_id = 1;

	tag_whole_tree();

	for (i = 3; i < 7; i++)
	{
		prev = NULL;
		new_used_objs_list = NULL;
		for (p = pool[i].head.u; p != NULL; p = next)
		{
			next = p->next;
			prev = p;
			if (p->gc != gc_id && p != null)
			{
				if (prev == NULL)
				{
					printf("WOWOWOW!!!\n");
					pool[i].head.u = next;
				}
				else
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

/*
GARBAGE COLLECTOR WITH THREADS

static
void *tt(void *tree)
{
	objectp *t_ptr = (objectp *)tree;
	if((*t_ptr)->gc == gc_id)
		return NULL;
	(*t_ptr)->gc = gc_id;
	if((*t_ptr)->type == OBJ_CONS) {
		tt(&((*t_ptr)->vcar));
		tt(&((*t_ptr)->vcdr));
	}
}
__inline__ static
void *tag_tree(void *tree)
{
	objectp *t_ptr = (objectp *)tree;
	pthread_t tag_l,tag_r;
	if((*t_ptr)->gc == gc_id)
		return NULL;
	(*t_ptr)->gc = gc_id;
	if( (*t_ptr)->type == OBJ_CONS) {
		pthread_create(&tag_l, NULL, tt, &((*t_ptr)->vcar));
		pthread_create(&tag_r, NULL, tt, &((*t_ptr)->vcdr));
		pthread_join(tag_l, NULL);
		pthread_join(tag_r, NULL);
	}
}
__inline__ static void
tag_whole_tree(void)
{
	object_pairp p;
	pthread_t tag_name, tag_value;
	for (p = setobjs_list; p != NULL; p = p->next) {
		pthread_create(&tag_name, NULL, tt, &(p->name));
		pthread_create(&tag_value, NULL, tt, &(p->value));
		pthread_join(tag_name, NULL);
		pthread_join(tag_value, NULL);
	}

}
*/