#include "./state.h"
#include <log.h>

// Initial lexer state
lexer_state state = {
  .source = NULL,
  .source_len = 0,
  .remaining = 0,
  .current_char = '\0',
  .errptr = NULL,
  .cur_tok = INVALID_TOKEN,
  .cur_tok_valid = false,
  .tok_next = {
    .buffer = {0},
    .len = 0,
    .read_idx = 0,
    .write_idx = 0
  },
  .tok_prev = {
    .buffer = {0},
    .len = 0,
    .write_idx = 0
  }
};

void set_source(const char *src, size_t len) {
  state.source = src;
  state.remaining = len;
  state.source_len = len;
  state.current_char = (len > 0) ? src[0] : '\0';
  sleek_report("set_source", "source set; length = %zu", len);
}

void move_source(const char *newptr) {
  if (state.source == NULL) {
    sleek_error("move_source", "source is NULL");
    return;
  }
  if ( newptr < state.source || newptr > state.source + state.remaining) {
    sleek_error("move_source", "new pointer is out of bounds");
    return;
  }
  set_source(newptr, strlen(newptr));
  sleek_report("move_source", "source moved; new length = %zu", state.remaining);
}

void set_errptr(const char **errdest) {
  state.errptr = errdest;
  sleek_report("set_errptr", "error pointer set");
}

char peek_char(int idx) {
  if (state.source == NULL) {
    sleek_error("peek_char", "bad state -> source is NULL");
    return '\0';
  } else if (idx < 0) {
    sleek_error("peek_char", "negative index (%d)", idx);
    return '\0';
  } else if ((size_t)idx >= state.remaining) {
    sleek_warn("peek_char", "index exceeds source length (%d >= %zu)", idx, state.remaining);
    return '\0';
  }
  sleek_report("peek_char", "'%c' (idx = %d)", state.source[idx], idx);
  return state.source[idx];
}

void advance_char() {
  if (state.source == NULL || state.remaining == 0) {
    sleek_warn("advance_char", "%s", (state.source == NULL) ? "source is NULL" : "0 remaining chars");
    state.current_char = '\0';
    return;
  }
  sleek_report("advance_char", "'%c' -> '%c' (idx = %zu)", state.current_char, state.source[1], state.source_len - state.remaining);
  state.source++;
  state.remaining--;
  state.current_char = *state.source;
}

void insert_next_token(sleek_tok tok) {
  if (state.tok_next.len >= lexer_buffer_max) {
    sleek_error("insert_next_token", "token buffer full");
    return;
  }
  if (!state.cur_tok_valid) {
    state.cur_tok = tok;
    state.cur_tok_valid = true;
    sleek_report("insert_next_token", "set cur_tok; (type = %d)", tok.type);
    return;
  }
  sleek_report("insert_next_token", "type = %d, index = %zu", tok.type, state.tok_next.write_idx);
  state.tok_next.buffer[state.tok_next.write_idx] = tok;
  state.tok_next.write_idx = (state.tok_next.write_idx + 1) % lexer_buffer_max;
  state.tok_next.len++;
}

void insert_prev_token(sleek_tok tok) {
  sleek_report("insert_prev_token", "type = %d, index = %zu", tok.type, state.tok_prev.write_idx);
  state.tok_prev.buffer[state.tok_prev.write_idx] = tok;
  state.tok_prev.write_idx = (state.tok_prev.write_idx + 1) % lexer_buffer_max;
  if (state.tok_prev.len < lexer_buffer_max) {
    state.tok_prev.len++;
  }
}

void peek_token(sleek_tok *dest, int idx) {
  if (idx == 0) {
    if (!state.cur_tok_valid) {
      insert_next_token(lex_token());
      *dest = state.cur_tok;
    }
    *dest = state.cur_tok;
  } else if (idx > 0) {
    if ((size_t)idx > lexer_buffer_max) {
      sleek_error("peek_token", "index exceeds buffer size (%d > %d)", idx, lexer_buffer_max);
      *dest = INVALID_TOKEN;
      return;
    }
    while (state.tok_next.len < (size_t)idx) {
      sleek_tok tok = lex_token();
      insert_next_token(tok);
      if ( tok.type == sleek_tok_type_eof ) {
        while (state.tok_next.len < (size_t)idx) {
          insert_next_token(tok);
        }
        break;
      }
    }
    size_t index = (state.tok_next.read_idx + (size_t)idx - 1) % lexer_buffer_max;
    *dest = state.tok_next.buffer[index];
  } else { // idx < 0
    int back = -idx;
    if ((size_t)back > state.tok_prev.len) {
      sleek_error("peek_token", "index %d exceeds buffer length (%d >= %zu)", idx, back, state.tok_prev.len);
      *dest = INVALID_TOKEN;
      return;
    }
    size_t index = (state.tok_prev.write_idx + lexer_buffer_max - back) % lexer_buffer_max;
    *dest = state.tok_prev.buffer[index];
  }
  sleek_report("peek_token", "idx = %d, type = %d", idx, dest->type);
}

void advance_token() {
  if (!state.cur_tok_valid) {
    state.cur_tok = lex_token();
    state.cur_tok_valid = true;
    sleek_report("advance_token", "primed current token; type = %d", state.cur_tok.type);
    return;
  }
  if (state.cur_tok.type == sleek_tok_type_eof && state.tok_next.len == 0) {
    sleek_warn("advance_token", "already at eof");
    return;
  }
  insert_prev_token(state.cur_tok);
  if (state.tok_next.len > 0) {
    state.cur_tok = state.tok_next.buffer[state.tok_next.read_idx];
    state.tok_next.read_idx = (state.tok_next.read_idx + 1) % lexer_buffer_max;
    state.tok_next.len--;
  } else {
    state.cur_tok = lex_token();
  }
  state.cur_tok_valid = true;
  sleek_report("advance_token", "type = %d, index = %zu", state.cur_tok.type, state.tok_next.read_idx);
}

void advance_token_by(int count) {
  if (count < 0) {
    sleek_error("advance_token_by", "count is negative (%d)", count);
    return;
  }
  for (int i = 0; i < count; i++) {
    advance_token();
  }
}

void state_reset() {
  lexer_state fresh = {
    .source = NULL,
    .remaining = 0,
    .current_char = '\0',
    .errptr = NULL,
    .cur_tok = INVALID_TOKEN,
    .cur_tok_valid = false,
    .tok_next = {
      .buffer = {0},
      .len = 0,
      .read_idx = 0,
      .write_idx = 0
    },
    .tok_prev = {
      .buffer = {0},
      .len = 0,
      .write_idx = 0
    }
  };
  state = fresh;
  sleek_report("state_reset", "lexer state reset");
}

void state_init(const char *src, const char **errdest) {
  sleek_report("state_init", "initializing lexer");
  state_reset();
  set_source(src, strlen(src));
  set_errptr(errdest);
}