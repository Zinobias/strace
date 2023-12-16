#include "../inc/io_utils.hpp"

/**
 * utility functions for writing
*/
void wr_stderr(const char *msg)
{
	std::cerr << msg << std::endl;
    return ;
}

/**
 * Prints the line number of the error, and exits with 1
 * should only be called with the log_exit1 macro.
*/
void log_and_exit1(const char *msg, const char *filename, int lineNumber) noexcept
{
#ifndef PROD
	std::string num = std::to_string(lineNumber);
	std::cerr << "error on line number: " << num << " filename: " << filename;
#endif
	std::cerr << "\nernno if applicable: " << strerror(errno);
	std::cerr << "\nerror: " << msg << std::endl;

    std::string num = std::to_string(lineNumber);
	std::cerr << "error on line number: " << num << " filename: " << filename;
    exit(1);
}