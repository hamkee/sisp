#ifndef _EXTERN_H
#include <setjmp.h>
extern void clean_pools(void);
extern char *buffer;
extern jmp_buf jb;
extern jmp_buf je;
extern jmp_buf jl;
/* object.c */

extern objectp nil;
extern objectp t;
extern objectp null;
extern void init_objects(void);
extern objectp new_object(a_type);
extern objectp search_object_identifier(const char *);
extern objectp search_object_integer(const long int);
extern objectp search_object_string(const char *);
extern objectp search_object_rational(const long int, const long int);

extern void set_object(objectp, objectp);
extern objectp get_object(const struct object *);
extern void remove_object(objectp);
extern objectp try_object(const struct object *);
extern objectp handsig(const char *);
extern void garbage_collect(void);
extern void dump_object(int);
/* parser.c */
extern objectp parse_object(int);
/* lexer.c */
extern void process_input(const char *);
extern  void process_file(void);
extern FILE *input_file;
extern char *token_buffer;
extern void init_lex(void);
extern void done_lex(void);
extern int gettoken(void);
extern void clean_buffers(void);
extern void clean_objects(void);
/* funcs.c */
#define _EXTERN_H

#endif
