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
    sleek_error("expected_token", "type %d, but got %d", type, tok.type);
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

struct sleek_args* sleek_parse_args()
{
  sleek_tok tok = sleek_peek_token(0);
  if (tok.type != sleek_tok_type_symbol || tok.data.symbol.type != sleek_symbol_type_parantheses_l)
  {
    sleek_error("expected_token", "Expected '(' at start of argument list");
    return NULL;
  }
  consume_token(1); // consume '('
  tok = sleek_peek_token(0);
  bool needs_comma = false;
  struct sleek_args *root = NULL;
  struct sleek_args *cur = NULL;
  while (!(tok.type == sleek_tok_type_symbol && tok.data.symbol.type == sleek_symbol_type_parantheses_r))
  {
    if (needs_comma)
    {
      if (tok.type != sleek_tok_type_symbol || tok.data.symbol.type != sleek_symbol_type_comma)
      {
        sleek_error("expected_token", "Expected ',' between arguments");
        return NULL;
      }
      consume_token(1); // consume ','
      tok = sleek_peek_token(0);
    }
    if (tok.type != sleek_tok_type_literal)
    {
      sleek_error("expected_token", "Expected literal argument");
      return NULL;
    }

    struct sleek_args *new_arg = arena_alloc(sizeof(struct sleek_args));
    new_arg->type = tok.data.literal.type;
    new_arg->data = arena_alloc(sizeof(sleek_ast_node));
    new_arg->data->type = sleek_ast_node_type_literal;
    new_arg->data->data.literal.type = tok.data.literal.type;
    new_arg->data->data.literal.data = tok.data.literal.data;
    new_arg->next = NULL;
    if ( root == NULL )
    {
      root = new_arg;
      cur = new_arg;
    }
    else
    {
      cur->next = new_arg;
      cur = new_arg;
    }

    consume_token(1); // consume literal
    needs_comma = true;
    tok = sleek_peek_token(0);
  }
  consume_token(1); // consume ')'
  return root;
}

sleek_ast_node *sleek_parse_stmt()
{
  // currently only supports function calls
  sleek_tok tok = sleek_peek_token(0);
  sleek_tok tok1 = sleek_peek_token(1);
  sleek_log("parse_stmt", "Peeking tokens in stmt: tok0 type=%d, tok1 type=%d", tok.type, tok1.type);
  if (match_token(sleek_tok_type_identifier) && tok1.type == sleek_tok_type_symbol && tok1.data.symbol.type == sleek_symbol_type_parantheses_l)
  {
    sleek_string fn_name = tok.data.identifier;
    consume_token(1); // consume identifier
    tok = sleek_peek_token(0);
    sleek_log("parse_stmt", "Peeking token after consuming identifier: type=%d, %d", tok.type, tok.data.symbol.type);
    sleek_ast_node *node = arena_alloc(sizeof(sleek_ast_node));
    node->type = sleek_ast_node_type_fn_call;
    node->data.fn_call.fn_name = fn_name;
    node->data.fn_call.args = sleek_parse_args();
    tok = sleek_peek_token(0);
    if (tok.type == sleek_tok_type_symbol && tok.data.symbol.type == sleek_symbol_type_semicolon)
    {
      consume_token(1); // consume ';'
    }
    return node;
  } else {
    sleek_error("parse_stmt", "Unsupported token type: %d", tok.type);
  }
  return NULL;
}

sleek_ast_node *sleek_parse_block()
{
  sleek_tok tok = sleek_peek_token(0);
  if (!expect_token(sleek_tok_type_symbol, &tok) || tok.data.symbol.type != sleek_symbol_type_braces_l)
  {
    sleek_error("parse_block", "Expected '{' at start of block");
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
      sleek_error("parse_block", "Failed to parse statement in block");
      break;
    }
  }
  sleek_report("parse_block", "Parsed block with %s statements", (root == NULL) ? "no" : "some");
  return root;
}

sleek_ast_node *sleek_parse_fn_defn()
{
  sleek_tok tok = sleek_peek_token(0);
  sleek_string fn_name;

  if (!expect_token(sleek_tok_type_keyword, &tok) || tok.data.keyword.type != sleek_keyword_type_fn)
  {
    sleek_error("parse_fn_defn", "Expected 'fn' keyword at start of function definition");
    return NULL;
  }

  if (!expect_token(sleek_tok_type_identifier, &tok))
  {
    sleek_error("parse_fn_defn", "Expected function name after 'fn' keyword");
    return NULL;
  }

  fn_name = tok.data.identifier;

  tok = sleek_peek_token(0);
  if (
      !match_token(sleek_tok_type_symbol) ||
      tok.data.symbol.type != sleek_symbol_type_parantheses_l)
  {
    sleek_error("parse_fn_defn", "Expected '(' after function name");
    return NULL;
  }
  consume_token(1); // consume '('

  tok = sleek_peek_token(0);

  // no argument parsing for now

  if (!match_token(sleek_tok_type_symbol) || tok.data.symbol.type != sleek_symbol_type_parantheses_r)
  {
    sleek_error("parse_fn_defn", "Expected ')' after function arguments");
    return NULL;
  }

  consume_token(1); // consume ')'

  tok = sleek_peek_token(0);
  sleek_log("parse_fn_defn", "Peeking token after function args: type=%d", tok.type);
  sleek_ast_node *node = arena_alloc(sizeof(sleek_ast_node));
  node->type = sleek_ast_node_type_fn_defn;
  node->data.fn_defn.fn_name = fn_name;
  node->data.fn_defn.return_type = sleek_args_type_void; // TODO: parse return type
  node->data.fn_defn.body = sleek_parse_block();         // TODO: parse function body
  return node;
}

sleek_ast_node *sleek_parse_toplevel()
{
  sleek_tok tok = sleek_peek_token(0);
  sleek_log("parse_toplevel", "parsing top level token, type=%d", tok.type);

  if (!match_token(sleek_tok_type_keyword))
  {
    sleek_error("parse_toplevel", "Expected keyword at toplevel");
    return NULL;
  }

  if (tok.data.keyword.type == sleek_keyword_type_fn)
  {
    sleek_ast_node* r = sleek_parse_fn_defn();
    sleek_report(
      "parse_toplevel", "parsed function definition: %.*s, with %s body",
      (int)r->data.fn_defn.fn_name.length,
      r->data.fn_defn.fn_name.ptr,
      r->data.fn_defn.body ? "some" : "no"
    );
    return r;
  }

  sleek_error("parse_toplevel", "Unexpected keyword at toplevel");
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
    sleek_log("parse_toplevel", "Peeking token at toplevel: type=%d", tok.type);
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
      sleek_error("parse_toplevel", "Failed to parse toplevel node");
      break;
    }
  }
  return root;
}