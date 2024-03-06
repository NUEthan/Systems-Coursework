#include <stdio.h>
#include "tokens.h"  //include header for operations with tokens
#include <assert.h>

int main(int argc, char **argv) {
    char input[256];  // Buffer(storing input)
    fgets(input, 256, stdin);  // Prompt user + input

    init_tokens();
    get_tokens(input);  // Tokenise input

    char **current_tokens = get_global_tokens();  // Gets tokens from get_tokens

    assert(current_tokens != NULL); // was getting tokens successful?

    // Iterate/Print tokens
    char **current = current_tokens;
    while (*current != NULL) {
        printf("%s\n", *current);
        ++current;
    }

    free_tokens();
    return 0;
}
