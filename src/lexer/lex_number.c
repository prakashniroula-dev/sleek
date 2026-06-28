#include "./state.h"

// TO - DO :
// 1. lex_float() first to account for decimal points if exists
// 2. lex_number() for integer literals with support for negative numbers
// 3. lex_number() with different bases (binary, octal, decimal, hexadecimal)

bool lex_number(sleek_tok *dest) {
  sleek_report("lex_number", "parsing from '%c'(idx = %zu)", state.current_char, state.source_len - state.remaining);
  sleek_tok tok = INVALID_TOKEN;
  char *endptr = NULL;
  int64_t data = strtoull(state.source, &endptr, 10);
  if (endptr == NULL || endptr == state.source) {
    return false;
  }
  tok.type = sleek_tok_type_literal;
  tok.data.literal.type = sleek_args_type_int64;
  tok.data.literal.data._int64 = data;
  *dest = tok;
  sleek_report("lex_number", "parsed = %lld", data);
  move_source(endptr);
  return true;
}