#pragma once
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <iostream>

/* Function-like macro for printing the file and line number where the error occurred on exit */
#define log_exit1(msg) \
{ \
	log_and_exit1(msg, __FILE__, __LINE__);\
}

/* IO utilities to simplify writing and reading to/from stdout, stderr & stdin. */

/* Writing */
/* stderr */
void wr_stderr(const char *msg, size_t n);
void wr_stderr(const char *msg);

/* stdin */
void wr_stdin(const char *msg, size_t n);
void wr_stdin(const char *msg);

/* stdout */
void wr_stdout(const char *msg, size_t n);
void wr_stdout(const char *msg);

/* Reading */

/* Utility that logs the linenumber, filename then exits */
void log_and_exit1(const char *msg, const char* filename, int lineNumber) noexcept;
