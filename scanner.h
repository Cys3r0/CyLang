#include <regex.h>


enum TokenType {
    NONE,
    SEMI, 
    ASSIGN, 
    COMMA,
    LPAR, 
    RPAR, 
    LWING, 
    RWING, 
    LBRACKET, 
    RBRACKET, 
    ADD, 
    SUB, 
    MUL, 
    DIV, 
    MOD, 
    EQ, 
    NEQ, 
    LT, 
    LEQ, 
    GT, 
    GEQ, 
    IF, 
    WHILE, 
    ID, 
    NUM, 
    EXPONENT, 
    WHITESPACE, 
    NEWLINE, 
};



typedef struct {
    enum TokenType token_type;
    int line;
    int col;

    char * str;         // union for str + value (?)
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

































