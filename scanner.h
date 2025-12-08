#ifndef SCANNER_H
#define SCANNER_H


#include <regex.h>


enum TokenType {
    NONE,
    TOKEN_SEMI, 
    TOKEN_ASSIGN, 
    TOKEN_COMMA,
    TOKEN_LPAR, 
    TOKEN_RPAR, 
    TOKEN_LWING, 
    TOKEN_RWING, 
    TOKEN_LBRACKET, 
    TOKEN_RBRACKET, 
    TOKEN_ADD, 
    TOKEN_SUB, 
    TOKEN_MUL, 
    TOKEN_DIV, 
    TOKEN_MOD, 
    TOKEN_EQ, 
    TOKEN_NEQ, 
    TOKEN_LT, 
    TOKEN_LEQ, 
    TOKEN_GT, 
    TOKEN_GEQ, 
    TOKEN_IF, 
    TOKEN_ELSE, 
    TOKEN_RETURN, 
    TOKEN_WHILE, 
    TOKEN_ID, 
    TOKEN_NUM, 
    TOKEN_EXPONENT, 
    TOKEN_WHITESPACE, 
    TOKEN_NEWLINE, 
};


typedef struct {
    enum TokenType token_type;
    int line;
    int col;
    
    char * str; 
    int value;
} token_t;


typedef struct {
    char * file_text;
    regex_t regex;
    regmatch_t * match;
    int rule_count;
    int line;
    int col;
} lexer_t;


extern const char * REGEX_RULES;
extern const int NUMBER_OF_TOKENS;

char * token_to_str(enum TokenType token_id);

lexer_t * init_lexer(char * file_text, regex_t regex, regmatch_t * m);

token_t * init_token(enum TokenType token_type, int line, int col, char * str, int value);

enum TokenType peak_token(lexer_t * lex);

enum TokenType peak_n_tokens(int lookahead, lexer_t * lex);

token_t * take_token(lexer_t * lexer);


#endif 