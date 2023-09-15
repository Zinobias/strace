#include "../inc/strace.hpp"
#include "../inc/io_utils.hpp"
#include <filesystem>
#include <unistd.h>
#include <linux/types.h>
#include <sys/stat.h>
#include <wait.h>
#include "../inc/strace.class.hpp"
#include <vector>
#include <limits>
#include <limits.h>

/* We split the PATH env into multiple paths, in case applicable */
const std::vector<std::string> split_path_env() 
{
    std::vector<std::string> paths;
    const char* path_env = getenv("PATH");
    if (path_env == nullptr)
    {
        char *cw = nullptr;
        if ((cw = getcwd(nullptr, PATH_MAX)) == nullptr)
            log_exit1("getcwd error.");
        paths.push_back(std::string(cw));
        free(cw);
        return paths;
    }
    std::string path(path_env);
    size_t start = 0, end = std::string::npos;
    do {
        end = path.find(':', start);
        paths.push_back(path.substr(start, (end == std::string::npos ? path.length() : end) - start));
        start = end + 1;
    } while (end != std::string::npos);
    return paths;
}

/**
 * If the file exists, has any exec bit set & is a regular file. We deem it valid for strace.
*/
static bool        file_is_valid(const char* file_path) 
{
    struct stat sbuf;
    int stat_ret = stat(file_path, &sbuf);
    const int &mode = sbuf.st_mode;

    bool    file_exists = stat_ret != -1;
    bool    is_executable = mode & (S_IXGRP | S_IXOTH | S_IXUSR);
    bool    is_regular = S_ISREG(mode);
    return (file_exists && is_executable && is_regular);
}

/**
 * We look for a matching $PATH, we use the first match.
 * We do not check for ownership, we let the `exec` call handle it internally.
*/
const  std::string find_exec(const char* execPath)
{
    if (execPath[0] == '/')
    {
        /* Path is absolute */
        if (file_is_valid(execPath))
            return (std::string(execPath));
        log_exit1("No matching executable found.");
    }
    std::vector<std::string> paths = split_path_env();
    for (auto& curPath : paths)
    {
        std::string full_path = std::string(curPath + '/' + execPath).c_str();
        if (file_is_valid(full_path.c_str()))
        {
            return (std::string(full_path));
        }
    }
    log_exit1("No matching executables found.");
    // exit(1);
}

int main ( int argc, char* argv[], char *envp[])
{
    /* We don't handle flags, so any use of flags has undefined behaviour */
    // TODO: Check whether we implement the -p flag. I probably will.
    // TODO: Parse flags for the bonus.
    if (argc < 2)
        log_exit1("strace: must have PROG [ARGS] or -p PID");
    /* Check if path is absolute or relative */
	const  std::string	exec_path = find_exec(argv[1]);
    std::cout << "Exec found is: {" << exec_path << "}" << std::endl;
    // We pass the arguments without the executables. And adjust argc accordingly.
    strace s(argc - 2, argv + 2, exec_path);
    s.start();
    return 0;
}

// TODO: parse flags.