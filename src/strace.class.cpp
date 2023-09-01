#include "../inc/strace.class.hpp"
#include <utility>
#include <filesystem>
#include <unistd.h>
#include <sys/stat.h>
#include <wait.h>
#include <vector>
#include "../inc/strace.hpp"
#include "../inc/io_utils.hpp"
#include <stdlib.h>
#include <sys/ptrace.h>

strace::strace(int argc, char *const argv[], const  std::string &exec_path) :
	argc(argc), args(argv), exec_path(exec_path) {}

strace::strace(int argc, char *const argv[], const std::string &&exec_path) :
	argc(argc), args(argv), exec_path(exec_path) {}

char** strace::construct_argv()
{
    char **argv = (char **)calloc(argc + 1, sizeof(char **));
    if (argv == NULL)
        log_exit1("Malloc failure");
    memcpy(argv + 1, args, (argc + 1) * sizeof(char*));
    std::cout << "sizeof the args : {" << sizeof(this->args) << "}" << "size of i{" << argc << "}" << std::endl;
    argv[0] = const_cast<char *>(this->exec_path.data());
    return (argv);
}

void        strace::ptrace_wr(__ptrace_request req) const
{
    errno = 0;
    if (ptrace(req, this->pid) == -1)
        log_exit1("ptrace error");
}

void		strace::exec_and_attach()
{
    char **argv = construct_argv();

    std::cout << "args in exec attach: " << std::endl;
    std::cout << argv[0] << " || " << argv[1] << " || " << argv[2] << " || " << argv[3] << " || " << argv[4] << std::endl;
    free(argv);
	// this->pid = fork();
    // if (this->pid == -1)
    //     log_exit1("Forking failed");
    // /* We ptrace() the child process, before we execve(), in order to not miss the initial set of instructions. */
    // if (this->pid != 0)
    // {
    //     int wstatus;
    //     ptrace_wr(PTRACE_SEIZE);
    //     ptrace_wr(PTRACE_INTERRUPT);
    //     /* We wait for the process to properly stop, to ensure synchronization */
    //     waitpid(this->pid, &wstatus, __WALL);
    //     if (WIFSTOPPED(wstatus) == true)
    //     {
    //         // stopped. subprocess stopped successfully.
    //     }
    //     else
    //     {
    //         // Other reasons, probably error territory.
    //     }
    //     /* We start listening, it restarts the tracee, but prevents it from executing */
    //     ptrace_wr(PTRACE_LISTEN);
    //     // TODO: Confirm whether this is required.
    //     // if (!WIFSTOPPED(wstatus))
    //     //     log_exit1("Child failed to stop");

    // }
    // if (pid == 0)
    // {
    //     // nanosleep(500);
    //     // size_t cArraySize = sizeof(args) / sizeof(args[0]);
    //     std::vector<char *const> arguments;
    //     char **argv = construct_argv();
    //     // if (execv(exec_path.c_str(), (char *const*)argv) == -1)
    //     // {
    //     //     log_exit1("Execv failed");
    //     //     free(argv);
    //     //     exit(0);
    //     // }
    // }

}

// strace::strace(const strace& rhs);

// strace::strace(strace&& rhs) {}
// strace::~strace() {}


// Assignment overloads.
// strace& strace::operator=(const strace& rhs);