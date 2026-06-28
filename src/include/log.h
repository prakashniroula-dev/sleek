#if !defined(sleek_log_h)
#define sleek_log_h

#include <stdio.h>
#include <string.h>
#include <defs.h>

// 0 = None, 1 = Errors, 2 = Errors/Warnings, 3 = Normal logs
// 4 = Log everything
#if !defined(sleek_log_level)
#define sleek_log_level 4
#endif

static inline size_t sleek_log__string_fn(const char* prefix, sleek_string s) {
  size_t length = s.length == 0 && s.ptr != NULL ? strlen(s.ptr): s.length;
  return printf(prefix) + fwrite(s.ptr, sizeof *s.ptr, length, stdout);
}

static inline int sleek_log__spaces(int level) {
  int result = 0;
  for ( int i = 0; i < level; i++ ) {
    result += printf("  ");
  }
  return result;
}

extern const char *sleek_log__filename;
extern const char *sleek_log__funcname;
extern const char *sleek_log__topic;

int sleek_log__print_info(const char* filename, const char* funcname, const char* line_str, const char* topic);

#define sleekm__force_str(x) #x
#define sleekm_force_str(x) sleekm__force_str(x)
#define sleekm_line_str() sleekm_force_str(__LINE__)
#define sleekm_func_str() __func__
#define sleekm_file_str() __FILE__

// #define sleek_log_info(name) \
//   "\n[file: " sleekm_file_str() "]" \
//   "\n[line: " sleekm_line_str() "][" name "]: "

#define sleek_log_info(name) \
  sleek_log__print_info(  \
    sleekm_file_str(), \
    sleekm_func_str(), \
    sleekm_line_str(), \
    name \
  )

// =========== Logging: Level 3 ===========
#define sleek_log(topic, args...) \
  (sleek_log_level >= 3 ? sleek_log_info("log") + printf("(" topic ") " args): 0)
  
#define sleek_log_continue(level, args...) \
  (sleek_log_level >= 3 ? sleek_log__spaces(level) + printf("" args): 0)

#define sleek_log_string(topic, sleek_str) \
  (sleek_log_level >= 3 ? sleek_log_info("log") + sleek_log__string_fn("(" topic ") ", sleek_str): 0)

#define sleek_log_string_continue(level, sleek_str) \
  (sleek_log_level >= 3 ? sleek_log__spaces(level) + sleek_log__string_fn("", sleek_str): 0)

// ========== Warning: Level 2 ===========
  
#define sleek_warn(topic, args...) \
  (sleek_log_level >= 2 ? sleek_log_info("warn") + printf("(" topic ") " args): 0)
  
#define sleek_warn_continue(level, args...) \
  (sleek_log_level >= 2 ? sleek_log__spaces(level) + printf("" args): 0)

#define sleek_warn_string(topic, sleek_str) \
  (sleek_log_level >= 2 ? sleek_log_info("warn") + sleek_log__string_fn("(" topic ") ", sleek_str): 0)
  
  #define sleek_warn_string_continue(level, sleek_str) \
  (sleek_log_level >= 2 ? sleek_log__spaces(level) + sleek_log__string_fn("", sleek_str): 0)

// ========== Errors: Level 1 =============

#define sleek_error(topic, args...) \
  (sleek_log_level >= 1 ? sleek_log_info("error") + printf("(" topic ") " args): 0)

#define sleek_error_continue(level, args...) \
  (sleek_log_level >= 1 ? sleek_log__spaces(level) + printf("" args): 0)

#define sleek_error_string(topic, sleek_str) \
  (sleek_log_level >= 1 ? sleek_log_info("error") + sleek_log__string_fn("(" topic ") ", sleek_str): 0)

#define sleek_error_string_continue(level, sleek_str) \
  (sleek_log_level >= 1 ? sleek_log__spaces(level) + sleek_log__string_fn("", sleek_str): 0)

// =========== Everything Report: Level 4 ==============

#define sleek_report(topic, args...) \
  (sleek_log_level >= 4 ? sleek_log_info("report") + printf("(" topic ") " args): 0)

#define sleek_report_continue(level, args...) \
  (sleek_log_level >= 4 ? sleek_log__spaces(level) + printf("" args): 0)

#define sleek_report_string(topic, sleek_str) \
  (sleek_log_level >= 4 ? sleek_log_info("report") + sleek_log__string_fn("(" topic ") ", sleek_str): 0)

#define sleek_report_string_continue(level, sleek_str) \
  (sleek_log_level >= 4 ? sleek_log__spaces(level) + sleek_log__string_fn("", sleek_str): 0)

#endif // sleek_log_h
