#include <stdio.h>
#include <stdlib.h>

#include "sisp.h"
#include "extern.h"
#include "vmm.h"

void clean_pools(void)
{
	objectp p;
	int i;

	for (i = 0; i < NUM_POOLS; i++)
	{
		while ((p = pool[i].head.u) != NULL)
		{
			pool[i].head.u = pool[i].head.u->next;
			if (p->type == OBJ_IDENTIFIER)
				free(p->value.id);
			else if (p->type == OBJ_STRING)
				free(p->value.s.str);
			free(p);
		}
		while ((p = pool[i].head.f) != NULL)
		{
			pool[i].head.f = pool[i].head.f->next;
			free(p);
		}
		pool[i].head.f = NULL;
		pool[i].head.u = NULL;
	}
}

void recycle_pool(a_type type)
{
	size_t chunk_size;
	objectp p;

	if (type == OBJ_IDENTIFIER)
		chunk_size = 64;
	else if (type == OBJ_CONS)
		chunk_size = 128;
	else
		chunk_size = 32;
	if (pool[type].head.f == NULL)
		return;
	while (pool[type].free_size > chunk_size)
	{
		p = pool[type].head.f;
		pool[type].head.f = pool[type].head.f->next;
		if (p->type == OBJ_IDENTIFIER)
			free(p->value.id);
		else if (p->type == OBJ_STRING)
			free(p->value.s.str);
		free(p);
		pool[type].free_size--;
	}
	return;
}

void feed_pool(a_type type)
{
	size_t units;
	objectp new_heap_list;

	if (type == OBJ_IDENTIFIER)
		units = 31;
	else if (type == OBJ_CONS)
		units = 255;
	else
		units = 15;
	pool[type].free_size = units + 1;
	pool[type].head.f = malloc(OBJ_SIZE);
	if (pool[type].head.f == NULL)
	{
		fprintf(stderr, "allocating memory\n");
		return;
	}
	pool[type].head.f->next = NULL;
	new_heap_list = pool[type].head.f;
	while (units--)
	{
		pool[type].head.f->next = malloc(OBJ_SIZE);
		if (pool[type].head.f->next == NULL)
		{
			fprintf(stderr, "allocating memory\n");
			return;
		}
		pool[type].head.f = pool[type].head.f->next;
	}
	pool[type].head.f->next = NULL;
	pool[type].head.f = new_heap_list;
}

objectp
oballoc(a_type obj_type)
{
	objectp p;
	if (obj_type <= 2)
		return malloc(OBJ_SIZE);
	if (pool[obj_type].free_size == 0)
		feed_pool(obj_type);
	p = pool[obj_type].head.f;
	pool[obj_type].head.f = pool[obj_type].head.f->next;
	pool[obj_type].free_size--;

	return p;
}
