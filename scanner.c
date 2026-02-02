#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "scanner.h"
// #include <regex.h>


#define RINGBUF_SIZE 20


// LATER:
// add a struct/group keyword. Preferably group
// clean up take_char, scan_next_tok etc.

char peak_char(int n, lexer_t * lex) {
    if (lex->take_i + n <= lex->source_len) 
        return lex->source[lex->take_i + n]; 
    return '\0'; 
}    

char take_char(lexer_t * lex) {
    char c = lex->source[lex->take_i];

    if (c == '\n') {
        lex->row++;
        lex->col = 1;
    } else {
        lex->col++;
    }
    
    return c;
}    

int is_numeric(char c) {
    return '0' <= c && c <= '9';
}    

int is_letter(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ;
}    

int is_alphanumeric(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9');
}    

void ringbuf_put(tok_ringbuf_t * rb, token_t * tok) {
    assert((rb->write_i + 1) % RINGBUF_SIZE != rb->read_i);

    if ((rb->write_i + 1) % RINGBUF_SIZE == rb->read_i) {
        printf("Error: ring buffer full.");
        exit(EXIT_FAILURE);
    }

    rb->data[rb->write_i] = tok;
    rb->write_i = (rb->write_i + 1) % RINGBUF_SIZE;
    rb->length++;
    // printf("PUT rb->length:, %d\n", rb->length);
}

token_t * ringbuf_get(tok_ringbuf_t * rb) {
    if (rb->write_i == rb->read_i) {
        printf("Error: ringbuffer empty.\n");
        return NULL;
    }


    token_t * ret_tok = rb->data[rb->read_i];
    rb->read_i = (rb->read_i + 1) % RINGBUF_SIZE;
    rb->length--;
    // printf("GET rb->length:, %d\n", rb->length);
    return ret_tok;
}

token_t * ringbuf_peak(tok_ringbuf_t * rb) {
    return (rb->length > 0) ? rb->data[rb->read_i] : NULL ;
}

token_t * ringbuf_get_n(tok_ringbuf_t * rb, int n) {
    if (n <= rb->length)
        return rb->data[(rb->read_i + n - 1) % RINGBUF_SIZE];
    return NULL;
}

tok_ringbuf_t create_ringbuf() {
    tok_ringbuf_t rb;
    rb.write_i = 0;
    rb.read_i = 0;
    rb.data = calloc(RINGBUF_SIZE, sizeof(token_t *));
    return rb;
}


lexer_t * create_lexer(char * source, int source_len) {
    lexer_t * lex = calloc(1, sizeof(lexer_t));
    lex->source = source;
    lex->source_len = source_len;
    lex->col = 1;
    lex->row = 1;
    lex->peaked = create_ringbuf();
    return lex;
}

token_t * create_token(enum TokenType token_type, int row, int col, char * lexeme, int value) {
    token_t * tok = malloc(sizeof(token_t));
    tok->token_type = token_type;
    tok->row = row;
    tok->col = col;

    tok->lexeme = lexeme;
    tok->value = value;
    return tok;
}

void destroy_token(token_t * tok) {
    if (tok->lexeme) 
        { free(tok->lexeme); }
    free(tok);
}

