1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:If we called execvp directly in the current process, the current shell process would be replaced by the new program. This means the shell would terminate, and the user wouldn't be able to enter any more commands. By using fork, we create a child process that is a copy of the shell. The execvp call then replaces the child process's image with the new program, while the parent shell process continues to run. The fork provides the ability to run a new program without terminating the current shell.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:If fork() fails, it returns a negative value. This typically occurs when the system lacks sufficient resources (e.g., memory or process table entries). In my implementation, I check the return value of fork(). If it's negative, I use perror("fork") to print an error message describing the failure and return -1, indicating an error.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:execvp() searches for the command to execute in the directories listed in the PATH environment variable. The PATH variable is a colon-separated list of directories. When execvp() is called, it iterates through these directories, looking for an executable file with the given command name.
    
4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:The purpose of calling wait() in the parent process is to wait for the child process to terminate. It allows the parent to collect the exit status of the child. If we didn't call wait(), the child process would become a zombie process after it terminates. Zombie processes consume system resources, even though they are no longer actively running

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:WEXITSTATUS() is a macro that extracts the exit status of a child process that terminated normally. This provides information about whether the child process succeeded or failed. It's important because it allows the parent process to know the outcome of the child process's execution

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:My implementation of parse_command_line handles quoted arguments by tracking whether the parser is currently inside a quoted string. When a quote character (" or ') is encountered, the parser toggles the in_quotes flag. Spaces within quoted strings are treated as part of the argument, not as argument separators. This is necessary because it allows users to pass arguments that contain spaces without them being interpreted as separate arguments

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:Compared to the previous assignment, I removed the logic for handling pipes and multiple commands in a single line. I focused on parsing a single command line into a cmd_buff_t structure. The main challenge was adapting the existing parsing logic to work with the new cmd_buff_t structure, which required refactoring some parts of the code

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:Signals are used to notify a process of an event. They are a form of limited interprocess communication (IPC) that allows one process to send a notification to another. Signals are different from other IPC mechanisms (like pipes, shared memory, or message queues) because they are asynchronous and have a limited amount of data (just the signal number). Signals are typically used to interrupt or terminate processes, report errors, or notify processes of events.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:SIGINT (Interrupt): Sent when the user presses Ctrl+C. It's typically used to interrupt or terminate a running program.
SIGTERM (Terminate): A generic signal used to request the termination of a process. It's the default signal sent by the kill command.
SIGKILL (Kill): A signal that immediately terminates a process. It cannot be caught or ignored.


 

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:When a process receives SIGSTOP, it is immediately stopped (paused). SIGSTOP cannot be caught or ignored. This is because it provides a reliable way for system administrators or debuggers to halt a process, regardless of its current state.
