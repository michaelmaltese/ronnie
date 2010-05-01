#ifndef CAOS_H
#define CAOS_H

#include "common.h"
#include "token.h"
#include "type.h"
#include "value.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CaosRuntime CaosRuntime;
typedef struct CaosContext CaosContext;

typedef void (*caos_command_t) (CaosContext*);
typedef CaosValue (*caos_expression_t) (CaosContext*);

typedef void (*caos_script_advance_t) (void*);
typedef CaosToken (*caos_script_get_t) (void*);
typedef void (*caos_script_jump_t) (void*, int);
typedef int (*caos_script_mark_t) (void*);

struct ICaosScript {
  caos_script_advance_t advance;
  caos_script_get_t get;
  caos_script_jump_t jump;
  caos_script_mark_t mark;
};

RONNIE_PUBLIC CaosRuntime* caos_runtime_new();

RONNIE_PUBLIC void caos_register_function
  (CaosRuntime*, char*, caos_command_t, caos_expression_t);
RONNIE_PUBLIC void caos_register_binomial_function
  (CaosRuntime*, char*, char*, caos_command_t, caos_expression_t);      

RONNIE_PUBLIC CaosContext* caos_context_new(CaosRuntime*);

RONNIE_PUBLIC void caos_advance (CaosContext*);
RONNIE_PUBLIC CaosToken caos_fast_forward (CaosContext*, ...);

RONNIE_PUBLIC void caos_stack_push (CaosContext*, int);
RONNIE_PUBLIC int caos_stack_pop (CaosContext*);
RONNIE_PUBLIC int caos_stack_peek (CaosContext*);

RONNIE_PUBLIC int caos_mark (CaosContext*);
RONNIE_PUBLIC void caos_jump (CaosContext*, int);

RONNIE_PUBLIC bool caos_done (CaosContext*);

RONNIE_PUBLIC char* caos_arg_symbol (CaosContext*);
RONNIE_PUBLIC CaosValue caos_arg_value (CaosContext*);
RONNIE_PUBLIC int caos_arg_int (CaosContext*);
RONNIE_PUBLIC char* caos_arg_string (CaosContext*);
RONNIE_PUBLIC bool caos_arg_bool (CaosContext*);

RONNIE_PUBLIC char* caos_get_error (CaosContext*);
RONNIE_PUBLIC void caos_set_error (CaosContext*, char*);
RONNIE_PUBLIC CaosToken caos_get_token (CaosContext*);

RONNIE_PUBLIC CaosRuntime* caos_get_runtime (CaosContext*);

RONNIE_PUBLIC void caos_set_script (CaosContext*, void*, struct ICaosScript);

RONNIE_PUBLIC void caos_tick (CaosContext*, void *user_data);
RONNIE_PUBLIC void* caos_user_data (CaosContext*);

#ifdef __cplusplus
}
#endif

#endif // CAOS_H
