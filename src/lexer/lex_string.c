#include "./state.h"

bool lex_string(sleek_tok *dest) {
  sleek_report("lex_string", "parsing from '%c'(idx = %zu)", state.current_char, state.source_len - state.remaining);
  sleek_tok tok = INVALID_TOKEN;
  if (state.current_char != '"') return false;
  advance_char(); // skip opening quote
  const char *end = state.source;
  char prev_char = '\0';
  while (*end != '\0') {
    if (*end == '"' && prev_char != '\\') break;
    prev_char = *end;
    end++;
  }
  if (*end != '"') return false; // Unterminated string
  size_t len = end - state.source;
  
  tok.type = sleek_tok_type_literal;
  tok.data.literal.type = sleek_args_type_string;
  tok.data.literal.data._string.ptr = state.source;
  tok.data.literal.data._string.length = len;
  
  *dest = tok;
  sleek_report("lex_string", "parsed string: \"%.*s\"(length = %zu)", (int)len, state.source, len);
  move_source(end + 1); // skip closing quote
  return true;
}