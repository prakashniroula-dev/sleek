#if !defined(sleek_transpiler_h)
#define sleek_transpiler_h

#include <defs.h>

void transpiler_reset();
const char* transpile_ast(sleek_ast_node *node);

#endif // sleek_transpiler_h
