#include <defs.h>
#include <log.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

#define tok_next_max_len 5
#define tok_prev_max_len 5

static sleek_tok tok_next_buffer[tok_next_max_len];
static sleek_tok tok_current;
static sleek_tok tok_prev_buffer[tok_prev_max_len];

// Next buffer (lookahead) – ring buffer with both read and write indices
static size_t tok_next_buf_len = 0;
static size_t tok_next_write_idx = 0;
static size_t tok_next_read_idx = 0; // NEW

// Previous buffer (history) – ring buffer, write only, read by negative offset
static size_t tok_prev_buf_len = 0;
static size_t tok_prev_write_idx = 0;

bool tok_current_valid = false;

static const char *source = NULL;
static char current_char = '\0';
static size_t source_len = 0;
static const char **errptr = NULL;

char sleek_peek_char(int idx)
{
  if (source == NULL)
  {
    sleek_error("error: no source provided to peek char from");
    return '\0';
  }
  if (idx < 0)
  {
    sleek_error("peeking char at negative index: %d", idx);
    return '\0';
  }
  else if (idx > source_len)
  {
    sleek_warn("peeking char beyond length: %d", idx);
    return '\0';
  }
  char c = source[idx];
  sleek_report("peeking char at index: %d", idx);
  sleek_report_continue(1, "|- char: %c", c);
  return c;
}

static void sleek_next_char()
{
  if (source_len <= 0)
  {
    sleek_warn("next char called when source length is 0");
    return;
  }
  source++;
  source_len--;
  current_char = *source;
}

static void sleek_move_source(const char *newptr)
{
  size_t diff = newptr - source;
  source = newptr;
  source_len = source_len - diff - 1;
  current_char = *source;
}

void sleek_lex_init(const char *src, const char **errdest)
{
  sleek_report("sleek_lex_init called");
  sleek_report_continue(1, "|- src = %.10s...\n");
  sleek_report_continue(1, "|- errdest = %s", errdest == NULL ? "null" : "(address)");
  source = src;
  current_char = *source;
  errptr = errdest;
  source_len = strlen(source);
}

bool sleek_lex_number(sleek_tok *dest)
{
  sleek_tok tok = sleek_tok_invalid();
  char *endptr = NULL;
  int64_t data = strtoull(source, &endptr, 10);
  if (endptr == NULL || endptr == source)
    return false;
  sleek_move_source(endptr);
  tok.type = sleek_tok_type_literal;
  tok.data.literal.type = sleek_args_type_int64;
  tok.data.literal.data._int64 = data;
  *dest = tok;
  return true;
}

bool sleek_lex_keyword(sleek_tok *dest)
{
  // keywords list
  static const struct
  {
    const char *name;
    enum sleek_keyword_types type;
  } keywords[] = {
      {"fn", sleek_keyword_type_fn},
      {NULL, -1}};

  sleek_tok tok = sleek_tok_invalid();
  const char *end = source;
  while (isalnum(*end))
    end++;
  size_t len = end - source;

  for (int i = 0; keywords[i].name != NULL; i++)
  {
    const char *kw = keywords[i].name;
    if (strlen(kw) == len && strncmp(source, kw, strlen(kw)) == 0)
    {
      tok.type = sleek_tok_type_keyword;
      tok.data.keyword.type = keywords[i].type;
    }
  }

  if (tok.type == sleek_tok_type_invalid) return false;

  *dest = tok;
  sleek_move_source(end);
  return true;
}

bool sleek_lex_identifier(sleek_tok *dest)
{
  sleek_tok tok = sleek_tok_invalid();
  const char *end = source;
  while (isalnum(*end))
    end++;
  size_t len = end - source;
  if (source == end) return false;
  
  tok.type = sleek_tok_type_identifier;
  tok.data.identifier.ptr = source;
  tok.data.identifier.length = len;
  *dest = tok;
  sleek_move_source(end);
  return true;
}

bool sleek_lex_string(sleek_tok *dest)
{
  sleek_tok tok = sleek_tok_invalid();
  if (current_char != '"') return false;
  sleek_next_char(); // skip opening quote
  const char *end = source;
  char prev_char = '\0';
  while (*end != '"' && prev_char != '\\' && *end != '\0') {
    prev_char = *end;
    end++;
  }
  if (*end != '"') return false; // Unterminated string
  size_t len = end - source;
  
  tok.type = sleek_tok_type_literal;
  tok.data.literal.type = sleek_args_type_string;
  tok.data.literal.data._string.ptr = source;
  tok.data.literal.data._string.length = len;
  
  *dest = tok;
  sleek_move_source(end + 1); // skip closing quote
  return true;
}

bool sleek_lex_symbol(sleek_tok *dest)
{
  sleek_tok tok = sleek_tok_invalid();
  // symbols list
  static const struct
  {
    const char *name;
    enum sleek_symbol_types type;
  } symbols[] = {
      {"(", sleek_symbol_type_parantheses_l},
      {")", sleek_symbol_type_parantheses_r},
      {"{", sleek_symbol_type_braces_l},
      {"}", sleek_symbol_type_braces_r},
      {",", sleek_symbol_type_comma},
      {";", sleek_symbol_type_semicolon},
      {"\"", sleek_symbol_type_dblquote},
      {NULL, -1}};
  for (int i = 0; symbols[i].name != NULL; i++)
  {
    const char *sm = symbols[i].name;
    if (strncmp(source, sm, strlen(sm)) == 0)
    {
      tok.type = sleek_tok_type_symbol;
      tok.data.symbol.type = symbols[i].type;
      sleek_move_source(source + strlen(sm));
      break;
    }
  }
  if (tok.type == sleek_tok_type_invalid)
    return false;
  *dest = tok;
  return true;
}

