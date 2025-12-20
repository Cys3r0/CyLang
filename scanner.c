#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "scanner.h"
#include <regex.h>

#define MAX_VECTOR_LENGTH 20
#define RINGBUF_SIZE 20


// TODO:
// implement ring buffer for the peaked that self, resize when within range (could work :))
// Move all regex stuff into init_lexer
// memoize peak_n_tokens
// add a take_token_specifc, which takes expects a certain token which is passed to it (or include this in skip_token)
// add a skip_token that doesn't create a token_t object but removes form lex->file_text
// Include a python style "pass" keyword
// Create own scanner
// add EXPONENT, (LOGICAL) NOT AND OR, (BITWISE) NOT AND OR XOR BITSHIFTS, INCREMENT, DECREMENT tokens

// Useless wrapper function right now
char peak_char(char * source) {
    return source[0]; // Is this necessary?
}

char take_char(char ** source, neo_lexer_t * lex) {
    char c = lex->source[0];

    if (c == '\0') 
        lex->source = NULL;
    else {
        if (c == '\n') {
            lex->row++;    
            lex->col = 1;
        } else {
            lex->col++;
        }
        lex->source++;
    }
    
    return c;
}

int is_numeric(char c) {
    return '0' <= c <= '9';
}

int is_letter(char c) {
    return 'a' <= c <= 'z' || 'A' <= c <= 'Z' ;
}

int is_alphanumeric(char c) {
    return 'a' <= c <= 'z' || 'A' <= c <= 'Z' || '0' <= c <= '9';
}
typedef struct {
    enum TokenType type;
    int col;
    int row;
    union {
        char * lexeme;
        int value;
    }
} neo_token_t;

typedef struct {
    char * source;
    int col;
    int row;
    tok_ringbuf_t peaked;
} neo_lexer_t;

typedef struct {
    token_t ** data;
    int write_i;
    int read_i;
} tok_ringbuf_t;

int ringbuf_put(tok_ringbuf_t * rb, token_t * tok) {
    if ((rb->write_i + 1) % rb->buf_size == rb->read_i) 
        return 0;

    rb->write_i = rb->write_i+1 % rb->buf_size;
    rb->data[rb->write_i] = tok;
    return 1;
}

token_t * ringbuf_get(tok_ringbuf_t * rb) {
    if (rb->write_i == rb->read_i) 
        return NULL;

    token_t * ret_tok = rb->data[rb->read_i];
    rb->read_i = rb->read_i + 1 % rb->buf_size;
    return ret_tok;
}

token_t * ringbuf_get_n(tok_ringbuf_t * rb, int n) {
    // IMPLEMENT THIS
}

tok_ringbuf_t create_ringbuf() {
    tok_ringbuf_t rb;
    rb->write_i = 0;
    rb->read_i = 0;
    rb->data = calloc(RINGBUF_SIZE, sizeof(token_t *));
    return rb;
}


neo_lexer_t * neo_create_lexer(char * source) {
    neo_lexer_t * lex = calloc(1, sizeof(neo_lexer_t));
    lex->source = source;
    lex->col = 1;
    lex->row = 1;
    lex->peaked = calloc(1, sizeof(tok_ringbuf_t));
    return lex;
}

neo_token_t * neo_create_token(enum TokenType token_type, int line, int col, char * lexeme, int value) {
    token_t * tok = malloc(sizeof(token_t));
    tok->token_type = token_type;
    tok->line = line;
    tok->col = col;
    if (lexeme) 
        tok->lexeme = lexeme;
    else 
        tok->value = value;
    return tok;
}

