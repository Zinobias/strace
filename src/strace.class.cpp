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
#include <sys/uio.h>

#include <sys/param.h>
#include <sys/user.h>
#include <sys/types.h>
#include <sys/procfs.h>
// fOR PR-STATUS STUFF AND ELF stuff
#include <elf.h>
#include <assert.h>

// For parsing the syscall headers
#include <regex>
#include <fstream>
#include <iostream>

strace::strace(int argc, char *const argv[], const  std::string &exec_path) :
	argc(argc), args(argv), exec_path(exec_path) {}

strace::strace(int argc, char *const argv[], const std::string &&exec_path) :
	argc(argc), args(argv), exec_path(std::move(exec_path)) {}

/* We extract the new argument vector from the original argv, by substracting the original command & command to run */
char** strace::construct_argv()
{
    // char **argv = (char **)calloc(argc + 1, sizeof(char **));
    char **argv = new char*[argc + 1]();
    if (argv == NULL)
        log_exit1("Malloc failure");
    memcpy(argv, args, (argc + 1) * sizeof(char*));
    argv[0] = const_cast<char *>(this->exec_path.data());
    return (argv);
}

void   strace::print_gpregs(const struct user_regs_struct* const regs)
{
    // TODO: Actually print gpregs.
    assert(false);
}

// long ptrace(enum __ptrace_request request, pid_t pid,
//                    void *addr, void *data);
void        strace::ptrace_wr(__ptrace_request req, void* addr = nullptr, void* data = nullptr) const
{
    /* We check the return value & errno and set it to 0, as in the case of PTRACE_PEEK, a succesful request may return -1 */
    errno = 0;
    // printf("-->[%ld]\n", ptrace(req, this->pid, addr, data));
    if (ptrace(req, this->pid, addr, data) == -1 && errno != 0)
        log_exit1("ptrace error");
}

void        strace::run_state_machine()
{
    static const int        ptrace_options = ( PTRACE_O_TRACEFORK | PTRACE_O_TRACECLONE | \
                                                PTRACE_O_TRACEEXEC | PTRACE_O_TRACEVFORK | \
                                                PTRACE_O_TRACEEXIT | PTRACE_O_TRACEVFORKDONE );
    siginfo_t               tracee_siginfo;
    bool                    is64bit = true;
    struct iovec            iov;
    // prstatus_t              prstat;
    struct user_regs_struct general_registers;

    /* We prepare the ptrace_options set */
    while (1)
    {
        switch (this->state)
        {
            case state_t::attach:
            {
                /* Takes control of the tracee with PTRACE_SEIZE, then stops the tracee w/ PTRACE_INTERRUPT */
                ptrace_wr(PTRACE_SEIZE);
                ptrace_wr(PTRACE_INTERRUPT);
                /* We use waitpid to wait for the seizing to finish */
                waitpid(this->pid, nullptr, __WALL);
                ptrace_wr(PTRACE_SETOPTIONS, nullptr, (int*)ptrace_options);
                this->state = next_syscall;
                break ;
            };
            case state_t::next_syscall:
            {
                /* We restart the stopped tracee as for PTRACE_CONT, stop at the next instruction / syscall / signal */
                ptrace_wr(PTRACE_SYSCALL);
                // this->state = inspect_tracee;
                this->state = wait_child;
                break ;
            };
            case state_t::inspect_tracee:
            {
                /**
                 * We read the tracee's registers. addr specifies the type of registers to be read.
                */
                // NT_PRSTATUS gets the general purpose registers, excluding the floating-point and/or vector registers.
                iov.iov_base = &general_registers;
                iov.iov_len = sizeof(general_registers);
                ptrace_wr(PTRACE_GETREGSET, (void*)NT_PRSTATUS, (void*) &iov);
                if (sizeof(general_registers) == 8)
                {
                    is64bit = true;
                }
                else if (sizeof(general_registers) == 4)
                {
                    is64bit = false;
                }
                // struct user_regs_struct* registers;
                // registers = (user_regs_struct*)prstat.pr_reg; // The general purpose registers of the process.
                // strace::print_gpregs(registers);
                std::cout << "Printing RAX[" << general_registers.orig_rax << "]" <<std::endl;
                // memcpy(&registers, prstat.pr_reg, sizeof(user_regs_struct));
                /* We copy a siginfo_t from the tracee to our siginfo struct, to get the signal information */
                ptrace_wr(PTRACE_GETSIGINFO, nullptr, &tracee_siginfo);
                this->state = next_syscall;
                break ;
            };
            case state_t::wait_child:
            {
                int wstatus = -1;
                /* We wait for the process to properly stop, to ensure synchronization */
                std::cout << "waiting..." << std::endl;
                
                waitpid(this->pid, &wstatus, __WALL);

                // TODO: Make a nice and clean convenient wrapper for this.
                if (wstatus>>8 == (SIGTRAP | PTRACE_EVENT_EXIT<<8))
                {
                    /* PTRACE_O_TRACEEXIT*/
                }
                if (wstatus>>8 == (SIGTRAP | (PTRACE_EVENT_FORK<<8)))
                {
                    /* PTRACE_O_TRACEFORK */
                }
                if (wstatus>>8 == (SIGTRAP | (PTRACE_EVENT_VFORK<<8)))
                {
                    /* PTRACE_O_TRACEVFORK */
                }
                if (wstatus>>8 == (SIGTRAP | (PTRACE_EVENT_EXEC<<8)))
                {
                    /* PTRACE_O_TRACEEXEC */
                }
                if (wstatus>>8 == (SIGTRAP | (PTRACE_EVENT_CLONE<<8)))
                {
                    /* PTRACE_O_TRACECLONE */
                }
                if (wstatus>>8 == (SIGTRAP | (PTRACE_EVENT_VFORK_DONE<<8)))
                {
                    /* PTRACE_O_VFORKDONE*/
                }
                if (WIFSTOPPED(wstatus))
                {
                        std::cout << "WIFSTOPPED" << std::endl;

                    // stopped. subprocess stopped successfully.
                    // exit(0);
                }
                else
                {
                    std::cout << "No matching statements" << std::endl;

                    // Other reasons, probably error territory.
                    exit(1);
                }
                this->state = inspect_tracee;
                break ;
            }
            default:
            {
                std::cout << "How the hell did it reach this branch?" << std::endl;
                assert(false);
                /* Default case should never be reached */
            };
        }
    }
}


