#include <defs.h>
#include <log.h>
#include <string.h>
#include <stdbool.h>
#include <lexer.h>

// memory arena for AST nodes
static char buf[1024 * 1024]; // 1MB arena
static size_t buflen = 0;

static void *arena_alloc(size_t size)
{
  if (buflen + size > sizeof(buf))
  {
    sleek_error("out of memory in arena");
    return NULL;
  }
  void *ptr = buf + buflen;
  buflen += size;
  return ptr;
}

void arena_reset()
{
  buflen = 0;
}

static bool arena_attempt_backtrack(size_t size, void *end)
{
  if (buf + buflen != end + size)
    return false;
  buflen -= size;
  return true;
}


void consume_token(int count)
{
  sleek_advance_token_by(count);
}

bool expect_token(enum sleek_tok_type type, sleek_tok *dest)
{
  sleek_tok tok = sleek_peek_token(0);
  if (tok.type != type)
  {
    sleek_error("Expected token of type %d but got %d", type, tok.type);
    if (dest)
      *dest = sleek_tok_invalid();
    return false;
  }
  if (dest)
    *dest = tok;
  consume_token(1); // consume token
  return true;
}

bool match_token(enum sleek_tok_type type)
{
  sleek_tok tok = sleek_peek_token(0);
  if (tok.type != type)
  {
    return false;
  }
  return true;
}

struct sleek_args sleek_parse_args()
{
  struct sleek_args args = {0};
  sleek_tok tok = sleek_peek_token(0);
  if (tok.type != sleek_tok_type_symbol || tok.data.symbol.type != sleek_symbol_type_parantheses_l)
  {
    sleek_error("Expected '(' at start of argument list");
    return args;
  }
  consume_token(1); // consume '('
  tok = sleek_peek_token(0);
  bool needs_comma = false;
  while (!(tok.type == sleek_tok_type_symbol && tok.data.symbol.type == sleek_symbol_type_parantheses_r))
  {
    if (needs_comma)
    {
      if (tok.type != sleek_tok_type_symbol || tok.data.symbol.type != sleek_symbol_type_comma)
      {
        sleek_error("Expected ',' between arguments");
        return args;
      }
      consume_token(1); // consume ','
      tok = sleek_peek_token(0);
    }
    if (tok.type != sleek_tok_type_literal)
    {
      sleek_error("Expected literal argument");
      return args;
    }
    if (args.type == sleek_args_type_none)
    {
      args.type = tok.data.literal.type;
      args.data = arena_alloc(sizeof(sleek_ast_node));
      args.data->type = sleek_ast_node_type_literal;
      args.data->data.literal.type = tok.data.literal.type;
      args.data->data.literal.data = tok.data.literal.data;
    }
    consume_token(1); // consume literal
    needs_comma = true;
    tok = sleek_peek_token(0);
  }
  consume_token(1); // consume ')'
  return args;
}

sleek_ast_node *sleek_parse_stmt()
{
  // currently only supports function calls
  sleek_tok tok = sleek_peek_token(0);
  sleek_tok tok1 = sleek_peek_token(1);
  sleek_log("Peeking tokens in stmt: tok0 type=%d, tok1 type=%d", tok.type, tok1.type);
  if (match_token(sleek_tok_type_identifier) && tok1.type == sleek_tok_type_symbol && tok1.data.symbol.type == sleek_symbol_type_parantheses_l)
  {
    sleek_string fn_name = tok.data.identifier;
    consume_token(1); // consume identifier
    tok = sleek_peek_token(0);
    sleek_log("Peeking token after consuming identifier: type=%d, %d", tok.type, tok.data.symbol.type);
    struct sleek_args args = sleek_parse_args();
    sleek_ast_node *node = arena_alloc(sizeof(sleek_ast_node));
    node->type = sleek_ast_node_type_fn_call;
    node->data.fn_call.fn_name = fn_name;
    node->data.fn_call.arg_count = args.type == sleek_args_type_none ? 0 : 1;
    if (args.data != NULL)
    {
      struct sleek_ast_call_args *call_arg = arena_alloc(sizeof(struct sleek_ast_call_args));
      call_arg->type = args.type;
      call_arg->data = args.data;
      call_arg->next = NULL;
      node->data.fn_call.args = call_arg;
    }
    else
    {
      node->data.fn_call.args = NULL;
    }
    tok = sleek_peek_token(0);
    if (tok.type == sleek_tok_type_symbol && tok.data.symbol.type == sleek_symbol_type_semicolon)
    {
      consume_token(1); // consume ';'
    }
    return node;
  }
  return NULL;
}

