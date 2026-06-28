#include "./state.h"

static const struct
{
  const char *name;
  enum sleek_symbol_types type;
}
symbols[] =
{
  {"(", sleek_symbol_type_parantheses_l},
  {")", sleek_symbol_type_parantheses_r},
  {"{", sleek_symbol_type_braces_l},
  {"}", sleek_symbol_type_braces_r},
  {",", sleek_symbol_type_comma},
  {";", sleek_symbol_type_semicolon},
  {"\"", sleek_symbol_type_dblquote},
  {NULL, -1}
};

bool lex_symbol(sleek_tok *dest) {
  sleek_tok tok = INVALID_TOKEN;
  int idx;
  for (idx = 0; symbols[idx].name != NULL; idx++)
  {
    const char *sm = symbols[idx].name;
    if (strncmp(state.source, sm, strlen(sm)) == 0)
    {
      tok.type = sleek_tok_type_symbol;
      tok.data.symbol.type = symbols[idx].type;
      move_source(state.source + strlen(sm));
      break;
    }
  }
  if (tok.type == sleek_tok_type_invalid)
    return false;
  *dest = tok;
  sleek_report("lex_symbol", "parsed symbol: %.*s(type = %d)", (int)strlen(symbols[idx].name), state.source, tok.data.symbol.type);
  return true;
}