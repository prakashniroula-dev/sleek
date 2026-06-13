#if !defined(sleek_lexer_h)
#define sleek_lexer_h

#include <defs.h>

void sleek_lex_init(const char *src, const char **errdest);
sleek_tok sleek_peek_token(int idx);
void sleek_advance_token(void);

#endif // sleek_lexer_h
