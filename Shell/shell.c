#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "tokens.h"
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>

char **find_special_token(char **tokens, const char *special);  //Forward declarating & clarity

//==== Execute commands via I/O redirection ====//
int execute_with_redirect(char **tokens, char direction) {
    char **filename_token = tokens;
    while (*filename_token && **filename_token != direction) filename_token++;

    if (!*filename_token) return -1; //If not found, return error

    *filename_token = NULL;
    char *filename = *(filename_token + 1);  //Split command & name

    //Create child to execute cmd
    pid_t pid = fork();
    if (pid == 0) {
        //Child executes cmd & redirect I/O
        int fd;
        if (direction == '>') {
	    //open file(writing/shorten)
            fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            //redirect output to file
            dup2(fd, STDOUT_FILENO);
        } else if (direction == '<') {
            //open file(reading)
            fd = open(filename, O_RDONLY);
            //redirect input to file
            dup2(fd, STDIN_FILENO);
        }

        //If file fails to open, error
        if (fd == -1) {
            perror("open failed");
            exit(EXIT_FAILURE);
        }

	//close since no longer needed
        close(fd);
        //execute cmd
        execvp(tokens[0], tokens); // Make sure 'tokens' is null-terminated
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
	//Parents wait for child to finish
        waitpid(pid, NULL, 0);
    }

    return 0;
}

//==== Execute combinational cmds (pipes 2 commands, with the ability to redirect) ====//
int execute_with_pipe_and_redirect(char **tokens) {
    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) {
        perror("pipe");
        return -1;
    }
    char **pipe_token = find_special_token(tokens, "|");
    if (pipe_token == NULL) {    // Split tokens around "|"
        fprintf(stderr, "Syntax error: expected a pipe '|' symbol.\n");
        return -1;
    }
    char **left_command = tokens; // Left side of the pipe
    char **right_command = pipe_token + 1;
    *pipe_token = NULL; // Termonate L command
    char **redirect_in_token = find_special_token(left_command, "<");
    if (redirect_in_token) {  // Redirect input if(input redirection on L of pipe)
        *redirect_in_token = NULL; // Remove the redirection token for execvp
    }
    char **redirect_out_token = find_special_token(right_command, ">");
    if (redirect_out_token) {  // Redirect output if(output redirection on R of pipe)
        *redirect_out_token = NULL; // Remove the redirection token for execvp
    }
    pid_t pid1 = fork();
    if (pid1 == 0) {  // Child process for L pipe
        if (redirect_in_token) {
            int file_in = open(*(redirect_in_token + 1), O_RDONLY);
            if (file_in < 0) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(file_in, STDIN_FILENO);
            close(file_in);
        }
        dup2(pipe_fds[1], STDOUT_FILENO);
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        execvp(left_command[0], left_command);
        perror("execvp left command");
        exit(EXIT_FAILURE);
    }
    pid_t pid2 = fork();
    if (pid2 == 0) {  // Child process for the right side of the pipe
        dup2(pipe_fds[0], STDIN_FILENO);
        close(pipe_fds[0]);
        close(pipe_fds[1]);
	    
        if (redirect_out_token) {
            int file_out = open(*(redirect_out_token + 1), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (file_out < 0) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(file_out, STDOUT_FILENO);
            close(file_out);
        }
        execvp(right_command[0], right_command);
        perror("execvp right command");
        exit(EXIT_FAILURE);
    }
    close(pipe_fds[0]);  // Parent process closes the pipe and waits for children
    close(pipe_fds[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    return 0;
}

//==== Helper function: Finds special token ("|", ">", "<", etc) in tokens array ====//
char **find_special_token(char **tokens, const char *special) {
    for (int i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], special) == 0) {
            return &tokens[i];
        }
    }
    return NULL;
}

