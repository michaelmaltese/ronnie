#include "caos.h"
#include "caos_private.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <list>

#define ERROR(type,token) caos_set_error(context, type, token)

CaosScript*
caos_script_from_array (CaosValue *tokens)
{
  CaosScript *s = (CaosScript*) malloc (sizeof (*s));
  s->tokens = tokens;
  return s;
}

CaosRuntime*
caos_runtime_new() {
  CaosRuntime *runtime = new CaosRuntime();

  return runtime;
}

void
caos_runtime_destroy (CaosRuntime *rt) {
  delete rt;
}

void
caos_register_function (
  CaosRuntime *runtime,
  char *label,
  caos_command_t command,
  caos_expression_t expression
) {
  FunctionRef f = { command, expression };
  // TODO: Error if label already registered
  runtime->functions [label] = f;
}

CaosContext*
caos_context_new (CaosRuntime *runtime, CaosScript *script) {
  CaosScriptHandle handle = { script, script->tokens };

  CaosContext *context = (CaosContext*) malloc (sizeof (*context)); {
    context->runtime = runtime;
    context->error = NULL;
    context->script_handle = handle;
    context->user_data = NULL;
    context->stack = new std::stack <int>();
  }
  return context;
}

void
caos_context_destroy (CaosContext *c) {
  delete c->stack;
  free (c);
}

/*void
caos_reset (CaosContext *context, void *script) {
  context->error = NULL;
  context->script = script;
  context->user_data = NULL;
  context->stack = new std::stack <int>();
}*/

int
caos_mark (CaosContext *context)
{
  return context->script_handle.position
       - context->script_handle.script->tokens;
}

void
caos_jump (CaosContext *context, int mark)
{
  context->script_handle.position =
    context->script_handle.script->tokens
  + mark;
}

void
caos_stack_push (CaosContext *context, int i)
{
  context->stack->push (i);
}

int
caos_stack_pop (CaosContext *context)
{
  int i = caos_stack_peek (context);
  context->stack->pop();
  return i;
}

int
caos_stack_peek (CaosContext *context)
{
  return context->stack->top();
}

CaosValue
caos_current_token (CaosContext *context)
{
  return *context->script_handle.position;
}

void
caos_set_error (CaosContext *context, CaosErrorType type, CaosValue token)
{
	if (!context->error) {
		context->error = (CaosError*) malloc (sizeof (CaosError));
		context->error->type = type;
		context->error->token = token;
	}
}

CaosError*
caos_get_error (CaosContext *context)
{
	return context->error;
}

void
caos_clear_error (CaosContext *context)
{
	free (context->error);
	context->error = NULL;
}

bool
caos_done (CaosContext *context)
{
  return context->error ||
         caos_value_is_eoi (*context->script_handle.position);
}

void caos_advance_to_next_symbol (CaosContext *context)
{
  CaosValue tok;
  do {
    caos_advance (context);
    tok = caos_current_token (context);
    if (caos_done (context)) return;
  } while (!caos_value_is_symbol (tok));
}

void
caos_fast_forward (CaosContext *context, ...)
{
  std::list <char*> strings;

  {
    va_list args;
    char *s;
    va_start (args, context);
    while ((s = va_arg (args, char*)))
      strings.push_back (s);
    va_end (args);
  }

  while (true) {
    CaosValue sym = caos_current_token(context);
	if (caos_value_is_eoi (sym)) {
	  ERROR (CAOS_FAILED_TO_FAST_FORWARD, sym);
      return;
    }
    char *str = caos_value_to_symbol (sym);

    std::list<char*>::iterator it, end;
    for (it = strings.begin(), end = strings.end(); it != end; ++it) {
      if (strcmp (str, *it) == 0) return;
    }
    caos_advance_to_next_symbol(context);
  }
}

/*

What if we define a CAOS script as a stream of 'symbols' and 'others'?
The runtime won't have to deal with 'others' at all

*/

CaosValue
caos_arg_value (CaosContext *context)
{
  CaosValue ret = caos_value_null();
  caos_expression_t expr = NULL;

  CaosValue token = caos_current_token (context);
  if (caos_done (context)) return ret;

  if (caos_value_is_symbol (token))
  {
      expr = caos_get_expression (context);
      if (expr)
        ret = expr (context);
      else {
        ERROR (CAOS_EXPECTED_EXPRESSION, token);
      }
  } else {
      ret = token;
      caos_advance (context);
  }

  return ret;
}

char*
caos_arg_symbol (CaosContext *context)
{
  CaosValue tok = caos_current_token (context);
  caos_advance (context);
  if (!caos_value_is_symbol (tok)) return NULL;
  return caos_value_to_symbol (tok);
}

FunctionRef
caos_get_function (CaosContext *context)
{
  static FunctionRef null_func = { NULL, NULL };
  char *label;

  label = caos_arg_symbol (context);
  if (!label) return null_func;
  //printf ("[DEBUG] Function '%s'\n", label);

  std::map<std::string, FunctionRef> &functions
    = context->runtime->functions;
  if (functions.count (label))
  {
    return functions[label];
  }

  return null_func;
}

caos_command_t
caos_get_command (CaosContext *context)
{
  //printf ("[DEBUG] Command\n");
  return caos_get_function (context).command;
}

caos_expression_t
caos_get_expression (CaosContext *context)
{
  return caos_get_function (context).expression;
}

void
caos_advance (CaosContext *context)
{
  if (!caos_done (context))
	  context->script_handle.position++;
}

void
caos_tick (CaosContext *context, void *user_data)
{
  context->user_data = user_data;
  {
	CaosValue sym = caos_current_token (context);
    caos_command_t command = caos_get_command (context);
    if (command) command (context);
    else {
		ERROR (CAOS_EXPECTED_COMMAND, sym);
	}
  }
  context->user_data = NULL;
}

void*
caos_user_data (CaosContext *context)
{
  return context->user_data;
}

/*
void*
caos_get_script (CaosContext *context)
{
  return context->script;
}
*/