neo_token_t * scan_next_tok(neo_lexer_t * lex) {
    // Current peaking with source[k] can segfault 
    enum TokenType token_type;
    char * lexeme = NULL;
    int col;
    int line;
    int end_offset = 0;
    int value = 0;
    enum TokenType tok_type;
    unsigned int valid_char = 0;
    char c = take_char(s);

    while (c == ' ' || c == '\n') {
        c = take_char(s);
    }

    if (c == ';')
        token_type = TOKEN_SEMI;
    else if (c == ',')
        token_type = TOKEN_COMMA;
    else if (c == '(')
        token_type = TOKEN_LPAR;
    else if (c == ')')
        token_type = TOKEN_RPAR;
    else if (c == '{')
        token_type = TOKEN_LWING;
    else if (c == '}')
        token_type = TOKEN_RWING;
    else if (c == '[')
        token_type = TOKEN_LBRACKET;
    else if (c == ']')
        token_type = TOKEN_RBRACKET;
    else if (c == '+')
        token_type = TOKEN_ADD;
    else if (c == '-')
        token_type = TOKEN_SUB;
    else if (c == '*')
        token_type = TOKEN_MUL;
    else if (c == '/')
        token_type = TOKEN_DIV;
    else if (c == '%')
        token_type = TOKEN_MOD;
    else if (c == '^' && lex->source[0] == '^') {
        token_type = TOKEN_EXPONENT;
        end_offset = 1;
    }
    else if (c == '>' && lex->source[0] == '=') {
        token_type =  TOKEN_GEQ; 
        end_offset = 1;
    }
    else if (c == '>')
        token_type = TOKEN_GT;
    else if (c == '<' && lex->source[0] == '=') {
        token_type =  TOKEN_LEQ; 
        end_offset = 1;
    }
    else if (c == '<')
        token_type = TOKEN_LT;
    else if (c == '=' && lex->source[0] == '=') {
        token_type = TOKEN_EQ; 
        end_offset = 1;
    }
    else if (c == '=')
        token_type = TOKEN_ASSIGN;
    // else if (c == '!') 
    //     token_type = (peak_char(s+1) == '=') ? TOKEN_ASSIGN : TOKEN_NEQ; //TOKEN_NOT
    // else if (c == '|') 
    //     token_type = (peak_char(s+1) == '|') ? TOKEN_ASSIGN : TOKEN_NEQ; //TOKEN_BIT_AND
    else if (c == 'w' 
        && lex->source[0] == 'h' 
        && lex->source[2] == 'i' 
        && lex->source[3] == 'l' 
        && lex->source[4] == 'e'
        && !is_letter(lex->source[5])) {
            token_type = TOKEN_WHILE;  
            end_offset = 4;
    }
    else if (c == 'i' 
        && lex->source[0] == 'f' 
        && !is_letter(lex->source[1])) {
            token_type = TOKEN_IF;
            end_offset = 1;
    }
    else if (c == 'e' 
        && lex->source[0] == 'l' 
        && lex->source[1] == 's' 
        && lex->source[2] == 'e' 
        && !is_letter(lex->source[3])) {
            token_type = TOKEN_ELSE;
            end_offset = 3;
    }
    else if (c == 'r' 
        && lex->source[0] == 'e' 
        && lex->source[1] == 't' 
        && lex->source[2] == 'u'
        && lex->source[3] == 'r'
        && lex->source[4] == 'n' 
        && !is_letter(lex->source[5])) {
            token_type = TOKEN_RETURN;
            end_offset = 5;
    }
    else if (is_letter(c)) {
        token_type = TOKEN_ID;
        int i = 0;
        while (is_alphanumeric(lex->source[0])) 
            i++;

        lexeme = malloc(50);
        for (size_t j = 0; j < s+i; j++) 
            lexeme[j] = s[j];
        lexeme[i] = '\0';
    }
    else if (is_numeric(c)) {
        token_type = TOKEN_NUM;
        int i = 1;
        while (is_numeric(lex->source[i])) 
            i++;

        if (is_alphanumeric(lex->source[i])) 
            printf("Incorrect numeral syntax");
        if (u >= 50)
            printf("Number has too many characters");

        char number[50];
        for (size_t j = 0; j < s+i; j++) 
            number[j] = s[j];
        number[i] = '\0';
        
        value = atoi(number); 
    }
    if (c == '\0') 
        token_type = TOKEN_EOF;

    // CHECK THORUGH IF THIS OFFSET CALC IS CORRECT
    lex->source += end_offset;

    return neo_create_token(
        token_type,
        lex->line;
        lex->col;
        lexeme,
        value,
    )
}


enum TokenType peaked_get_nth(neo_lexer_t * lex, int n) {
    tok_node_t curr = lex.peaked->head;
    for (size_t i = 1; i < n; i++) 
        curr = curr->next;

    return curr->tok
}

enum TokenType peak_n_tok(neo_lexer_t * lex, int n) {
    int to_scan = n - lex.peaked.count;
    if (to_scan > 0) 
        return peaked_get_nth(lex, to_scan);
    
