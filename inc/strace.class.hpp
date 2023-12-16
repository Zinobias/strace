#pragma once
#include <string>
#include <sys/ptrace.h>

typedef enum state_e {
    attach, next_syscall,
    neutral, inspect_tracee,
    wait_child,
}   state_t ;

class strace
{
	public:
		/* Constructors etc*/
		strace(int argc, char *const argv[], const  std::string &paths);
		strace(int argc, char *const argv[], const  std::string &&paths);
		strace(const strace& rhs) = delete;
		strace(strace&& rhs) noexcept = delete;
		strace& operator=(const strace& rhs) = delete;
		strace& operator=(const strace&& rhs) noexcept = delete;

		~strace() = default ;

	public:
		void		        start();

	private:
		char** 		        construct_argv() ;
		void                ptrace_wr(__ptrace_request req, void* addr, void* data) const;
        static void         print_gpregs(const struct user_regs_struct* const regs);
        void                run_state_machine();
		// bool        is_executable(const char* file_path) const ;
	
	private:
		int								argc;
		const  std::string				exec_path;
		char *const*					args;

		// Options for supporting the -c flag
		size_t							total_syscalls;
		size_t							time_passed /* Time passed since the initial execution ni*/;
		size_t							total_errors;

		pid_t 							pid /* tracee */;
        state_t                         state /* state of the state machine */;

};
