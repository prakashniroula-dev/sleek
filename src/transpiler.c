#include <defs.h>
#include <log.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

// memory arena for transpiled code
static char buf[1024 * 1024]; // 1MB arena
static size_t buflen = 0;

void transpiler_reset()
{
  buflen = 0;
}

void transpiler_emit(const char *code)
{
  size_t len = strlen(code);
  if (buflen + len >= sizeof(buf))
  {
    sleek_error("out of memory in transpiler arena");
    return;
  }
  memcpy(buf + buflen, code, len);
  buflen += len;
}

void transpiler_emitf(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  int len = vsnprintf(buf + buflen, sizeof(buf) - buflen, format, args);
  va_end(args);
  if (len < 0 || (size_t)len >= sizeof(buf) - buflen)
  {
    sleek_error("out of memory in transpiler arena");
    return;
  }
  buflen += len;
}

static void transpile_ast_recursive(sleek_ast_node *node, int level)
{
  if (node == NULL)
    return;

  for (int i = 0; i < level; i++)
  {
    transpiler_emit("  "); // indent with 2 spaces per level
  }

  switch (node->type)
  {
  case sleek_ast_node_type_none:
    // do nothing
    break;
  case sleek_ast_node_type_fn_defn:
    const char *return_type_str = "void"; // TODO: handle return type properly
    if (strncmp(node->data.fn_defn.fn_name.ptr, "main", node->data.fn_defn.fn_name.length) == 0)
    {
      return_type_str = "int"; // main function should return int
    }
    transpiler_emitf("%s %.*s() {\n", return_type_str, (int)node->data.fn_defn.fn_name.length, node->data.fn_defn.fn_name.ptr);
    transpile_ast_recursive(node->data.fn_defn.body, level + 1);
    if (strncmp(node->data.fn_defn.fn_name.ptr, "main", node->data.fn_defn.fn_name.length) == 0)
    {
      transpiler_emit("  return 0;\n"); // main function should return 0
    }
    transpiler_emit("}\n");
    break;
  case sleek_ast_node_type_fn_call:
    
    if (strncmp(node->data.fn_defn.fn_name.ptr, "println", node->data.fn_defn.fn_name.length) == 0)
    {
      transpiler_emit("printf(\"");
      struct sleek_args *arg = node->data.fn_call.args;
      while (arg != NULL)
      {
        if (arg->data->type == sleek_ast_node_type_literal && arg->data->data.literal.type == sleek_args_type_string)
        {
          transpiler_emitf("%%s");
        }
        else if (arg->data->type == sleek_ast_node_type_literal && arg->data->data.literal.type == sleek_args_type_int64)
        {
          transpiler_emitf("%%lld");
        }
        else
        {
          sleek_error("Unsupported argument type for print function");
          return;
        }
        if (arg->next != NULL)
          transpiler_emit(" ");
        arg = arg->next;
      }
      transpiler_emit("\\n\", ");
    } else {
      transpiler_emitf("%.*s(", (int)node->data.fn_call.fn_name.length, node->data.fn_call.fn_name.ptr);
    }
    struct sleek_args *arg = node->data.fn_call.args;
    while (arg != NULL)
    {
      transpile_ast_recursive(arg->data, 0);
      if (arg->next != NULL)
        transpiler_emit(", ");
      arg = arg->next;
    }
    transpiler_emit(");\n");
    break;
  case sleek_ast_node_type_literal:
    if (node->data.literal.type == sleek_args_type_int64)
    {
      transpiler_emitf("%lld", node->data.literal.data._int64);
    }
    else if (node->data.literal.type == sleek_args_type_string)
    {
      transpiler_emitf("\"%.*s\"", (int)node->data.literal.data._string.length, node->data.literal.data._string.ptr);
    }
    break;
  default:
    sleek_error("transpile_ast_recursive", "Unknown AST node type: %d", node->type);
  }

  return transpile_ast_recursive(node->next, level);
}

const char *transpile_ast(sleek_ast_node *node)
{
  transpiler_reset();
  /// add necessary includes for C code
  transpiler_emit("#include <stdio.h>\n");
  transpile_ast_recursive(node, 0);
  if (buflen == 0)
  {
    sleek_warn("Transpiler produced no output");
    return NULL;
  }
  buf[buflen] = '\0'; // null-terminate the output
  return buf;
}