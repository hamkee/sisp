#ifndef SISP_H
#define IDENTIFIER  7
#define INTEGER     8
#define RATIONAL    9
#define OBJ_SIZE		sizeof(struct object)

typedef enum	{false, true} bool;
typedef enum 	{OBJ_NULL, OBJ_NIL, OBJ_T, OBJ_IDENTIFIER,
				      OBJ_CONS, OBJ_INTEGER, OBJ_RATIONAL} a_type;

typedef struct 	object *objectp;

struct object {
  a_type type;
  unsigned int gc; /* unsigned? */
  union {
    char *id;
    long int i;
    struct {
      long int n;
      long int d;
    } r;
    struct {
      objectp car;
      objectp cdr;
    } c;
  } value;
  objectp next;
};

typedef struct object_pair *object_pairp;

struct object_pair {
	objectp name;
	objectp value;
	object_pairp next;
};

typedef struct {
	char *name;
	objectp (*func)(objectp args);
} funcs;

#define vcar	value.c.car
#define vcdr	value.c.cdr

#define SISP_H

#endif
