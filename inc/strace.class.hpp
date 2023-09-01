#pragma once
#include <string>
#include <sys/ptrace.h>

class strace
{
	public:
		/* Constructors etc*/
		strace(int argc, char *const argv[], const  std::string &paths);
		strace(int argc, char *const argv[], const  std::string &&paths);
		strace(const strace& rhs) = default;
		strace(strace&& rhs) noexcept = default;
		// strace& operator=(const strace& rhs) = default;
		// strace& operator=(const strace&& rhs) noexcept = default;

		~strace() = default ;

	public:
		void		exec_and_attach();

	private:
		char** 		construct_argv() ;
		void        ptrace_wr(__ptrace_request req) const;
		// bool        is_executable(const char* file_path) const ;
	
	private:
		int								argc;
		const  std::string				exec_path;
		char *const*					args;

		// Options for supporting the -c flag
		size_t							total_syscalls;
		size_t							time_passed;
		size_t							total_errors;

		// child / tracee pid
		pid_t 							pid;

};
