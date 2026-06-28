#include "./state.h"

// used string comparison for symbols to support multiple character symbols
// like +=, >=, <=, etc.

static const struct
{
  const char *sym;
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
  {NULL, -1} /* end indicator */
};

bool lex_symbol(sleek_tok *dest) {
  sleek_tok tok = INVALID_TOKEN;
  int idx;
  for (idx = 0; symbols[idx].sym != NULL; idx++)
  {
    const char *sm = symbols[idx].sym;
    if (strncmp(state.source, sm, strlen(sm)) == 0)
    {
      tok.type = sleek_tok_type_symbol;
      tok.data.symbol.type = symbols[idx].type;
      break;
    }
  }
  if (tok.type == sleek_tok_type_invalid)
  return false;
  sleek_report("lex_symbol", "parsed symbol: `%.*s` (type = %d)", (int)strlen(symbols[idx].sym), state.source, tok.data.symbol.type);
  move_source(state.source + strlen(symbols[idx].sym));
  *dest = tok;
  return true;
}