#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "scanner.h"
#include <regex.h>




// LATER:
// peak_token could just be take token that memoizes the tokens for later 
// Move all regex stuff into init_lexer
// memoize peak_n_tokens
// add a take_token_specifc, which takes expects a certain token which is passed to it (or include this in skip_token)
// add a skip_token that doesn't create a token_t object but removes form lex->file_text
// add ELSE, EXPONENT, (LOGICAL) NOT AND OR, (BITWISE) NOT AND OR XOR BITSHIFTS, INCREMENT, DECREMENT tokens
// Create EOF-token at end of file
// Include a python style "pass" keyword
// Create own scanner


char peak_char(char * source) {
    //add \0 check
    return source[0];
}

char take_char(char ** source) {
    //add \0 check
    char c = (*source)[0];
    (*source)++;
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

enum TokenType peak_tok(char * s) {
    // Incomplete, needs expanding as soon as identifiers take form [a-zA-Z][a-zA-Z0-9],
    // which it already should to be honest

    char c = peak_char(s);
    
    int i = 0; 
    while (!valid_char) {
        
        if (c == ' ' || c == '\n') {
            i++;
            c = peak_char(s+i);
        } else 
            valid_char = 1;
    }


    if (c == ';')
        return TOKEN_SEMI;
    if (c == ',')
        return TOKEN_COMMA;
    if (c == '(')
        return TOKEN_LPAR;
    if (c == ')')
        return TOKEN_RPAR;
    if (c == '{')
        return TOKEN_LWING;
    if (c == '}')
        return TOKEN_RWING;
    if (c == '[')
        return TOKEN_LBRACKET;
    if (c == ']')
        return TOKEN_RBRACKET;
    if (c == '+')
        return TOKEN_ADD;
    if (c == '-')
        return TOKEN_SUB;
    if (c == '*')
        return TOKEN_MUL;
    if (c == '/')
        return TOKEN_DIV;
    if (c == '%')
        return TOKEN_MOD;
    if (c == '^') 
        return (peak_char(s+1) == '^') ? TOKEN_EXPONENT : TOKEN_EXPONENT; //TOKEN_XOR
    if (c == '>') 
        return (peak_char(s+1) == '=') ? TOKEN_GT : TOKEN_GEQ; 
    if (c == '<') 
        return (peak_char(s+1) == '=') ? TOKEN_LT : TOKEN_LEQ; 
    if (c == '=') 
        return (peak_char(s+1) == '=') ? TOKEN_ASSIGN : TOKEN_EQ;
    if (c == '!' || peak_char(s+1) == '=') 
        return TOKEN_NEQ;
    // if (c == '!') 
    //     return (peak_char(s+1) == '=') ? TOKEN_ASSIGN : TOKEN_NEQ; //TOKEN_NOT
    // if (c == '|') 
    //     return (peak_char(s+1) == '|') ? TOKEN_ASSIGN : TOKEN_NEQ; //TOKEN_NOT
    if (c == 'w' 
        && peak_char(s+1) == 'h' 
        && peak_char(s+2) == 'i' 
        && peak_char(s+3) == 'l' 
        && peak_char(s+4) == 'e'
        && !is_letter(peak_char(s+5))) return TOKEN_WHILE;
    if (c == 'i' 
        && peak_char(s+1) == 'f' 
        && !is_letter(peak_char(s+2))) return TOKEN_IF;
    if (c == 'e' 
        && peak_char(s+1) == 'l' 
        && peak_char(s+2) == 's' 
        && peak_char(s+3) == 'e' 
        && !is_letter(peak_char(s+4))) return TOKEN_ELSE;
    if (c == 'r' 
        && peak_char(s+1) == 'e' 
        && peak_char(s+2) == 't' 
        && peak_char(s+3) == 'u'
        && peak_char(s+4) == 'r'
        && peak_char(s+5) == 'n' 
        && !is_letter(peak_char(s+6))) return TOKEN_RETURN;
    if (is_letter(c))
        return TOKEN_ID;
    if (is_numeric(c))
        return TOKEN_NUM;
}


typedef struct {
    char * source;
    int col;
    int row;
} neo_lexer_t;

typedef struct {
    enum TokenType type;
    int col;
    int row;
    union {
        char * lexeme;
        int value;
    }
} neo_token_t;


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

neo_token_t * take_tok(neo_lexer_t * lex) {
    enum TokenType token_type;
    char * s = lex->source;
    char * lexeme = NULL;
    int col;
    int line;
    int value = 0;
    enum TokenType tok_type;
    unsigned int valid_char = 0;
    char c = take_char(s);

    // goto here in case of whitespace would be nice

    while (c == ' ' || c == '\n') {
        if (c == ' ') {
            lex->col++;
        } else {
            lex->row++;;
            lex->col=1;
        }
        c = take_char(s);
    }

    if (c == ';')
        return TOKEN_SEMI;
    else if (c == ',')
        return TOKEN_COMMA;
    else if (c == '(')
        return TOKEN_LPAR;
    else if (c == ')')
        return TOKEN_RPAR;
    else if (c == '{')
        return TOKEN_LWING;
    else if (c == '}')
        return TOKEN_RWING;
    else if (c == '[')
        return TOKEN_LBRACKET;
    else if (c == ']')
        return TOKEN_RBRACKET;
    else if (c == '+')
        return TOKEN_ADD;
    else if (c == '-')
        return TOKEN_SUB;
    else if (c == '*')
        return TOKEN_MUL;
    else if (c == '/')
        return TOKEN_DIV;
    else if (c == '%')
        return TOKEN_MOD;
    else if (c == '^') 
        return (peak_char(s+1) == '^') ? TOKEN_EXPONENT : TOKEN_EXPONENT; //TOKEN_XOR
    else if (c == '>') 
        return (peak_char(s+1) == '=') ? TOKEN_GT : TOKEN_GEQ; 
    else if (c == '<') 
        return (peak_char(s+1) == '=') ? TOKEN_LT : TOKEN_LEQ; 
    else if (c == '=') 
        return (peak_char(s+1) == '=') ? TOKEN_ASSIGN : TOKEN_EQ;
    else if (c == '!' 
        || peak_char(s+1) == '=') return TOKEN_NEQ;
    // else if (c == '!') 
    //     return (peak_char(s+1) == '=') ? TOKEN_ASSIGN : TOKEN_NEQ; //TOKEN_NOT
    // else if (c == '|') 
    //     return (peak_char(s+1) == '|') ? TOKEN_ASSIGN : TOKEN_NEQ; //TOKEN_BIT_AND
    else if (c == 'w' 
        && peak_char(s+1) == 'h' 
        && peak_char(s+2) == 'i' 
        && peak_char(s+3) == 'l' 
        && peak_char(s+4) == 'e'
        && !is_letter(peak_char(s+5))) return TOKEN_WHILE;
    else if (c == 'i' 
        && peak_char(s+1) == 'f' 
        && !is_letter(peak_char(s+2))) return TOKEN_IF;
    else if (c == 'e' 
        && peak_char(s+1) == 'l' 
        && peak_char(s+2) == 's' 
        && peak_char(s+3) == 'e' 
        && !is_letter(peak_char(s+4))) return TOKEN_ELSE;
    else if (c == 'r' 
        && peak_char(s+1) == 'e' 
        && peak_char(s+2) == 't' 
        && peak_char(s+3) == 'u'
        && peak_char(s+4) == 'r'
        && peak_char(s+5) == 'n' 
        && !is_letter(peak_char(s+6))) return TOKEN_RETURN;
    else if (is_letter(c)) {
        token_type = TOKEN_ID;
        int i = 1;
        while (is_alphanumeric(peak_char(s+1))) 
            i++;

        lex->line += i;        
        lexeme = malloc(50);
        for (size_t j = 0; j < s+i; j++) 
            lexeme[j] = s[j];
        lexeme[i] = '\0';
    }
    else if (is_numeric(c)) {
        token_type = TOKEN_NUM;
        int i = 1;
        while (is_numeric(peak_char(s+1))) {
            i++;
        }
        if (is_alphanumeric(peak_char(s+1))) 
            printf("Incorrect numeral syntax");
            
        lex->line += i;
        char number[50];
        for (size_t j = 0; j < s+i; j++) 
            number[j] = s[j];
        number[i] = '\0';
        value = atoi(number); 
    }

    return neo_create_token(
        token_type,
        lex->line;
        lex->col;
        lexeme,
        value,
    )
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

