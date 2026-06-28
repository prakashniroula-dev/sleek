#include <stdio.h>
#include <lexer.h>
#include <parser.h>
#include <transpiler.h>
#include <stdlib.h>

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
    if (tok.data.literal.type == sleek_args_type_int64) {
      printf("  Value: %lld\n", tok.data.literal.data._int64);
    }
    else if (tok.data.literal.type == sleek_args_type_string) {
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


void print_space_level(int level) {
  for (int i = 0; i < level; i++) {
    printf("  ");
  }
}

void print_sleek_ast(sleek_ast_node* node, int level) {
  if ( node == NULL) return;
  print_space_level(level);
  if (node->type == sleek_ast_node_type_fn_defn) {
    printf("[function_defn]: \n");
    print_space_level(level + 1);
    printf("name: %.*s\n", (int)node->data.fn_defn.fn_name.length, node->data.fn_defn.fn_name.ptr);
    print_space_level(level + 1);
    printf("return_type: %d\n", node->data.fn_defn.return_type);
    print_space_level(level + 1);
    printf("body:\n");
    print_sleek_ast(node->data.fn_defn.body, level + 2);
  }
  else if (node->type == sleek_ast_node_type_fn_call) {
    printf("[function_call]: \n");
    print_space_level(level + 1);
    printf("name: %.*s\n", (int)node->data.fn_call.fn_name.length, node->data.fn_call.fn_name.ptr);
    print_space_level(level + 1);
    print_space_level(level + 1);
    printf("args:\n");
    struct sleek_args* arg = node->data.fn_call.args;
    int arg_index = 0;
    while (arg != NULL) {
      print_space_level(level + 2);
      printf("arg %d:\n", arg_index++);
      print_sleek_ast(arg->data, level + 3);
      arg = arg->next;
    }
  }
  else if (node->type == sleek_ast_node_type_literal) {
    printf("[literal]: \n");
    print_space_level(level + 1);
    printf("type: %d\n", node->data.literal.type);
    if (node->data.literal.type == sleek_args_type_int64) {
      print_space_level(level + 1);
      printf("value: %lld\n", node->data.literal.data._int64);
    }
    else if (node->data.literal.type == sleek_args_type_string) {
      print_space_level(level + 1);
      printf("value: %.*s\n", (int)node->data.literal.data._string.length, node->data.literal.data._string.ptr);
    }
  }
  node = node->next;
  if (node != NULL) {
    print_sleek_ast(node, level);
  }
}

const char* code = 
  "fn main() {\n"
  "  println(\"\\\"Hello Sleek!\\\"\", \"(\", 5, 1, 3, 3, \"K )\");\n"
  "}"
  ;

void lexerTest() {
  sleek_lex_init(code, NULL);
  sleek_tok tok = sleek_tok_invalid();
  while ((tok.type != sleek_tok_type_eof)) {
    tok = sleek_peek_token(0);
    print_sleek_token(tok);
    sleek_advance_token();
  }
}

void parserTest() {
  sleek_ast_node* ast = sleek_parse(code);
  printf("\n=== AST ===\n");
  print_sleek_ast(ast, 0);
}

const char* transpilerTest() {
  sleek_ast_node* ast = sleek_parse(code);
  const char* output = transpile_ast(ast);
  printf("\n=== Transpiled Output ===\n");
  printf("%s\n", output);
  return output;
}

void test() {
  printf("=== Sleek Code ===\n");
  printf("%s\n\n", code);
  printf("=== Lexer Test ===\n");
  lexerTest();
  printf("\n=== Parser Test ===\n");
  parserTest();
  printf("\n=== Transpiler Test ===\n");
  const char* output = transpilerTest();
  printf("Saving transpiled output to 'output.c'\n");
  FILE* f = fopen("output.c", "w");
  if (f == NULL) {
    printf("Error opening file for writing\n");
    return;
  }
  fprintf(f, "%s", output);
  fclose(f);
  printf("Compilation Test: Compiling 'output.c'...\n");
  int ret = system("gcc output.c -o output");
  if (ret != 0) {
    printf("Compilation failed with return code %d\n", ret);
    return;
  }
  printf("Running compiled output...\n\n");
  ret = system("output");
  if (ret != 0) {
    printf("Execution failed with return code %d\n", ret);
    return;
  }
  printf("\nExecution completed successfully\n");
  printf("Cleaning up: Removing 'output.c' and 'output'\n");
  remove("output.c");
  remove("output.exe");
}

int main() {
  test();
  return 0;
}