    for (size_t i = 0; i < -; i++)
    {
        /* code */
    }
    
    
}


enum TokenType peak_tok(neo_lexer_t * lex) {
    
}

enum TokenType take_tok(neo_lexer_t * lex) {
    if (lex.peaked.count > 0) 
        return peaked_take(lex);
    
    return scan_next_tok(lex);
}


const int NUMBER_OF_TOKENS = TOKEN_NEWLINE;

const char * REGEX_RULES = 
    "(;)"
    "|(=)"
    "|(,)"
    "|(\\()"
    "|(\\))"
    "|(\\{)"
    "|(\\})"
    "|(\\[)"
    "|(\\])"
    "|(\\+)"
    "|(\\-)"
    "|(\\*)"
    "|(\\/)"
    "|(%)"
    "|(==)"
    "|(!=)"
    "|(<)"
    "|(<=)"
    "|(>)"
    "|(>=)"
    "|(if)"
    "|(else)"
    "|(return)"
    "|(while)"
    "|([a-zA-Z][a-zA-Z0-9]*)"
    "|([0-9]+)"
    "|(\\^\\^)"
    "|([ ]+)"
    "|(\n)";


char * token_to_str(enum TokenType token_type) {
    switch (token_type) {
        case TOKEN_SEMI: return "SEMI";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_LPAR: return "LPAR";
        case TOKEN_RPAR: return "RPAR";
        case TOKEN_LWING: return "LWING";
        case TOKEN_RWING: return "RWING";
        case TOKEN_LBRACKET: return "LBRACKET";
        case TOKEN_RBRACKET: return "RBRACKET";
        case TOKEN_ADD: return "ADD";
        case TOKEN_SUB: return "SUB";
        case TOKEN_MUL: return "MUL";
        case TOKEN_DIV: return "DIV";
        case TOKEN_MOD: return "MOD";
        case TOKEN_EQ: return "EQ";
        case TOKEN_NEQ: return "NEQ";
        case TOKEN_LT: return "LT";
        case TOKEN_LEQ: return "LEQ";
        case TOKEN_GT: return "GT";
        case TOKEN_GEQ: return "GEQ";
        case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_ID: return "ID";
        case TOKEN_NUM: return "NUM";
        case TOKEN_EXPONENT: return "EXPONENT";
        case TOKEN_WHITESPACE: return "WHITESPACE";
        case TOKEN_NEWLINE: return "NEWLINE";
        default: return "NONE";
    }
}

typedef struct {
    char * str;
    int cap;
    int len;
} String;


lexer_t * init_lexer(char * file_text, regex_t regex, regmatch_t * m) {



    lexer_t * lex = malloc(sizeof(lexer_t)); 
    lex->file_text = file_text;
    lex->regex = regex;
    lex->match = m;
    lex->rule_count = NUMBER_OF_TOKENS;

    lex->line = 1;
    lex->col = 1;
    return lex;
}

token_t * init_token(enum TokenType token_type, int line, int col, char * str, int value) {
    token_t * t = malloc(sizeof(token_t));
    t->token_type = token_type;
    t->line = line;
    t->col = col;

    t->str = str;
    t->value = value;
    return t; 
}

enum TokenType peak_token(lexer_t * lex) {
    return peak_n_tokens(1, lex);
}


enum TokenType peak_n_tokens(int lookahead, lexer_t * lex) { 
    // @DEBUG
    // Inefficient, could memoize previously peeked values.
    // Store pointer to last memoized/regexed in lexer_t.

    if (lookahead == 0) {
        printf("Lookahead must be greater than 0.\n");
        exit(EXIT_FAILURE);
    }
        
    char * temp_str = lex->file_text;
    regmatch_t * m = lex->match;
    uint valid_count = 0;
    enum TokenType token_type;

    while (valid_count < lookahead) {
        memset(m, -1, sizeof(regmatch_t) * lex->rule_count + 1); 

        if (regexec(&lex->regex, temp_str, lex->rule_count + 1, m, 0) != 0) 
            return -1;

        uint valid = !(m[TOKEN_WHITESPACE].rm_so != -1 || m[TOKEN_NEWLINE].rm_so != -1);
        if (valid) 
            valid_count++;

        temp_str += m[0].rm_eo - m[0].rm_so;
    }

