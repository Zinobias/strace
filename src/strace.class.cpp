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

strace::strace(int argc, char *const argv[], const  std::vector<std::string> &paths) :
	argc(argc), args(argv), exec_paths(paths)
{

}

strace::strace(int argc, char *const argv[], const  std::vector<std::string> &&paths) :
	argc(argc), args(argv), exec_paths(paths)
{

}

char** strace::construct_argv(char *const exec)
{
    char **argv = (char **)calloc(argc + 1, sizeof(char **));
    if (argv == NULL)
        log_exit1("Malloc failure");
    memcpy(argv + 1, args, sizeof(this->args));
    argv[0] = exec;
    return (argv);
}

void        strace::ptrace_wr(__ptrace_request req) const
{
    errno = 0;
    if (ptrace(req, this->pid) == -1)
        log_exit1("ptrace error");
}

// bool        strace::is_executable(const char* file_path) const 
// {
//     struct stat sbuf;
//     int stat_ret = stat(file_path, &sbuf);
//     const int &mode = sbuf.st_mode;

//     bool    file_exists = stat_ret != -1;
//     bool    is_executable = mode & (S_IXGRP | S_IXOTH | S_IXUSR);
//     bool    is_regular = S_ISREG(mode);
//     return (file_exists && is_executable && is_regular);
// }

void		strace::exec_and_attach()
{
	this->pid = fork();
    if (this->pid == -1)
        log_exit1("Forking failed");
    /* We ptrace() the child process, before we execve(), in order to not miss the initial set of instructions. */
    if (this->pid != 0)
    {
        int wstatus;
        ptrace_wr(PTRACE_SEIZE);
        ptrace_wr(PTRACE_INTERRUPT);
        /* We wait for the process to properly stop, to ensure synchronization */
        waitpid(this->pid, &wstatus, __WALL);
        if (WIFSTOPPED(wstatus) == true)
        {
            // stopped. subprocess stopped successfully.
        }
        else
        {
            // Other reasons, probably error territory.
        }
        /* We start listening, it restarts the tracee, but prevents it from executing */
        ptrace_wr(PTRACE_LISTEN);
        // TODO: Confirm whether this is required.
        // if (!WIFSTOPPED(wstatus))
        //     log_exit1("Child failed to stop");

    }
    if (pid == 0)
    {
        nanosleep()
        // size_t cArraySize = sizeof(args) / sizeof(args[0]);
        for (auto path : exec_paths)
        {
            // TODO: Add new fullpath to args.
            std::vector<char *const> arguments;
            char **argv = construct_argv(path.data());
            if (execv(path.c_str(), (char *const*)argv) == -1)
                log_exit1("Execve failed");
            free(argv);
            exit(0);
        }
    }

}

// strace::strace(const strace& rhs);

// strace::strace(strace&& rhs) {}
// strace::~strace() {}


// Assignment overloads.
// strace& strace::operator=(const strace& rhs);