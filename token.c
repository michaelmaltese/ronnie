/*
   Structs
*/

#include "token.h"

/*
   Constructors
*/


CaosToken token_int_new (int i)
{
  CaosToken t = { TOKEN_INTEGER, .value.i = i };
  return t;
}

CaosToken token_string_new (char *s)
{
  CaosToken t = { TOKEN_STRING, .value.s = s };
  return t;
}

CaosToken token_symbol_new (char *s)
{
  CaosToken t = { TOKEN_SYMBOL, .value.s = s };
  return t;
}

CaosToken token_eoi()
{
  CaosToken t = { TOKEN_EOI };
  return t;
}

CaosToken token_null()
{
  CaosToken t;
  return t;
}

/*
   Type-checking
*/

TokenType token_get_type (CaosToken t)
{
  return t.type;
}

/*
   Conversions
*/

char* token_as_string (CaosToken t) {
  return t.value.s;
}

int token_as_int (CaosToken t) {
  return t.value.i;
}

char* token_as_symbol (CaosToken t) {
  return t.value.s;
}

/*CaosValue token_as_value (CaosToken t) {
  switch (token_get_type (t)) {
  case TOKEN_INTEGER:  return value_new_integer (token_as_integer (t));
  case TOKEN_STRING:   return value_new_string (token_as_string (t));
  case TOKEN_FUNCTION: return expression_call (token_as_function (t));
  default:
    assert (NULL && "Not a valid token!");
  }
}*/