    if (m[TOKEN_SEMI].rm_so != -1)     return TOKEN_SEMI;
    if (m[TOKEN_ASSIGN].rm_so != -1)   return TOKEN_ASSIGN;
    if (m[TOKEN_COMMA].rm_so != -1)    return TOKEN_COMMA;
    if (m[TOKEN_LPAR].rm_so != -1)     return TOKEN_LPAR;
    if (m[TOKEN_RPAR].rm_so != -1)     return TOKEN_RPAR;
    if (m[TOKEN_LWING].rm_so != -1)    return TOKEN_LWING;
    if (m[TOKEN_RWING].rm_so != -1)    return TOKEN_RWING;
    if (m[TOKEN_LBRACKET].rm_so != -1) return TOKEN_LBRACKET;
    if (m[TOKEN_RBRACKET].rm_so != -1) return TOKEN_RBRACKET;
    if (m[TOKEN_ADD].rm_so != -1)      return TOKEN_ADD;
    if (m[TOKEN_SUB].rm_so != -1)      return TOKEN_SUB;
    if (m[TOKEN_MUL].rm_so != -1)      return TOKEN_MUL;
    if (m[TOKEN_DIV].rm_so != -1)      return TOKEN_DIV;
    if (m[TOKEN_MOD].rm_so != -1)      return TOKEN_MOD;
    if (m[TOKEN_EQ].rm_so != -1)       return TOKEN_EQ;
    if (m[TOKEN_NEQ].rm_so != -1)      return TOKEN_NEQ;
    if (m[TOKEN_LT].rm_so != -1)       return TOKEN_LT;
    if (m[TOKEN_LEQ].rm_so != -1)      return TOKEN_LEQ;
    if (m[TOKEN_GT].rm_so != -1)       return TOKEN_GT;
    if (m[TOKEN_GEQ].rm_so != -1)      return TOKEN_GEQ;
    if (m[TOKEN_IF].rm_so != -1)       return TOKEN_IF;
    if (m[TOKEN_WHILE].rm_so != -1)    return TOKEN_WHILE;
    if (m[TOKEN_ID].rm_so != -1)       return TOKEN_ID;
    if (m[TOKEN_NUM].rm_so != -1)      return TOKEN_NUM;
    if (m[TOKEN_EXPONENT].rm_so != -1) return TOKEN_EXPONENT;
    return -1;  
}

