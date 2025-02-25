#include "lab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pwd.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

/*
 * get_prompt:
 *  - Purpose: Returns the shell prompt string.
 *      * It checks the environment variable specified by 'env' (usually "MY_PROMPT").
 *      * If the variable exists, its value is duplicated (using strdup) and returned.
 *      * If not, a default prompt "shell>" is duplicated and returned.
 *  - Note: The returned string is allocated on the heap, so the caller is responsible for freeing it.
 */
char *get_prompt(const char *env) {
    char *env_val = getenv(env);
    if (env_val != NULL) {
        return strdup(env_val);
    } else {
        return strdup("shell>");
    }
}

/*
 * change_dir:
 *  - Purpose: Changes the current working directory.
 *  - Parameters:
 *      * dir: An array of strings where dir[0] is expected to be "cd" and dir[1] is the target directory.
 *      * If no directory argument is provided (i.e., dir[1] is NULL), it attempts to use the HOME directory.
 *      * If the HOME environment variable isn't set, it uses getpwuid/getuid to retrieve the home directory.
 *      * It then calls chdir() to change to the specified directory.
 *      * If chdir() fails, an error message is printed.
 *  - Returns: 0 on success, -1 on error.
 */
int change_dir(char **dir) {
    char *target;
    if (dir[1] == NULL) {
        // No target directory provided; use the HOME directory.
        target = getenv("HOME");
        if (target == NULL) {
            // If HOME isn't set, try to get the user's home directory via getpwuid.
            struct passwd *pw = getpwuid(getuid());
            if (pw) {
                target = pw->pw_dir;
            } else {
                perror("change_dir: cannot determine home directory");
                return -1;
            }
        }
    } else {
        // Use the provided target directory.
        target = dir[1];
    }
    // Attempt to change to the target directory.
    int ret = chdir(target);
    if (ret != 0) {
        perror("change_dir");
    }
    return ret;
}

/*
 * cmd_parse:
 *  - Purpose: Splits a command line string into tokens.
 *      * Allocates an initial array (tokens) to store pointers to token strings.
 *      * Duplicates the input line so the original isn't modified.
 *      * Uses strtok() to split the string on whitespace (spaces, tabs, newlines, etc.).
 *      * Each token is duplicated (using strdup) and stored in the tokens array.
 *      * If the number of tokens exceeds the initial buffer size, the buffer is resized.
 *      * The tokens array is terminated with a NULL pointer.
 *  - Returns: A NULL-terminated array of tokens.
 *  - Note: Memory for both the array and each token is allocated on the heap. Use cmd_free() to free it.
 */
