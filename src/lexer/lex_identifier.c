#include "./state.h"

bool lex_identifier(sleek_tok *dest)
{
  sleek_tok tok = INVALID_TOKEN;
  const char *end = state.source;
  while (isalnum(*end))
    end++;
  size_t len = end - state.source;
  if (state.source == end) return false;
  tok.type = sleek_tok_type_identifier;
  tok.data.identifier.ptr = state.source;
  tok.data.identifier.length = len;
  *dest = tok;
  sleek_report("lex_identifier", "parsed identifier: %.*s(length = %zu)", (int)len, state.source, len);
  move_source(end);
  return true;
}