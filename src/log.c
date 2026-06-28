#include <stdio.h>
#include <string.h>

const char *sleek_log__filename = NULL;
const char *sleek_log__funcname = NULL;

int sleek_log__print_info(const char* filename, const char* funcname, const char* line_str, const char* topic) {
  static int same_file = 1;
  static int same_func = 1;
  int out = 0;
  if (!sleek_log__filename || strncmp(filename, sleek_log__filename, strlen(filename)) != 0) {
    sleek_log__filename = filename;
    out += printf("\n\n[file: \"%s\" ]\n", filename);
  } else {
    out += printf("\n");
  }
  if (!sleek_log__funcname || strncmp(funcname, sleek_log__funcname, strlen(funcname)) != 0) {
    sleek_log__funcname = funcname;
    out += printf("-> %s()\n", funcname);
  }
  out += printf("  [%s] ", topic);
  out += printf("(line %s): ", line_str);
  return out;
}