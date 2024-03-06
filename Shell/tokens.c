#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "tokens.h"  //For token operations

#define BUFFER_SIZE 512  // Buffer for reading input
#define MAX_CAP 256 // Initial capacity for tokens array
#define MAX_TOKEN_SIZE 255 // Max length per token

static int tokens_size = 0; // Current # of tokens
static int tokens_capacity = MAX_CAP;  // Current capacity of tokens array
static char **tokens = NULL;  //Array of tokens (strings)
static int is_tokens_initialized = 0; // Flag for init
static char prev_command[256] = {0}; // Buffer for storing last cmd

//==== Initialize token array ====//
void init_tokens() {
    if (!is_tokens_initialized) {
        tokens = malloc(sizeof(char *) * tokens_capacity); // Allocates memory if needed
        if (!tokens) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        tokens[0] = NULL;
        is_tokens_initialized = 1;
    }
}

//==== Grow tokens arrayy if MAX_CAP is reached ====//
void grow_tokens() {
    int new_capacity = tokens_capacity + MAX_CAP;
    char** new_tokens = realloc(tokens, sizeof(char *) * new_capacity);
    if (!new_tokens) {
        fprintf(stderr, "Memory reallocation failed\n");
        exit(EXIT_FAILURE);
    }
    tokens_capacity = new_capacity;
    tokens = new_tokens;
}

//==== Check if token array reached MAX_CAP ====//
int tokens_full() {
    return tokens_size >= tokens_capacity - 1;
}

//==== Add token to array of tokens ====//
void add_token(const char* token) {
    if (tokens_full()) {
        grow_tokens();
    }
    if (tokens[tokens_size]) {  // Free existing if overwriting a token
        free(tokens[tokens_size]);
    }
    tokens[tokens_size] = strdup(token);  // Duplicate and add token to array
    if (!tokens[tokens_size]) {
        fprintf(stderr, "Memory allocation for token failed\n");
        exit(EXIT_FAILURE);
    }
    tokens[++tokens_size] = NULL;  //Terminate array after new token
}

//==== Read quoted string & Handles escape sequences ====//
int read_string(const char *input, char *buf) {
    unsigned int bytes = 0; // # bytes copied
    int escaped = 0; // if the prev = backslash
   
    while (*input && *input != '"') {   // Loop through input till " or end 
        if (escaped) {  // Handle escaped characters   
            switch (*input) {
                case 'n': *buf++ = '\n'; break; // Newline
                case '\\': *buf++ = '\\'; break; // Backslash
                default: *buf++ = *input; // Unrecognized, then copy the character
            }
            bytes++;
            escaped = 0; // Reset escaped flag
        } else if (*input == '\\') {   // Set escape flag if backslash found
            escaped = 1;
        } else {  // Normal character, copy them
            *buf++ = *input;
            bytes++;
        }
        if (!escaped) {  // Next character if not escaping
            input++;
        }
    }
    if (escaped) {  // Handle if string ends with escape char
        fprintf(stderr, "Bad input: Unescorted backslash at end of string\n");
        exit(EXIT_FAILURE);
    }
    if (*input == '"') {
        ++input; // Skip the closing quote
    }
    *buf = '\0'; // Null-terminate the string
    return bytes;
}

//==== Read token till space,newLine, tab, semicolon =====//
int read_until_space(const char *input, char *buf) {
    unsigned int bytes = 0;
    while (*input && *input != ' ' && *input != '\n' && *input != '\t' && *input != ';') { //Copy char till delimiter found
        *buf = *input;
        ++bytes;
        ++buf;
        ++input;
    }
    *buf = '\0'; //Terminate token
    return bytes;
}

//==== Parse input into token ====//
int get_tokens(char *line) {
    if (!is_tokens_initialized) {
        init_tokens();
    }
    unsigned int tokens_found = 0;
    while (*line) { // Parse each token
        while (isspace((unsigned char)*line)) {
            ++line; // Skips whitespace
        }
        if (*line == '\0') {
            break; // Break if end of line
        }
        char token_buf[MAX_TOKEN_SIZE];  // For individual tokens
        int bytes = 0;  // For token_buf
        if (*line == '"' || *line == '\'') {  // Handles special chars and quoted
            char quote_char = *line++;
            char *buf_ptr = token_buf;
            while (*line && *line != quote_char) {
                if (*line == '\\' && *(line + 1) == quote_char) {  // Skip escape char for quote
                    line++;
                } else if (*line == '\\' && *(line + 1)) {  // Handles escaped chars within quotes
                    line++; // Skip backslash
                    switch (*line) {
                        case 'n': *buf_ptr++ = '\n'; break; // newline
                        case 't': *buf_ptr++ = '\t'; break; // tab
                        default: *buf_ptr++ = *line; // literal character after backslash
                    }
                } else {
                    *buf_ptr++ = *line; // Copy other chars inside quotes
                }
                line++;
            }
            if (*line == quote_char) line++; // Skip closing quote
            *buf_ptr = '\0'; // Terminate token
        } else if (strchr("();|<>", *line)) {  // Special chars --> separate tokens
            token_buf[0] = *line++;
            token_buf[1] = '\0';
        } else {  // Read till space, special char, end of line
            bytes = read_until_space(line, token_buf);
            line += bytes;
        }
        add_token(token_buf); // Add token to array
        tokens_found++;
    }
    if (tokens_size == 1 && strcmp(tokens[0], "prev") == 0) {  // Handle "prev" command , retrieves prev command
        free_tokens();
        char *prev_cmd = get_prev_command();
        if (prev_cmd && *prev_cmd) {
            return get_tokens(prev_cmd);
        }
        return 0; // If nonexistant, return zero tokens
    }
    return tokens_found; // Return # of tokens found
}

//==== Free allocated memory for tokens =====//
void free_tokens(void) {
    for (int i = 0; i < tokens_size; i++) {
        free(tokens[i]); // Free individual tokens
    }
    free(tokens); // Free array
    tokens = NULL;
    tokens_size = 0;
    tokens_capacity = MAX_CAP; // Reset
    is_tokens_initialized = 0; // Reset
}

//==== Return token array(GLOBAL) ====/
char** get_global_tokens(void) {
    return tokens;
}

//==== Store prev cmd in a buffer(GLOBAL) ====//
void set_prev_command(const char *cmd) {
    if (cmd) {
        memset(prev_command, 0, sizeof(prev_command)); // Clear buffer --> ensures no leftovers
        snprintf(prev_command, sizeof(prev_command), "%s", cmd);  // Copy cmd into buffer --> ensures termination.
    }
}

//==== Gets previously stored cmd ====//
char *get_prev_command(void) {
    return prev_command;
}