char **cmd_parse(char const *line) {
    int bufsize = 64;  // Initial allocation for tokens
    char **tokens = malloc(bufsize * sizeof(char *));
    if (!tokens) {
        fprintf(stderr, "cmd_parse: allocation error\n");
        exit(EXIT_FAILURE);
    }
    // Duplicate the input line to avoid modifying the original string.
    char *line_copy = strdup(line);
    char *token;
    int position = 0;
    // Split the string into tokens based on whitespace.
    token = strtok(line_copy, " \t\r\n");
    while (token != NULL) {
        tokens[position] = strdup(token);  // Allocate memory for the token
        position++;
        // If the buffer is full, increase its size.
        if (position >= bufsize) {
            bufsize *= 2;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens) {
                fprintf(stderr, "cmd_parse: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, " \t\r\n");
    }
    tokens[position] = NULL;  // Terminate the array with NULL
    free(line_copy);          // Free the duplicated input string
    return tokens;
}

/*
 * cmd_free:
 *  - Purpose: Frees memory allocated for the tokens by cmd_parse.
 *      * Iterates through the tokens array and frees each token.
 *      * Frees the tokens array itself.
 */
void cmd_free(char **line) {
    if (line) {
        for (int i = 0; line[i] != NULL; i++) {
            free(line[i]);  // Free each individual token
        }
        free(line);  // Free the tokens array
    }
}

/*
 * trim_white:
 *  - Purpose: Removes leading and trailing whitespace from a string.
 *      * Moves the pointer forward to skip any leading whitespace.
 *      * Finds the end of the string and moves backward to remove trailing whitespace.
 *      * Inserts a null terminator after the last non-whitespace character.
 *  - Note: The function modifies the string in place and returns a pointer to the trimmed string.
 */
char *trim_white(char *line) {
    if (line == NULL) return line;
    // Skip leading whitespace.
    while (*line && isspace((unsigned char)*line)) {
        line++;
    }
    // If the string is empty after trimming, return it.
    if (*line == '\0') {
        return line;
    }
    // Find the end of the string.
    char *end = line + strlen(line) - 1;
    // Move backward over any trailing whitespace.
    while (end > line && isspace((unsigned char)*end)) {
        end--;
    }
    // Terminate the string after the last non-whitespace character.
    *(end + 1) = '\0';
    return line;
}

/*
 * do_builtin:
 *  - Purpose: Checks if a command is a built-in command (e.g., exit, cd, history) and executes it.
 *      * If the command is "exit":
 *            - During tests (when SKIP_EXIT is set to "1"), it simply returns true.
 *            - Otherwise, it calls exit(0) to terminate the shell.
 *      * If the command is "cd", it calls change_dir() to change the directory.
 *      * If the command is "history", it returns true (a full implementation might print command history).
 *  - Returns: true if the command is a built-in, false otherwise.
 */
bool do_builtin(struct shell *sh, char **argv) {
    (void)sh;  // Unused parameter; suppresses compiler warnings.
    if (argv == NULL || argv[0] == NULL) {
        return false;
    }
    if (strcmp(argv[0], "exit") == 0) {
        // Check if SKIP_EXIT is set to "1" to bypass exit (used during testing).
        char *skip_exit = getenv("SKIP_EXIT");
        if (skip_exit && strcmp(skip_exit, "1") == 0) {
            return true;  // Indicate the command was handled without terminating.
        }
        exit(0);  // In normal operation, terminate the shell.
    } else if (strcmp(argv[0], "cd") == 0) {
        change_dir(argv);  // Change the current working directory.
        return true;
    } else if (strcmp(argv[0], "history") == 0) {
        // For a full shell, you might print the command history here.
        return true;
    }
    return false;  // Not a built-in command.
}

/*
 * sh_init:
 *  - Purpose: Initializes the shell structure.
 *      * Sets the shell's terminal file descriptor to standard input (STDIN_FILENO).
 *      * Checks if the shell is running interactively using isatty().
 *      * If interactive and SKIP_TC is not set to "1", it sets up terminal control:
 *            - Puts the shell in its own process group.
 *            - Gets the terminal's current attributes.
 *            - Sets the shell's process group as the foreground process group.
 *      * Sets the shell's prompt using the MY_PROMPT environment variable (or a default).
 */
void sh_init(struct shell *sh) {
    if (!sh) return;
    // Set the terminal descriptor (standard input)
    sh->shell_terminal = STDIN_FILENO;
    // Check if the shell is running interactively (i.e., attached to a terminal)
    sh->shell_is_interactive = isatty(sh->shell_terminal);
    /* Check if SKIP_TC is set to "1" to bypass terminal control (useful during testing) */
    char *skip_tc = getenv("SKIP_TC");
    if (sh->shell_is_interactive && (!skip_tc || strcmp(skip_tc, "1") != 0)) {
        // Set the shell's process group ID to its own PID
        sh->shell_pgid = getpid();
        // Put the shell in its own process group
        if (setpgid(sh->shell_pgid, sh->shell_pgid) < 0) {
            perror("sh_init: Couldn't put the shell in its own process group");
            exit(1);
        }
        // Get the terminal's current attributes
        tcgetattr(sh->shell_terminal, &sh->shell_tmodes);
        // Set the shell's process group as the foreground process group
        tcsetpgrp(sh->shell_terminal, sh->shell_pgid);
    }
    // Set the shell prompt based on the MY_PROMPT environment variable (or default to "shell>")
    sh->prompt = get_prompt("MY_PROMPT");
}

/*
 * sh_destroy:
 *  - Purpose: Cleans up the shell structure.
 *      * Frees the prompt string if it was allocated.
 *      * Sets the prompt pointer to NULL to prevent dangling references.
 */
void sh_destroy(struct shell *sh) {
    if (!sh) return;
    if (sh->prompt) {
        free(sh->prompt);   // Free the dynamically allocated prompt
        sh->prompt = NULL;  // Avoid leaving a dangling pointer
    }
}

/*
 * parse_args:
 *  - Purpose: A placeholder function for parsing command line arguments.
 */
void parse_args(int argc, char **argv) {
    (void)argc;
    (void)argv;
    /* No argument parsing implemented for now */
}
