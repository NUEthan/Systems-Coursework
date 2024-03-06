#ifndef TOKENS_H
#define TOKENS_H

void init_tokens(void);
int get_tokens(char *line);
void free_tokens(void);
char** get_global_tokens(void);
void set_prev_command(const char *cmd);
char *get_prev_command(void);

#endif
