#include <stdio.h>
#include <lexer.h>

void print_sleek_token(sleek_tok tok) {
  if ( tok.type == sleek_tok_type_keyword ) {
    printf("Keyword: %d\n", tok.data.keyword.type);
  }
  else if ( tok.type == sleek_tok_type_identifier ) {
    printf("Identifier: %.*s\n", (int)tok.data.identifier.length, tok.data.identifier.ptr);
  }
  else if ( tok.type == sleek_tok_type_symbol ) {
    printf("Symbol: %d\n", tok.data.symbol.type);
  }
  else if ( tok.type == sleek_tok_type_literal ) {
    printf("Literal: %d\n", tok.data.literal.type);
    if (tok.data.literal.type == sleek_literal_type_int64) {
      printf("  Value: %lld\n", tok.data.literal.data._int64);
    }
    else if (tok.data.literal.type == sleek_literal_type_string) {
      printf("  Value: %.*s\n", (int)tok.data.literal.data._string.length, tok.data.literal.data._string.ptr);
    }
  }
  else if ( tok.type == sleek_tok_type_eof ) {
    printf("End of File\n");
  }
  else {
    printf("Invalid Token\n");
  }
}

int main() {
  const char* code = 
  "fn main() {\n"
  "  printf(\"Hello Sleek\");"
  "}"
  ;

  sleek_lex_init(code, NULL);
  sleek_tok tok = sleek_tok_invalid();
  while ((tok.type != sleek_tok_type_eof)) {
    tok = sleek_peek_token(0);
    print_sleek_token(tok);
    sleek_advance_token();
  }
  return 0;
}