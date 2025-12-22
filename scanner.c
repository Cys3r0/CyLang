#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "scanner.h"
#include <regex.h>


#define RINGBUF_SIZE 20
#define MAX_LEXEME_LENGTH 50


// TODO:
// Include a python style "pass" keyword
// add EXPONENT, (LOGICAL) NOT AND OR, (BITWISE) NOT AND OR XOR BITSHIFTS, INCREMENT, DECREMENT tokens

typedef struct {
    enum TokenType token_type;
    int col;
    int row;
    char * lexeme;
    int value;
} neo_token_t;

typedef struct {
    neo_token_t ** data;
    int length;
    int write_i;
    int read_i;
} tok_ringbuf_t;

typedef struct {
    char * source;
    int source_len;
    int source_i;
    int col;
    int row;
    tok_ringbuf_t peaked;
} neo_lexer_t;



char peak_char(int n, neo_lexer_t * lex) {
    if (lex->source_i + n <= lex->source_len) 
        return lex->source[lex->source_i + n]; 
    return '\0'; // should never match with value    
}    

char take_char(neo_lexer_t * lex) {
    char c = lex->source[lex->source_i];

    if (c == '\n') {
        lex->row++;    
        lex->col = 1;
    } else {
        lex->col++;
    }    
    lex->source_i++;
    
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

void ringbuf_put(tok_ringbuf_t * rb, neo_token_t * tok) {
    assert((rb->write_i + 1) % RINGBUF_SIZE != rb->read_i);

    if ((rb->write_i + 1) % RINGBUF_SIZE == rb->read_i) {
        printf("Error: ring buffer full.");
        exit(EXIT_FAILURE);
    }

    rb->data[rb->write_i] = tok;
    rb->length++;
    rb->write_i = (rb->write_i + 1) % RINGBUF_SIZE;
}

neo_token_t * ringbuf_get(tok_ringbuf_t * rb) {
    if (rb->write_i == rb->read_i) 
        return NULL;

    neo_token_t * ret_tok = rb->data[rb->read_i];
    rb->read_i = (rb->read_i + 1) % RINGBUF_SIZE;
    rb->length--;
    return ret_tok;
}

neo_token_t * ringbuf_peak(tok_ringbuf_t * rb) {
    return (rb->length > 0) ? rb->data[rb->read_i] : NULL ;
}

neo_token_t * ringbuf_get_n(tok_ringbuf_t * rb, int n) {
    if (n <= rb->length)
        return rb->data[(rb->read_i + n - 1) % RINGBUF_SIZE];
    return NULL;
}

tok_ringbuf_t create_ringbuf() {
    tok_ringbuf_t rb;
    rb.write_i = 0;
    rb.read_i = 0;
    rb.data = calloc(RINGBUF_SIZE, sizeof(neo_token_t *));
    return rb;
}


neo_lexer_t * create_lexer(char * source, int source_len) {
    neo_lexer_t * lex = calloc(1, sizeof(neo_lexer_t));
    lex->source = source;
    lex->source_len = source_len;
    lex->col = 1;
    lex->row = 1;
    lex->peaked = create_ringbuf();
    return lex;
}

neo_token_t * create_token(enum TokenType token_type, int row, int col, char * lexeme, int value) {
    neo_token_t * tok = malloc(sizeof(neo_token_t));
    tok->token_type = token_type;
    tok->row = row;
    tok->col = col;

    tok->lexeme = lexeme;
    tok->value = value;
    return tok;
}

neo_token_t * scan_next_tok(neo_lexer_t * lex) {
    // Could just the peak_char, should work
    enum TokenType token_type;
    char * lexeme = NULL;
    //line should be row, and they are used incorrectly. they should be given at each peaked token or some such
    //now they point at the end of a token , or???.
    int col;
    int line;
    int end_offset = 0;
    int value = 0;
    enum TokenType tok_type;
    unsigned int valid_char = 0;
    char c = take_char(lex);

    while (c == ' ' || c == '\n') {
        c = take_char(lex);
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
    else if (c == '^' && peak_char(0, lex) == '^') {
        token_type = TOKEN_EXPONENT;
        end_offset = 1;
    }
    else if (c == '>' && peak_char(0, lex) == '=') {
        token_type =  TOKEN_GEQ; 
        end_offset = 1;
    }
    else if (c == '>')
        token_type = TOKEN_GT;
    else if (c == '<' && peak_char(0, lex) == '=') {
        token_type =  TOKEN_LEQ; 
        end_offset = 1;
    }
    else if (c == '<')
        token_type = TOKEN_LT;
    else if (c == '=' && peak_char(0, lex) == '=') {
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
        && peak_char(0, lex) == 'h' 
        && peak_char(1, lex) == 'i' 
        && peak_char(2, lex) == 'l' 
        && peak_char(3, lex) == 'e'
        && !is_alphanumeric(peak_char(4, lex))) {
            token_type = TOKEN_WHILE;  
            end_offset = 4;
    }
    else if (c == 'i' 
        && peak_char(0, lex) == 'f' 
        && !is_alphanumeric(peak_char(1, lex))) {
            token_type = TOKEN_IF;
            end_offset = 1;
    }
    else if (c == 'e' 
        && peak_char(0, lex) == 'l' 
        && peak_char(1, lex) == 's' 
        && peak_char(2, lex) == 'e' 
        && !is_alphanumeric(peak_char(3, lex))) {
            token_type = TOKEN_ELSE;
            end_offset = 3;
    }
    else if (c == 'r' 
        && peak_char(0, lex) == 'e' 
        && peak_char(1, lex) == 't' 
        && peak_char(2, lex) == 'u'
        && peak_char(3, lex) == 'r'
        && peak_char(4, lex) == 'n' 
        && !is_alphanumeric(peak_char(5, lex))) {
            token_type = TOKEN_RETURN;
            end_offset = 5;
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
    if (c == '\0') 
        token_type = TOKEN_EOF;

    // CHECK THORUGH IF THIS OFFSET CALC IS CORRECT
    lex->source_i += end_offset;

    return create_token(token_type, lex->row, lex->col, lexeme, value);
}

enum TokenType peak_n_toks(neo_lexer_t * lex, int n) {
    if (n <= 0) {
        printf("Peak 0 tokens ahead not allowed.\n");
        exit(EXIT_FAILURE);
    }

    if (lex->peaked.length >= n) {
        return ringbuf_get_n(&lex->peaked, n)->token_type;
    }
    
    neo_token_t * tok = NULL;
    int current_len = lex->peaked.length;
    for (size_t i = 0; i < n - current_len; i++) {
        tok = scan_next_tok(lex);
        printf("scanned tok := %s\n", token_to_str(tok->token_type));
        ringbuf_put(&lex->peaked, tok);
    }
    return tok->token_type;
}

enum TokenType peak_tok(neo_lexer_t * lex) {
    neo_token_t * tok;
    if (lex->peaked.length > 0) {
        printf("checked\n");
        if ((tok = ringbuf_peak(&lex->peaked)) != NULL)
            return ringbuf_peak(&lex->peaked)->token_type;
    }
    
    tok = scan_next_tok(lex);
    ringbuf_put(&lex->peaked, tok);
    // return TOKEN_SEMI;
    return tok->token_type;
}

neo_token_t * take_tok(neo_lexer_t * lex) {
    if (lex->peaked.length > 0) 
        return ringbuf_get(&lex->peaked);
    
    return scan_next_tok(lex);
}

void skip_token(neo_lexer_t * lex, enum TokenType skipped) {
    if (take_tok(lex)->token_type != skipped) {
        printf("Unexpected token at %d, %d.", lex->row, lex->col);
        exit(EXIT_FAILURE);
    }
}


int main() {
    char * source = "9 if (abs == 10) , A - B print(10000); while (list) {a[]}";
    // source = "while else if return == ";

    int len = strlen(source);
    neo_lexer_t * lex = create_lexer(source, len);
    printf("%s\n", lex->source);

    //TODO: make sure that take token works after peak_tok
    // I've changed some values right now, i.e in the peak token to not peak directly so please check that.
    // and try to do peak_tok and then take tok 
    // printf("%d:%s ", 1, token_to_str(take_tok(lex)->token_type));
    for (int i = 0; i < 19; i++) {
        printf("%d:%s ", i, token_to_str(peak_n_toks(lex, i)));
        // printf("%d:%s ", i, token_to_str(peak_n_toks(lex, 2)));
        // printf("%d:%s ", i, token_to_str(peak_n_toks(lex, 3)));
    }
    printf("\n");

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
        case TOKEN_SEMI:       return "SEMI";
        case TOKEN_ASSIGN:     return "ASSIGN";
        case TOKEN_COMMA:      return "COMMA";
        case TOKEN_LPAR:       return "LPAR";
        case TOKEN_RPAR:       return "RPAR";
        case TOKEN_LWING:      return "LWING";
        case TOKEN_RWING:      return "RWING";
        case TOKEN_LBRACKET:   return "LBRACKET";
        case TOKEN_RBRACKET:   return "RBRACKET";
        case TOKEN_ADD:        return "ADD";
        case TOKEN_SUB:        return "SUB";
        case TOKEN_MUL:        return "MUL";
        case TOKEN_DIV:        return "DIV";
        case TOKEN_MOD:        return "MOD";
        case TOKEN_EQ:         return "EQ";
        case TOKEN_NEQ:        return "NEQ";
        case TOKEN_LT:         return "LT";
        case TOKEN_LEQ:        return "LEQ";
        case TOKEN_GT:         return "GT";
        case TOKEN_GEQ:        return "GEQ";
        case TOKEN_IF:         return "IF";
        case TOKEN_ELSE:       return "ELSE";
        case TOKEN_RETURN:     return "RETURN";
        case TOKEN_WHILE:      return "WHILE";
        case TOKEN_ID:         return "ID";
        case TOKEN_NUM:        return "NUM";
        case TOKEN_EXPONENT:   return "EXPONENT";
        case TOKEN_WHITESPACE: return "WHITESPACE";
        case TOKEN_NEWLINE:    return "NEWLINE";
        default:               return "NONE";
    }
}


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


int old_main() {
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

