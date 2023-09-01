# Strace project strategy

* Adjust C++ json properties in vscode, to adjust standard intellisense uses. Currently using C++20.

* PTRACE_SYSCALL
  Restart the stopped tracee as for PTRACE_CONT, but arrange for the tracee to be stopped at the next entry to or exit from a system call, or
  after execution of a single instruction, respectively.  (The tracee will also, as usual, be stopped upon receipt of a  signal.)   From  the
  tracer's  perspective,  the tracee will appear to have been stopped by receipt of a SIGTRAP.  So, for PTRACE_SYSCALL, for example, the idea
  is to inspect the arguments to the system call at the first stop, then do another PTRACE_SYSCALL and inspect the return value of the system
  call at the second stop.  The data argument is treated as for PTRACE_CONT.  (addr is ignored.)

* PTRACE_GETREGSET
  Read the tracee's registers.  addr specifies, in an architecture-dependent way, the type of registers to be read.  NT_PRSTATUS (with numer‐
  ical  value  1)  usually results in reading of general-purpose registers.  If the CPU has, for example, floating-point and/or vector regis‐
  ters, they can be retrieved by setting addr to the corresponding NT_foo constant.  data points to a struct iovec, which describes the  des‐
  tination buffer's location and length.  On return, the kernel modifies iov.len to indicate the actual number of bytes returned.

* PTRACE_SETOPTIONS
  * PTRACE_O_EXITKILL
  * PTRACE_O_TRACECLONE
  * PTRACE_O_TRACEEXEC
  * PTRACE_O_TRACEEXIT
  * PTRACE_O_TRACEFORK
  * PTRACE_O_TRACESYSGOOD
  * PTRACE_O_TRACEVFORKDONE
  * PTRACE_O_TRACESECCOMP
  * PTRACE_O_SUSPEND_SECCOMP

* PTRACE_GETSIGINFO
  Retrieve information about the signal that caused the stop.  Copy a siginfo_t structure (see sigaction(2)) from the tracee to  the  address
  data in the tracer.  (addr is ignored.)

* PTRACE_SEIZE
  Attach to the process specified in pid, making it a tracee of the calling process.  Unlike PTRACE_ATTACH, PTRACE_SEIZE does  not  stop  the
  process.  Group-stops are reported as PTRACE_EVENT_STOP and WSTOPSIG(status) returns the stop signal.  Automatically attached children stop
  with PTRACE_EVENT_STOP and WSTOPSIG(status) returns SIGTRAP instead of having SIGSTOP signal delivered to them.  execve(2) does not deliver
  an  extra  SIGTRAP.   Only  a PTRACE_SEIZEd process can accept PTRACE_INTERRUPT and PTRACE_LISTEN commands.  The "seized" behavior just de‐
  scribed is inherited by children that are automatically attached using PTRACE_O_TRACEFORK,  PTRACE_O_TRACEVFORK,  and  PTRACE_O_TRACECLONE.
  addr must be zero.  data contains a bit mask of ptrace options to activate immediately.

  Permission to perform a PTRACE_SEIZE is governed by a ptrace access mode PTRACE_MODE_ATTACH_REALCREDS check; see below.
* PTRACE_INTERRUPT
  Stop  a  tracee.   If the tracee is running or sleeping in kernel space and PTRACE_SYSCALL is in effect, the system call is interrupted and
  syscall-exit-stop is reported.  (The interrupted system call is restarted when the tracee is restarted.)  If the tracee was already stopped
  by a signal and PTRACE_LISTEN was sent to it, the tracee stops with PTRACE_EVENT_STOP and WSTOPSIG(status) returns the stop signal.  If any
  other ptrace-stop is generated at the same time (for example, if a signal is sent to the tracee), this ptrace-stop happens.  If none of the
  above  applies  (for  example,  if  the tracee is running in user space), it stops with PTRACE_EVENT_STOP with WSTOPSIG(status) == SIGTRAP.
  PTRACE_INTERRUPT only works on tracees attached by PTRACE_SEIZE.

* PTRACE_LISTEN // See Group-stop in the man page.P&
  Restart  the  stopped  tracee,  but  prevent  it  from executing.  The resulting state of the tracee is similar to a process which has been
  stopped by a SIGSTOP (or other stopping signal).  See the "group-stop" subsection for additional information.  PTRACE_LISTEN works only  on
  tracees attached by PTRACE_SEIZE.

# TODO:

* Recursively go through all $PATH values, to find the executable.
* Confirm whether I want to implement -p
* Change the program, so that exec a paths are only constructed when neccessary, and not as a prelimary flow.

# Continue :
	find_exec, making sure file is executable.



# Bonus

* Path management.

* Manage the -c flag
-c
--summary-only
  Count time, calls, and errors for each system call and report a summary on program
  exit, suppressing the regular output.  This attempts to show system time (CPU time
  spent  running  in the kernel) independent of wall clock time.  If -c is used with
  -f, only aggregate totals for all traced processes are kept.

Steps :
1. Make sure the program uses the correct executable, using `PATH`.
2. Make sure it uses `execv` correctly.
3. Move find_execs into the class. Only construct a path one at a time, only when necessar.
4. Use ptrace to attach to the tracee, and become the tracer.
5. Stonks.

When should we walk the PATHs, and when should we just exit & error?


# STOPPED STATES
A  tracee can be in two states: running or stopped.  For the purposes of ptrace, a tracee which is blocked in a system call (such as read(2), pause(2), etc.)  is nevertheless considered to be run‐
       ning, even if the tracee is blocked for a long time.  The state of the tracee after PTRACE_LISTEN is somewhat of a gray area: it is not in any ptrace-stop (ptrace commands won't work on it, and it
       will  deliver  waitpid(2)  notifications), but it also may be considered "stopped" because it is not executing instructions (is not scheduled), and if it was in group-stop before PTRACE_LISTEN, it
       will not respond to signals until SIGCONT is received.

       There are many kinds of states when the tracee is stopped, and in ptrace discussions they are often conflated.  Therefore, it is important to use precise terms.

       In this manual page, any stopped state in which the tracee is ready to accept ptrace commands from the tracer is called ptrace-stop.  Ptrace-stops can be further subdivided  into  signal-delivery-
       stop, group-stop, syscall-stop, PTRACE_EVENT stops, and so on.  These stopped states are described in detail below.

       When the running tracee enters ptrace-stop, it notifies its tracer using waitpid(2) (or one of the other "wait" system calls).  Most of this manual page assumes that the tracer waits with:

           pid = waitpid(pid_or_minus_1, &status, __WALL);

       Ptrace-stopped tracees are reported as returns with pid greater than 0 and WIFSTOPPED(status) true.

       The __WALL flag does not include the WSTOPPED and WEXITED flags, but implies their functionality.

       Setting the WCONTINUED flag when calling waitpid(2) is not recommended: the "continued" state is per-process and consuming it can confuse the real parent of the tracee.

       Use of the WNOHANG flag may cause waitpid(2) to return 0 ("no wait results available yet") even if the tracer knows there should be a notification.