#include "../inc/strace.hpp"
#include "../inc/io_utils.hpp"
#include <filesystem>
#include <unistd.h>
#include <linux/types.h>
#include <sys/stat.h>
#include <wait.h>
// #include "../inc/strace.class.hpp"
#include <vector>

namespace fs = std::filesystem;

const std::string getCurrentWorkingDir() {
    return fs::current_path().string();
}

/* We split the PATH env into multiple paths, in case applicable */
const std::vector<std::string> split_path_env() 
{
    std::vector<std::string> paths;
    const char* path_env = getenv("PATH");
    if (path_env == NULL)
        paths.push_back(getCurrentWorkingDir());
    else
    {
        std::string path(path_env);
        size_t start = 0, end = std::string::npos;
        do {
            end = path.find(':', start);
            paths.push_back(path.substr(start, (end == std::string::npos ? path.length() : end) - start));
            start = end + 1;
        } while (end != std::string::npos);
    }
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
    std::string full_path;
    if (execPath[0] == '/')
    {
        /* Path is absolute */
        return (std::string(execPath));
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
    exit(1);
}

int main ( int argc, char* argv[], char *envp[])
{
    /* We don't handle flags, so any use of flags has undefined behaviour */
    // TODO: Check whether we implement the -p flag. I probably will.
    if (argc < 2)
        log_exit1("strace: must have PROG [ARGS] or -p PID");
    /* Check if path is absolute or relative */
	const  std::string	exec_path = find_exec(argv[1]);
    std::cout << "Exec found is: {" << exec_path << "}" << std::endl;
    // strace s(argc, argv + 1, exec_path);
    return 0;
}

// TODO: parse flags.