void        strace::fill_syscall_map(const char* const& target_file)
{
    std::ifstream stream(target_file);
    // Setup the regular expression.
    const std::regex pattern(R"(__NR_(\S+)\s+(\d+))");
    // Create the match object for the regex
    std::smatch match;
    if (stream.is_open() == false)
    {
        stream.open(target_file);
        if (stream.is_open() == false)
            return ;
    }
    std::string line;
    while (std::getline(stream, line))
    {
        if (std::regex_match(line, match, pattern))
        {
            /**
             * If the line matches the expression, we want to insert the syscall with the corresponding number into the map.
             * @match[1] = syscall name.
             * @match[2] is the syscall number in ascii, therefore we must convert it with stoi.
             * */ 
            strace::x64_syscalls.insert({std::stoi(match[2]), match[1]});
        }
    }
    stream.close();
    return ;
}

// TODO: Add error messages to init x64 / 86 maps
// TODO: Change the if branch into a helper function, that takes a reference to the map & of the stream

/**
 * Finds the x64 header and initializes the syscall map if found.
 * When not found, leaves the map empty, which is acceptable.
 * */
void        strace::init_x64_map()
{
    struct stat sbuf;
    // Set distribution dependent targets, only using default paths.
    const char * const mint_target = "/usr/include/x86_64-linux-gnu/asm/unistd_64.h";
    const char * const ubuntu_target = "/usr/include/x86_64-linux-gnu/asm/unistd_64.h";

    if (stat(mint_target, &sbuf) != -1)
        fill_syscall_map(mint_target);
    else if (stat(ubuntu_target, &sbuf) != -1)
        fill_syscall_map(ubuntu_target);
    /* If no header can be found, we leave the map empty */
    return ;
}

/* Finds the x84 header and initializes the syscall map if found */
void    strace::init_x86_map()
{
    struct stat sbuf;
    // Set distribution dependent targets, only using default paths.
    const char * const mint_target = "/usr/include/x86_64-linux-gnu/asm/unistd_32.h";
    const char * const ubuntu_target = "/usr/include/x86_64-linux-gnu/asm/unistd_32.h";

    if (stat(mint_target, &sbuf) != -1)
        fill_syscall_map(mint_target);
    else if (stat(ubuntu_target, &sbuf) != -1)
        fill_syscall_map(ubuntu_target);
    /* If no header can be found, we leave the map empty */
    return ;
}

void         strace::init_syscall_maps()
{
    strace::init_x64_map();
    // strace::init_x86_map();
    strace::syscall_map_initialized = true;
}

void		strace::start()
{
    if (syscall_map_initialized == false)
        init_syscall_maps();
	this->pid = fork();
    if (this->pid == -1)
        log_exit1("Forking failed");
    if (this->pid != 0)
    {
        /* We engage the main loop */
        this->state = state_t::attach;
        this->run_state_machine();
    }
    if (pid == 0)
    {
        char** argv = construct_argv();
        if (execv(exec_path.c_str(), (char *const*)argv) == -1)
        {
            log_exit1("Execv failed");
            delete[] argv;
            exit(0);
        }
    }
}

// strace::strace(const strace& rhs);

// strace::strace(strace&& rhs) {}
// strace::~strace() {}


// Assignment overloads.
// strace& strace::operator=(const strace& rhs);