//==== Execute cmds seperated by ";" (sequentially) ====//
int execute_sequential_commands(char **tokens) {
    int command_start = 0;
    int i = 0;
    while (tokens[i] != NULL) {  // Find ";"
        if (strcmp(tokens[i], ";") == 0) {
            tokens[i] = NULL;  // Teriminate current cmd at ";"
            pid_t pid = fork(); // Fork to execute current cmd
            if (pid == 0) {
                execvp(tokens[command_start], &tokens[command_start]); // Child executes cmd
                perror("Failed to execute command");
                exit(1);
            }
            wait(NULL);  // Parent waits for child
            command_start = i + 1;  // next cmd
        }
        i++;
    }
    if (tokens[command_start]) {
        pid_t pid = fork();
        if (pid == 0) {
            execvp(tokens[command_start], &tokens[command_start]);
            perror("Failed to execute command");
            exit(1);
        }
        wait(NULL);
    }
    return 0;
}

//==== Execute cmds connected by pipe ====//
int execute_with_pipe(char **tokens) {
    char **right_command = tokens;
    while (*right_command && **right_command != '|') right_command++;  // Finds "|" in token array
    if (!*right_command) return -1;  // If "|" doesn't exist, return error
    *right_command = NULL; // Terminate L cmd
    right_command++;  // go to start of R cmd
    int pipe_fd[2]; // Create pipe [0]Reading, [1]Writing
    pipe(pipe_fd);

    if (fork() == 0) {  // Fork to execute L side
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);
        execvp(tokens[0], tokens);
        perror("Failed to execute command");
        exit(1);
    }
    if (fork() == 0) {  // Fork to execute R side
        close(pipe_fd[1]);
        dup2(pipe_fd[0], STDIN_FILENO);
        close(pipe_fd[0]);
        execvp(right_command[0], right_command);
        perror("Failed to execute command");
        exit(1);
    }
    close(pipe_fd[0]); // Close all ends
    close(pipe_fd[1]); // Close all ends
    wait(NULL); // wait for child to finish
    wait(NULL);
    return 0;
}

//==== Execute basic cmds ====//
int execute_simple_command(char **tokens) {
    if (strcmp(tokens[0], "cd") == 0) {  // Handles'cd'cmd
        if (tokens[1] == NULL) {
            fprintf(stderr, "cd: expected argument\n");
            return -1;
        }
        if (chdir(tokens[1]) != 0) {
            perror("cd failed");
            return -1;
        }
        return 0;
    }
    if (strcmp(tokens[0], "pwd") == 0) {  // Handles 'pwd'cmd
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("pwd failed");
            return -1;
        }
        return 0;
    }
    if (strcmp(tokens[0], "mkdir") == 0) {  // Handles 'mkdir' cmd
        int flags = 0;
        int argIndex = 1;
        if (tokens[argIndex] && strcmp(tokens[argIndex], "-p") == 0) {  // If '-p' flag exists
            flags |= 1; // Set flag bit for '-p'
            argIndex++;
        }
        if (tokens[argIndex] == NULL) {
            fprintf(stderr, "mkdir: missing operand\n");
            return -1;
        }
        for (; tokens[argIndex] != NULL; argIndex++) {  // Process args
            if (flags & 1) {
                if (mkdir(tokens[argIndex], S_IRWXU | S_IRWXG | S_IRWXO) != 0 && errno != EEXIST) {  // Create Parent directories
                    perror("mkdir failed");
                    return -1;
                }
            } else {  // Create single Directory
                if (mkdir(tokens[argIndex], S_IRWXU) != 0) {
                    perror("mkdir failed");
                    return -1;
                }
            }
        }
        return 0;
    }
    if (strcmp(tokens[0], "echo") == 0) {  // Handles 'e' cmd
        int interpret_escapes = 0;
        int i = 1;  // Start after "echo"
        if (tokens[i] && strcmp(tokens[i], "-e") == 0) { // Check "-e" exists
            interpret_escapes = 1;
            i++;  // Skip '-e'
        }
        for (; tokens[i] != NULL; i++) {  // Process/print rest of args
            char *arg = tokens[i];
            if (interpret_escapes) {  // If '-e' exists, handle escape sequences
                for (char *p = arg; *p; p++) {
                    if (*p == '\\' && *(p + 1)) {
                        switch (*(++p)) {
                            case 'n': putchar('\n'); break;
                            case 't': putchar('\t'); break;
                            case '\\': putchar('\\'); break;
                            default: putchar('\\'); putchar(*p); break;  // Print unrecognized as is
                        }
                    } else {
                        putchar(*p);
                    }
                }
            } else {  // If "-e" doesn't exist, print as is
                fputs(arg, stdout);
            }
            if (tokens[i + 1] != NULL) {  // Print space between args, excluding last arg
                putchar(' ');
            }
        }
        putchar('\n');
        return 0;
    } else {  // Execute other cmds(external)
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork failed");
            return -1;
        }
        if (pid == 0) {  // Child process executes cmd
            execvp(tokens[0], tokens);
            perror(tokens[0]);  // If execvp returns --> failed :(
            exit(EXIT_FAILURE);
        } else {
            int status;
            waitpid(pid, &status, 0);  // Parent process waits for child to finish
            return status;  // Return status of child process
        }
    }
}

