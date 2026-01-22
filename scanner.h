#ifndef SCANNER_H
#define SCANNER_H

#define MAX_LEXEME_LENGTH 64

enum TokenType {
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
    TOKEN_LOG_NOT,
    TOKEN_LOG_AND,
    TOKEN_LOG_OR,
    TOKEN_BIT_NOT,
    TOKEN_BIT_AND,
    TOKEN_BIT_OR,
    TOKEN_BIT_XOR,
    TOKEN_DEREF,
    TOKEN_ADDRESSOF,
    TOKEN_I32,
    TOKEN_BOOL,
    TOKEN_PASS,
    TOKEN_IF, 
    TOKEN_ELSE, 
    TOKEN_RETURN, 
    TOKEN_WHILE, 
    TOKEN_STRUCT, 
    TOKEN_ID, 
    TOKEN_NUM, 
    TOKEN_EXPONENT, 
    TOKEN_WHITESPACE, 
    TOKEN_NEWLINE, 
    TOKEN_EOF,
    TOKEN_ERROR,
    TOKEN_NONE,
};


typedef struct {
    enum TokenType token_type;
    int col;
    int row;
    char * lexeme;
    int value;
} token_t;

typedef struct {
    token_t ** data;
    int length;
    int write_i;
    int read_i;
} tok_ringbuf_t;

typedef struct {
    char * source;
    int source_len;
    int take_i;
    int col;
    int row;
    tok_ringbuf_t peaked;
} lexer_t;

char * token_to_str(enum TokenType token_id);

lexer_t * create_lexer(char * source, int source_len);

token_t * create_token(enum TokenType token_type, int row, int col, char * lexeme, int value);

enum TokenType peak_token(lexer_t * lex);

enum TokenType peak_n_tokens(int lookahead, lexer_t * lex);

void skip_token(lexer_t * lex, enum TokenType skipped);

token_t * take_token(lexer_t * lexer);


#endif 