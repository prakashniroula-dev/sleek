#if !defined(sleek_defs_h)
#define sleek_defs_h

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

enum sleek_tok_type;
enum sleek_keyword_types;
enum sleek_identifier_types;
enum sleek_symbol_types;
enum sleek_literal_types;

struct sleek_keyword;
// struct sleek_identifier;
struct sleek_symbol;
struct sleek_literal;

typedef struct sleek_string {
  const char* ptr;
  size_t length;
} sleek_string;

union sleek_tok_data {
  int64_t _int64;
  uint64_t _uint64;
  float _float32;
  double _float64;
  char _char;
  sleek_string _string;
};

enum sleek_tok_type {
  sleek_tok_type_invalid = -1,
  sleek_tok_type_none,
  sleek_tok_type_eof,
  sleek_tok_type_keyword,
  sleek_tok_type_identifier,
  sleek_tok_type_symbol,
  sleek_tok_type_literal
};

enum sleek_keyword_types {
  sleek_keyword_type_invalid = -1,
  sleek_keyword_type_none,
  sleek_keyword_type_fn = 1,
};

struct sleek_keyword {
  enum sleek_keyword_types type;
};

enum sleek_symbol_types {
  sleek_symbol_type_invalid = -1,
  sleek_symbol_type_none,
  sleek_symbol_type_parantheses_l,
  sleek_symbol_type_parantheses_r,
  sleek_symbol_type_braces_l,
  sleek_symbol_type_braces_r,
  sleek_symbol_type_semicolon,
  sleek_symbol_type_dblquote,
};

struct sleek_symbol {
  enum sleek_symbol_types type;
};

enum sleek_literal_types {
  sleek_literal_type_invalid = -1,
  sleek_literal_type_none,
  sleek_literal_type_string,
  sleek_literal_type_int64,
};

struct sleek_literal {
  enum sleek_literal_types type;
  union sleek_tok_data data;
};

typedef struct sleek_tok {
  enum sleek_tok_type type;
  union {
    struct sleek_literal literal;
    struct sleek_symbol symbol;
    struct sleek_string identifier;
    struct sleek_keyword keyword;
    int32_t _value;
  } data;
} sleek_tok;

static inline sleek_tok sleek_tok_invalid() {
  sleek_tok tok = {.type = sleek_tok_type_invalid, .data._value = 0};
  return tok;
}

#endif // sleek_defs_h