token_t * take_token(lexer_t * lex) { 
    enum TokenType token_type;
    char * s;
    int value = 0;
    regmatch_t * m = lex->match;


    uint invalid = 1;    // As in not newline or whitespace
    while (invalid) {
        memset(m, -1, sizeof(regmatch_t) * lex->rule_count + 1); 
        
        if (regexec(&lex->regex, lex->file_text, lex->rule_count + 1, m, 0) != 0) {
            printf("Parse error.\n");
            exit(EXIT_FAILURE);
        }
        
        invalid = (m[TOKEN_WHITESPACE].rm_so != -1 || m[TOKEN_NEWLINE].rm_so != -1);
        if (invalid) {
            if (m[TOKEN_WHITESPACE].rm_so != -1) {
                lex->col += m[0].rm_eo - m[0].rm_so;
            } else if (m[TOKEN_NEWLINE].rm_so != -1) {
                lex->line++;
                lex->col = 1;
            }
            lex->file_text += m[0].rm_eo - m[0].rm_so;
        }
    }

    int token_len = m[0].rm_eo - m[0].rm_so;
    if (m[TOKEN_SEMI].rm_so != -1) { 
        token_type = TOKEN_SEMI;
    }
    else if (m[TOKEN_ASSIGN].rm_so != -1) { 
        token_type = TOKEN_ASSIGN;
    }
    else if (m[TOKEN_COMMA].rm_so != -1) { 
        token_type = TOKEN_COMMA;
    }
    else if (m[TOKEN_LPAR].rm_so != -1) { 
        token_type = TOKEN_LPAR;
    }
    else if (m[TOKEN_RPAR].rm_so != -1) { 
        token_type = TOKEN_RPAR;
    }
    else if (m[TOKEN_LWING].rm_so != -1) { 
        token_type = TOKEN_LWING;
    }
    else if (m[TOKEN_RWING].rm_so != -1) { 
        token_type = TOKEN_RWING;
    }
    else if (m[TOKEN_LBRACKET].rm_so != -1) { 
        token_type = TOKEN_LBRACKET;
    }
    else if (m[TOKEN_RBRACKET].rm_so != -1) { 
        token_type = TOKEN_RBRACKET;
    }
    else if (m[TOKEN_ADD].rm_so != -1) {
        token_type = TOKEN_ADD;
    }
    else if (m[TOKEN_SUB].rm_so != -1) {
        token_type = TOKEN_SUB;
    }
    else if (m[TOKEN_MUL].rm_so != -1) {
        token_type = TOKEN_MUL; 
    }
    else if (m[TOKEN_DIV].rm_so != -1) {
        token_type = TOKEN_DIV;
    }
    else if (m[TOKEN_MOD].rm_so != -1) {
        token_type = TOKEN_MOD;
    }
    else if (m[TOKEN_EQ].rm_so != -1) {
        token_type = TOKEN_EQ;
    }
    else if (m[TOKEN_NEQ].rm_so != -1) {
        token_type = TOKEN_NEQ;
    }
    else if (m[TOKEN_LT].rm_so != -1) {
        token_type = TOKEN_LT;
    }
    else if (m[TOKEN_LEQ].rm_so != -1) {
        token_type = TOKEN_LEQ;
    }
    else if (m[TOKEN_GT].rm_so != -1) {
        token_type = TOKEN_GT;
    }
    else if (m[TOKEN_GEQ].rm_so != -1) {
        token_type = TOKEN_GEQ;
    }
    else if (m[TOKEN_IF].rm_so != -1) {
        token_type = TOKEN_IF;
    }
    else if (m[TOKEN_WHILE].rm_so != -1) {
        token_type = TOKEN_WHILE;
    }
    else if (m[TOKEN_ID].rm_so != -1) { 
        token_type = TOKEN_ID;
        s = malloc(token_len + 1); 
        for (int i = 0; i < token_len; i++) 
            s[i] = lex->file_text[i];  // Optimize!!!
        
        s[token_len] = '\0';
    }
    else if (m[TOKEN_NUM].rm_so != -1) { 
        token_type = TOKEN_NUM;
        char * value_s = malloc(token_len + 1);
        for (int i = 0; i < token_len; i++) 
            value_s[i] = lex->file_text[i];  // Optimize!!!!    

        value_s[token_len] = '\0';
        value = atoi(value_s);
    }
    else if (m[TOKEN_EXPONENT].rm_so != -1) {
        token_type = TOKEN_EXPONENT;
    }
    else if (m[TOKEN_WHITESPACE].rm_so != -1) {
        token_type = TOKEN_WHITESPACE;
        lex->col += m[0].rm_eo - m[0].rm_so;
    }
    else if (m[TOKEN_NEWLINE].rm_so != -1) {
        token_type = TOKEN_NEWLINE;
        lex->line++;
        lex->col = 1;
    }
    
    token_t * t = init_token(token_type, lex->line, lex->col, s, value);
    lex->file_text += m[0].rm_eo - m[0].rm_so;
    lex->col += m[0].rm_eo - m[0].rm_so;       // recheck that this works with changes to invalid token, unit tests would be nice here
    return t;
}


int main() {
    char * file_text = "9A if (abs == 10) , A - B       print(10000); while (list) {a[]}";
    regex_t regex;
    regmatch_t m[NUMBER_OF_TOKENS+1];
    regcomp(&regex, REGEX_RULES, REG_EXTENDED);
    lexer_t * lex = init_lexer(file_text, regex, m);
    printf("%s\n", lex->file_text);
    for (int i = 1; i < 28; i++) {
        printf("%s ", token_to_str(peak_n_tokens(i, lex)));
    }
    printf("\n");

    
    // for (uint i = 1; i < 20; i++) {
    //     token_t * t = take_token(lex);
    //     if (t->token_type == NUM){
    //         printf("%s: ", token_to_str(NUM));
    //         printf("%d\n", t->value);
    //     } else if (t->token_type == ID){
    //         printf("%s: ", token_to_str(ID));
    //         printf("%s\n", t->str);
    //     } else {
    //         printf("%s\n", token_to_str(t->token_type));
    //     }
    // }
    return 0;
}

// int main() {
//     return 0;    
// }

