#include <regex.h>


typedef enum {
    ID, NUM, ASSIGN, SEMI, LPAR, RPAR,
    LWING, RWING, LBRACKET, RBRACKET,
    ADD, SUB, MUL, DIV, MOD, EQ, NEQ,
    LT, LEQ, GT, GEQ, IF, WHILE, WHITESPACE,
    NEWLINE, ELSE
    
    VOID,
} token_id_enum;



typedef struct {
    int token_id;
    int line;
    int col;

    char * str;         // union for str + value (?)
    int value;
} token_t;


typedef struct {
    regex_t regex;
    regmatch_t * m;
    int line;
    int col;
} lexer_t;




lexer_t init_lexer(regex_t regex, regmatch_t * m, int line, int col);

token_t * init_token(int token_id, int line, int col, char * str, int value);

int peak_token(char ** str, int lookahead, lexer_t lex);

token_t * take_token(char ** str, int expected, lexer_t * lexer);

