//==== Determines how to execute cmds ====//
int shell_run(char **tokens) {
    // Iterate over tokens to execute sequential commands first.
    for (int i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], ";") == 0) {
            tokens[i] = NULL;  // Terminate cmd
            int pid = fork();  // Execute cmd
            if (pid == 0) { // Child process 
                execvp(tokens[0], tokens);
                perror("execvp");
                exit(EXIT_FAILURE);
            } else if (pid < 0) {  // Child process
                perror("fork");
                return -1;
            }
            waitpid(pid, NULL, 0);  // Parent process waits for the child to complete
            tokens += i + 1;  // Process the next command
            i = -1; // Reset index for the next iteration.
        }
    }
    char **pipe_token = find_special_token(tokens, "|");
    char **redirect_in_token = find_special_token(tokens, "<");
    char **redirect_out_token = find_special_token(tokens, ">");

    if (pipe_token != NULL) { // After handling sequential commands, check for pipes or redirections.
        if (redirect_in_token != NULL || redirect_out_token != NULL) {  // Check for redirection in conjunction with a pipe.
            return execute_with_pipe_and_redirect(tokens);
        } else {
            return execute_with_pipe(tokens);
        }
    } else if (redirect_in_token != NULL || redirect_out_token != NULL) {  // Handle redirection without a pipe.
        char direction = **(redirect_in_token ? redirect_in_token : redirect_out_token);
        return execute_with_redirect(tokens, direction);
    }
    return execute_simple_command(tokens);  // If no special tokens are found, execute a simple command.
}

//==== Where execution begins ====//
int main(int argc, char **argv) {
    init_tokens();
    char input[256]; // Buffer(input)
    printf("Welcome to mini-shell.\n");

    while (1) {  // Main loop
        printf("shell $ ");  // Prompt user
        fflush(stdout);  // Ensure "shell $ " got printed immediately

        if (fgets(input, sizeof(input), stdin) == NULL) {  // Read input, Exits on end of file
            break; // Exit on EOF
        }
        input[strcspn(input, "\n")] = 0;  // Remove trailing newline

        if (input[0] == '\0') {  // Skip empty input
            continue;
        }
        get_tokens(input);
        char **tokens = get_global_tokens();  // tokenize input

        if (tokens[0]) {  // Process tokens & handle built-in cmds
            if (strcmp(tokens[0], "exit") == 0) {  //If cmd = "exit", terminate shell
                printf("Bye bye.\n");
                break;
            } else if (strcmp(tokens[0], "prev") == 0) {  // Handle "prev" cmd
                char *prev_cmd = get_prev_command();
                if (prev_cmd && *prev_cmd && strcmp(prev_cmd, "prev") != 0) {
                    get_tokens(prev_cmd);
                    char **prev_tokens = get_global_tokens();
                    shell_run(prev_tokens);
                    free_tokens(); // Clear tokens after executing 'prev'
                }
            } else {
                set_prev_command(input);
                shell_run(tokens);  // Execute cmd 
            }
        }
        free_tokens(); // Free tokens after each iteration
    }
    free_tokens(); // Final clean
    return 0;
}
