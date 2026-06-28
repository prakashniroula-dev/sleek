#include "./state.h"
#include <defs.h>
#include <lexer.h>

// Main lexer
sleek_tok lex_token() {
  sleek_tok tok = INVALID_TOKEN;
  while (isspace(state.current_char)) {
    advance_char();
  }
  if (state.current_char == '\0') {
    return EOF_TOKEN;
  }
  if (isdigit(state.current_char)) {
    if (lex_number(&tok)) return tok;
  }
  if (isalpha(state.current_char)) {
    if (lex_keyword(&tok)) return tok;
    if (lex_identifier(&tok)) return tok;
  }
  if (state.current_char == '"') {
    if (lex_string(&tok)) return tok;
  }
  if (lex_symbol(&tok)) return tok;
  sleek_warn("lex_token", "unrecognized character '%c'", state.current_char);
  advance_char();
  return INVALID_TOKEN;
}

// Other redirects to internal state functions for the lexer API

void sleek_lex_init(const char* src, const char **errdest)
{
  state_init(src, errdest);
}

sleek_tok sleek_peek_token(int idx)
{
  sleek_tok tok = {0};
  peek_token(&tok, idx);
  return tok;
}

void sleek_advance_token(void)
{
  advance_token();
}

void sleek_advance_token_by(int count)
{
  advance_token_by(count);
}

void sleek_lex_reset()
{
  state_reset();
}