token_t * scan_next_tok(lexer_t * lex) {
    // Could just the peak_char, should work
    enum TokenType token_type;
    char * lexeme = NULL;
    //line should be row, and they are used incorrectly. they should be given at each peaked token or some such
    //now they point at the end of a token , or???.
    int col;
    int line;
    int end_offset = 1;
    int value = 0;
    enum TokenType tok_type;
    unsigned int valid_char = 0;
    char c = take_char(lex);

    while (c == ' ' || c == '\n') {
        lex->take_i++;
        c = take_char(lex);
    }

    if (c == ';')
        token_type = TOKEN_SEMI;
    else if (c == ',')
        token_type = TOKEN_COLON;
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
    else if (c == '@') 
        token_type = TOKEN_DEREF;
    else if (c == '-' && peak_char(1, lex) == '>') {
        token_type =  TOKEN_ADDRESSOF; 
        int peaked = 2;
        while (peak_char(peaked, lex) == '>') 
            { peaked++; }
        
        value = peaked-1;
        end_offset = peaked;
    }
    else if (c == '-')
        token_type = TOKEN_SUB;
    else if (c == '+')
        token_type = TOKEN_ADD;
    else if (c == '*')
        token_type = TOKEN_MUL;
    else if (c == '/')
        token_type = TOKEN_DIV;
    else if (c == '%')
        token_type = TOKEN_MOD;
    else if (c == '^' && peak_char(1, lex) == '^') {
        token_type = TOKEN_EXPONENT;
        end_offset = 2;
    }
    else if (c == '>' && peak_char(1, lex) == '=') {
        token_type =  TOKEN_GEQ; 
        end_offset = 2;
    }
    else if (c == '>')
        token_type = TOKEN_GT;
    else if (c == '<' && peak_char(1, lex) == '=') {
        token_type =  TOKEN_LEQ; 
        end_offset = 2;
    }
    else if (c == '<')
        token_type = TOKEN_LT;
    else if (c == '=' && peak_char(1, lex) == '=') {
        token_type = TOKEN_EQ; 
        end_offset = 2;
    }
    else if (c == '=') 
        token_type = TOKEN_ASSIGN;
    else if (c == '!' && peak_char(1, lex) == '=') {
        token_type = TOKEN_NEQ; 
        end_offset = 2;
    }
    else if (c == '!' && peak_char(1, lex) == '=') {
        token_type = TOKEN_NEQ; 
        end_offset = 2;
    }
    else if (c == '!') 
        token_type = TOKEN_LOG_NOT;
    else if (c == '~') 
        token_type = TOKEN_BIT_NOT;
    else if (c == '|'  && peak_char(1, lex) == '|') {
        token_type = TOKEN_LOG_OR; 
        end_offset = 2;
    }
    else if (c == '|') 
        token_type = TOKEN_BIT_OR;
    else if (c == '&'  && peak_char(1, lex) == '&') {
        token_type = TOKEN_LOG_AND; 
        end_offset = 2;
    }
    else if (c == '&') 
        token_type = TOKEN_BIT_AND;
    else if (c == 'i' 
        && peak_char(1, lex) == '3'
        && peak_char(2, lex) == '2' 
        && !is_alphanumeric(peak_char(3, lex))) {
            token_type = TOKEN_I32;
            end_offset = 3;
    }
    else if (c == 'b' 
        && peak_char(1, lex) == 'o'
        && peak_char(2, lex) == 'o' 
        && peak_char(3, lex) == 'l' 
        && !is_alphanumeric(peak_char(4, lex))) {
            token_type = TOKEN_BOOL;
            end_offset = 4;
    }
    else if (c == 'p' 
        && peak_char(1, lex) == 'a' 
        && peak_char(2, lex) == 's' 
        && peak_char(3, lex) == 's' 
        && !is_alphanumeric(peak_char(4, lex))) {
            token_type = TOKEN_PASS;  
            end_offset = 4;
    }
    else if (c == 'w' 
        && peak_char(1, lex) == 'h' 
        && peak_char(2, lex) == 'i' 
        && peak_char(3, lex) == 'l' 
        && peak_char(4, lex) == 'e'
        && !is_alphanumeric(peak_char(5, lex))) {
            token_type = TOKEN_WHILE;  
            end_offset = 5;
    }
    else if (c == 'i' 
        && peak_char(1, lex) == 'f' 
        && !is_alphanumeric(peak_char(1, lex))) {
            token_type = TOKEN_IF;
            end_offset = 2;
    }
    else if (c == 'e' 
        && peak_char(1, lex) == 'l' 
        && peak_char(2, lex) == 's' 
        && peak_char(3, lex) == 'e' 
        && !is_alphanumeric(peak_char(4, lex))) {
            token_type = TOKEN_ELSE;
            end_offset = 4;
    }
    else if (c == 'r' 
        && peak_char(1, lex) == 'e' 
        && peak_char(2, lex) == 't' 
        && peak_char(3, lex) == 'u'
        && peak_char(4, lex) == 'r'
        && peak_char(5, lex) == 'n' 
        && !is_alphanumeric(peak_char(6, lex))) {
            token_type = TOKEN_RETURN;
            end_offset = 6;
    }
    else if (c == 's' 
        && peak_char(1, lex) == 't' 
        && peak_char(2, lex) == 'r' 
        && peak_char(3, lex) == 'u'
        && peak_char(4, lex) == 'c'
        && peak_char(5, lex) == 't' 
        && !is_alphanumeric(peak_char(6, lex))) {
            token_type = TOKEN_STRUCT;
            end_offset = 6;
    }
    else if (is_letter(c)) {
        token_type = TOKEN_ID;
        int i = 0;
        while (is_alphanumeric(peak_char(i, lex))) 
            i++;

        lexeme = malloc(MAX_LEXEME_LENGTH);
        for (size_t j = 0; j < i; j++) 
            lexeme[j] = peak_char(j, lex);
        lexeme[i] = '\0';

        end_offset = i;
    }
    else if (is_numeric(c)) {
        token_type = TOKEN_NUM;
        int i = 0;
        while (is_numeric(peak_char(i, lex))) 
            i++;

        if (is_alphanumeric(peak_char(i, lex))) 
            printf("Incorrect numeral syntax");
        if (i >= MAX_LEXEME_LENGTH)
            printf("Number has too many characters");

        char number[MAX_LEXEME_LENGTH];
        for (size_t j = 0; j < i; j++) 
            number[j] = peak_char(j, lex);
        number[i] = '\0';
        
        value = atoi(number); 
        end_offset = i;
    }
    else if (c == '\0') 
        token_type = TOKEN_EOF;
    else {
        token_type = TOKEN_ERROR;
    }

    lex->take_i += end_offset;
    return create_token(token_type, lex->row, lex->col, lexeme, value);
}