sleek_tok sleek_lex_token()
{
  sleek_tok tok = sleek_tok_invalid();
  while (isspace(current_char))
    sleek_next_char();
  if (current_char == '\0')
  {
    tok.type = sleek_tok_type_eof;
    return tok;
  }
  if (isdigit(current_char))
  {
    sleek_lex_number(&tok);
    return tok;
  }
  if (isalpha(current_char))
  {
    if (sleek_lex_keyword(&tok))
      return tok;
    sleek_lex_identifier(&tok);
    return tok;
  }
  if (current_char == '"')
  {
    if (sleek_lex_string(&tok)) return tok;
  }
  sleek_lex_symbol(&tok);
  return tok;
}
static void sleek_insert_next_tok(sleek_tok tok)
{
  if (tok_next_buf_len >= tok_next_max_len)
  {
    sleek_error("next token buffer overflow");
    return;
  }
  tok_next_buffer[tok_next_write_idx] = tok;
  tok_next_write_idx = (tok_next_write_idx + 1) % tok_next_max_len;
  tok_next_buf_len++;
}

static void sleek_insert_prev_tok(sleek_tok tok)
{
  tok_prev_buffer[tok_prev_write_idx] = tok;
  tok_prev_write_idx = (tok_prev_write_idx + 1) % tok_prev_max_len;
  if (tok_prev_buf_len < tok_prev_max_len)
  {
    tok_prev_buf_len++;
  }
}

// Optional – fixed helpers (can be inlined in sleek_peek_token)
static void sleek_peek_next_tok(int idx, sleek_tok *dest)
{
  if (idx < 1 || idx > (int)tok_next_buf_len)
  {
    *dest = sleek_tok_invalid();
    return;
  }
  int index = (tok_next_read_idx + idx - 1) % tok_next_max_len;
  *dest = tok_next_buffer[index];
}

static void sleek_peek_prev_tok(int idx, sleek_tok *dest)
{
  if (idx >= 0)
  {
    *dest = sleek_tok_invalid();
    return;
  }
  int back = -idx;
  if ((int)tok_prev_buf_len < back)
  {
    *dest = sleek_tok_invalid();
    return;
  }
  int index = (tok_prev_write_idx - back + tok_prev_max_len) % tok_prev_max_len;
  *dest = tok_prev_buffer[index];
}

sleek_tok sleek_peek_token(int idx)
{
  if (idx == 0)
  {
    if (!tok_current_valid)
    {
      tok_current = sleek_lex_token();
      tok_current_valid = true;
    }
    return tok_current;
  }
  else if (idx > 0)
  {
    if (idx > tok_next_max_len)
    {
      sleek_error("peek ahead index %d exceeds buffer size %d", idx, tok_next_max_len);
      return sleek_tok_invalid();
    }
    // Fill next buffer until we have at least `idx` tokens
    while (tok_next_buf_len < (size_t)idx)
    {
      sleek_tok tok = sleek_lex_token();
      sleek_insert_next_tok(tok);
      if (tok.type == sleek_tok_type_eof)
      {
        // Pad remaining slots with EOF so the requested index is available
        while (tok_next_buf_len < (size_t)idx)
        {
          sleek_insert_next_tok(tok);
        }
        break;
      }
    }
    int index = (tok_next_read_idx + idx - 1) % tok_next_max_len;
    return tok_next_buffer[index];
  }
  else
  { // idx < 0
    int back = -idx;
    if (back > tok_prev_max_len)
    {
      sleek_error("peek behind index %d exceeds buffer size %d", idx, tok_prev_max_len);
      return sleek_tok_invalid();
    }
    if ((int)tok_prev_buf_len < back)
    {
      sleek_warn("not enough history to peek %d tokens back", back);
      return sleek_tok_invalid();
    }
    int index = (tok_prev_write_idx - back + tok_prev_max_len) % tok_prev_max_len;
    return tok_prev_buffer[index];
  }
}


void sleek_advance_token(void)
{
  if (!tok_current_valid)
  {
    // No current token – lex the first one (safety)
    tok_current = sleek_lex_token();
    tok_current_valid = true;
    return;
  }

  // Save the token we are leaving behind
  sleek_insert_prev_tok(tok_current);

  if (tok_next_buf_len > 0)
  {
    // Take the next token from the lookahead buffer
    tok_current = tok_next_buffer[tok_next_read_idx];
    tok_next_read_idx = (tok_next_read_idx + 1) % tok_next_max_len;
    tok_next_buf_len--;
  }
  else
  {
    // No lookahead available – lex directly
    tok_current = sleek_lex_token();
  }
  tok_current_valid = true;
}

void sleek_advance_token_by(int count)
{
  if (count < 0)
  {
    sleek_error("cannot advance token by negative count: %d", count);
    return;
  }
  for (int i = 0; i < count; i++)
  {
    sleek_advance_token();
  }
}

void sleek_lex_reset()
{
  source = NULL;
  current_char = '\0';
  source_len = 0;
  errptr = NULL;
  tok_current_valid = false;
  tok_next_buf_len = 0;
  tok_next_write_idx = 0;
  tok_next_read_idx = 0;
  tok_prev_buf_len = 0;
  tok_prev_write_idx = 0;
}