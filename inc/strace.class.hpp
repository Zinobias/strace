#pragma once
#include <string>
#include <sys/ptrace.h>
#include <map>
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
        static void         init_syscall_maps();
        /* Initialize the x64 and x86 syscall maps, distribution specific, can fail, which is okay */

	private:
		char** 		        construct_argv();
		void                ptrace_wr(__ptrace_request req, void* addr, void* data) const;
        static void         print_gpregs(const struct user_regs_struct* const regs);
        void                run_state_machine();
        static void         init_x64_map();
        static void         init_x86_map();

		// bool        is_executable(const char* file_path) const ;
	
	private:
		int								        argc;
		const  std::string				        exec_path;
		char *const*					        args;

		// Options for supporting the -c flag
		size_t							        total_syscalls;
		size_t							        time_passed /* Time passed since the initial execution ni*/;
		size_t							        total_errors;

		pid_t 							        pid /* tracee */;
        state_t                                 state /* state of the state machine */;
        static std::map<int, std::string>       x86_syscalls;
        static std::map<int, std::string>       x64_syscalls;

    friend void        fill_syscall_map(const char* const& target_file);
};