enum TokenType peak_n_tokens(int n, lexer_t * lex) {
    if (n <= 0) {
        printf("Peak 0 tokens ahead not allowed.\n");
        exit(EXIT_FAILURE);
    }

    if (lex->peaked.length >= n) {
        return ringbuf_get_n(&lex->peaked, n)->token_type;
    }
    
    token_t * tok = NULL;
    int current_len = lex->peaked.length;
    for (size_t i = 0; i < n - current_len; i++) {
        tok = scan_next_tok(lex);
        ringbuf_put(&lex->peaked, tok);
    }
    return tok->token_type;
}


enum TokenType peak_token(lexer_t * lex) {
    token_t * tok;
    if (lex->peaked.length > 0) {
        if ((tok = ringbuf_peak(&lex->peaked)) != NULL)
            return ringbuf_peak(&lex->peaked)->token_type;
    }
    
    tok = scan_next_tok(lex);
    ringbuf_put(&lex->peaked, tok);
    // return TOKEN_SEMI;
    return tok->token_type;
}

token_t * take_token(lexer_t * lex) {
    if (lex->peaked.length > 0) {
        return ringbuf_get(&lex->peaked);
    }    
    return scan_next_tok(lex);
}

void skip_token(lexer_t * lex, enum TokenType skipped) {
    // can optimize to not create token to begin with.
    token_t * tok = take_token(lex); 
    if (tok->token_type != skipped) {
        printf("Unexpected token at %d, %d.", lex->row, lex->col);
        exit(EXIT_FAILURE);
    }   
    destroy_token(tok); // 
}



char * token_to_str(enum TokenType type) {
    switch (type) {
        case TOKEN_SEMI:        return "SEMI";
        case TOKEN_ASSIGN:      return "ASSIGN";
        case TOKEN_COMMA:       return "COMMA";
        case TOKEN_LPAR:        return "LPAR";
        case TOKEN_RPAR:        return "RPAR";
        case TOKEN_LWING:       return "LWING";
        case TOKEN_RWING:       return "RWING";
        case TOKEN_LBRACKET:    return "LBRACKET";
        case TOKEN_RBRACKET:    return "RBRACKET";
        case TOKEN_ADDRESSOF:   return "ADDRESSOF";
        case TOKEN_COLON:       return "COLON";
        case TOKEN_ADD:         return "ADD";
        case TOKEN_SUB:         return "SUB";
        case TOKEN_MUL:         return "MUL";
        case TOKEN_DIV:         return "DIV";
        case TOKEN_MOD:         return "MOD";
        case TOKEN_EQ:          return "EQ";
        case TOKEN_NEQ:         return "NEQ";
        case TOKEN_LT:          return "LT";
        case TOKEN_LEQ:         return "LEQ";
        case TOKEN_GT:          return "GT";
        case TOKEN_GEQ:         return "GEQ";
        case TOKEN_LOG_NOT:     return "LOG_NOT";
        case TOKEN_LOG_AND:     return "LOG_AND";
        case TOKEN_LOG_OR:      return "LOG_OR";
        case TOKEN_BIT_NOT:     return "BIT_NOT";
        case TOKEN_BIT_AND:     return "BIT_AND";
        case TOKEN_BIT_OR:      return "BIT_OR";
        case TOKEN_BIT_XOR:     return "BIT_XOR";
        case TOKEN_DEREF:       return "DEREF";
        case TOKEN_I32:         return "I32";
        case TOKEN_BOOL:        return "BOOL";
        case TOKEN_PASS:        return "PASS";
        case TOKEN_IF:          return "IF";
        case TOKEN_ELSE:        return "ELSE";
        case TOKEN_RETURN:      return "RETURN";
        case TOKEN_WHILE:       return "WHILE";
        case TOKEN_ID:          return "ID";
        case TOKEN_NUM:         return "NUM";
        case TOKEN_EXPONENT:    return "EXPONENT";
        case TOKEN_WHITESPACE:  return "WHITESPACE";
        case TOKEN_NEWLINE:     return "NEWLINE";
        case TOKEN_EOF:         return "EOF";
        case TOKEN_ERROR:       return "ERROR";
        default:                return "NONE";
    }
}


int no_main() {
    char * source = "9 if (abs == 10) , A - B print(10000); while (list) {a[]}";
    source = "->>>>> i32 counter = -> null;";
    lexer_t * lex = create_lexer(source, strlen(source));
    printf("%s\n", lex->source);

    for (int i = 1; i < 20; i++) {
        printf("%s ", token_to_str(peak_n_tokens(i, lex)));
    }
    printf("\n");
    for (int i = 1; i < 20; i++) {
        printf("%s ", token_to_str(take_token(lex)->token_type));
    }
    
    printf("\n");
    return 0;
}