sleek_ast_node *sleek_parse_block()
{
  sleek_tok tok = sleek_peek_token(0);
  if (!expect_token(sleek_tok_type_symbol, &tok) || tok.data.symbol.type != sleek_symbol_type_braces_l)
  {
    sleek_error("Expected '{' at start of block");
    return NULL;
  }
  sleek_ast_node *root = NULL;
  sleek_ast_node *cur = NULL;
  while (true)
  {
    sleek_tok tok = sleek_peek_token(0);
    if (tok.type == sleek_tok_type_symbol && tok.data.symbol.type == sleek_symbol_type_braces_r)
    {
      consume_token(1); // consume '}'
      break;
    }
    sleek_ast_node *stmt = sleek_parse_stmt();
    if (stmt)
    {
      if (root == NULL)
      {
        root = stmt;
      }
      else
      {
        cur->next = stmt;
      }
      cur = stmt;
    }
    else
    {
      sleek_error("Failed to parse statement in block");
      break;
    }
  }
  return root;
}

sleek_ast_node *sleek_parse_fn_defn()
{
  sleek_tok tok = sleek_peek_token(0);
  sleek_string fn_name;

  if (!expect_token(sleek_tok_type_keyword, &tok) || tok.data.keyword.type != sleek_keyword_type_fn)
  {
    sleek_error("Expected 'fn' keyword at start of function definition");
    return NULL;
  }

  if (!expect_token(sleek_tok_type_identifier, &tok))
  {
    sleek_error("Expected function name after 'fn' keyword");
    return NULL;
  }

  fn_name = tok.data.identifier;

  tok = sleek_peek_token(0);
  if (
      !match_token(sleek_tok_type_symbol) ||
      tok.data.symbol.type != sleek_symbol_type_parantheses_l)
  {
    sleek_error("Expected '(' after function name");
    return NULL;
  }
  consume_token(1); // consume '('

  tok = sleek_peek_token(0);

  // no argument parsing for now

  if (!match_token(sleek_tok_type_symbol) || tok.data.symbol.type != sleek_symbol_type_parantheses_r)
  {
    sleek_error("Expected ')' after function arguments");
    return NULL;
  }

  consume_token(1); // consume ')'

  tok = sleek_peek_token(0);
  sleek_log("Peeking token after function args: type=%d", tok.type);
  sleek_ast_node *node = arena_alloc(sizeof(sleek_ast_node));
  node->type = sleek_ast_node_type_fn_defn;
  node->data.fn_defn.fn_name = fn_name;
  node->data.fn_defn.return_type = sleek_args_type_void; // TODO: parse return type
  node->data.fn_defn.body = sleek_parse_block(); // TODO: parse function body
  return node;
}

sleek_ast_node *sleek_parse_toplevel()
{
  sleek_tok tok = sleek_peek_token(0);
  sleek_log("Peeking token at toplevel: type=%d", tok.type);

  if (!match_token(sleek_tok_type_keyword))
  {
    sleek_error("Expected keyword at toplevel");
    return NULL;
  }

  if (tok.data.keyword.type == sleek_keyword_type_fn)
  {
    return sleek_parse_fn_defn();
  }

  sleek_error("Unexpected keyword at toplevel");
  return NULL;
}

sleek_ast_node *sleek_parse(const char *source)
{
  arena_reset();
  sleek_lex_reset();
  sleek_lex_init(source, NULL);
  sleek_ast_node *root = arena_alloc(sizeof(sleek_ast_node));
  root->type = sleek_ast_node_type_none;
  root->next = NULL;
  sleek_ast_node *cur = root;
  while (true)
  {
    sleek_tok tok = sleek_peek_token(0);
    sleek_log("Peeking token at toplevel: type=%d", tok.type);
    if (tok.type == sleek_tok_type_eof)
      break;
    sleek_ast_node *node = sleek_parse_toplevel();
    if (node)
    {
      cur->next = node;
      cur = node;
    }
    else
    {
      sleek_error("Failed to parse toplevel node");
      break;
    }
  }
  return root;
}