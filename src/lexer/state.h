#if !defined(sleek__internal_lexerstate_h)
#define sleek__internal_lexerstate_h

#include <defs.h>
#include <stdbool.h>
#include <log.h>
#include <ctype.h>

#if !defined(lexer_buffer_max)
#define lexer_buffer_max 5
#endif // lexer_buffer_max

sleek_tok lex_token();

typedef struct lexer_state {
  const char *source;
  size_t remaining;
  size_t source_len;

  char current_char;
  const char **errptr;

  sleek_tok cur_tok;
  bool cur_tok_valid;

  struct {
    sleek_tok buffer[lexer_buffer_max];
    size_t len;
    size_t read_idx;
    size_t write_idx;
  } tok_next;

  struct {
    sleek_tok buffer[lexer_buffer_max];
    size_t len;
    size_t write_idx;
  } tok_prev;
  
} lexer_state;

#define INVALID_TOKEN ((sleek_tok){.type = sleek_tok_type_invalid, .data._value = 0})
#define EOF_TOKEN ((sleek_tok){.type = sleek_tok_type_eof, .data._value = 0})

extern lexer_state state;

void set_source(const char *src, size_t len);
void set_errptr(const char **errdest);
void move_source(const char *newptr);
char peek_char(int idx);
void advance_char();
void insert_next_token(sleek_tok tok);
void insert_prev_token(sleek_tok tok);
void peek_token(sleek_tok *dest, int idx);
void advance_token();
void advance_token_by(int count);
void state_reset();
void state_init(const char *src, const char **errdest);

bool lex_keyword(sleek_tok *dest);
bool lex_number(sleek_tok *dest);
bool lex_string(sleek_tok *dest);
bool lex_identifier(sleek_tok *dest);
bool lex_symbol(sleek_tok *dest);

#endif // sleek__internal_lexerstate_h
