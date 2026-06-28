#include "./state.h"

// keywords list
static const struct {
  const char *kw;
  enum sleek_keyword_types type;
}
keywords[] = {
  {"fn", sleek_keyword_type_fn},
  {NULL, -1}
};

bool lex_keyword(sleek_tok *dest)
{
  sleek_tok tok = INVALID_TOKEN;
  const char *end = state.source;
  while (isalnum(*end))
    end++;
  size_t len = end - state.source;
  for (int i = 0; keywords[i].kw != NULL; i++)
  {
    const char *kw = keywords[i].kw;
    if (strlen(kw) == len && strncmp(state.source, kw, strlen(kw)) == 0)
    {
      tok.type = sleek_tok_type_keyword;
      tok.data.keyword.type = keywords[i].type;
      break;
    }
  }
  if (tok.type == sleek_tok_type_invalid)
    return false;
  
  *dest = tok;
  sleek_report("lex_keyword", "parsed keyword: %.*s(type = %d)", (int)len, state.source, tok.data.keyword.type);
  move_source(end);
  return true;
}