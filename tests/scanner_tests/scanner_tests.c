#include "../test_utils.h"
#include "../../scanner.h"
#include <stddef.h>
#include <stdio.h>

int main() {
    enum TokenType tok; 
    int max = 1000;
    
    while (1) {
    // {
        char * source = NULL;
        int source_len = 0;
        read_file_stdin(&source, &source_len);
        if (source_len == 0) {
            break;
        }

        int count = 0;
        lexer_t * lex = create_lexer(source, source_len);

        while (++count < max && tok != TOKEN_EOF) {
            tok = take_token(lex)->token_type;
            char * str_token = token_to_str(tok);
            printf("%s ", str_token);
            if (count % 15 == 14) 
                { printf("\n"); }
        }
    }

    return 0;
}