#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "scanner.h"
#include <regex.h>



//TODO: 
//include a lookahead counter in lex to avoid passing a lookahead int.
//add ELSE, EXPONENT, (LOGICAL) NOT AND OR, (BITWISE) NOT AND OR XOR BITSHIFTS, EXPONENT, INCREMENT, DECREMENT token
//Create EOF-token at end of file


// LATER:
// Optimize the string
// Include a python style "pass" keyword


const int NUMBER_OF_TOKENS = 26;

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
    "|(while)"
    "|([a-zA-Z][a-zA-Z0-9]*)"
    "|([0-9]+)"
    "|(\\^\\^)"
    "|([ ]+)"
    "|(\n)";


char * token_to_str(enum TokenType token_type) {
    switch (token_type) {
        case SEMI: return "SEMI";
        case ASSIGN: return "ASSIGN";
        case COMMA: return "COMMA";
        case LPAR: return "LPAR";
        case RPAR: return "RPAR";
        case LWING: return "LWING";
        case RWING: return "RWING";
        case LBRACKET: return "LBRACKET";
        case RBRACKET: return "RBRACKET";
        case ADD: return "ADD";
        case SUB: return "SUB";
        case MUL: return "MUL";
        case DIV: return "DIV";
        case MOD: return "MOD";
        case EQ: return "EQ";
        case NEQ: return "NEQ";
        case LT: return "LT";
        case LEQ: return "LEQ";
        case GT: return "GT";
        case GEQ: return "GEQ";
        case IF: return "IF";
        case WHILE: return "WHILE";
        case ID: return "ID";
        case NUM: return "NUM";
        case EXPONENT: return "EXPONENT";
        case WHITESPACE: return "WHITESPACE";
        case NEWLINE: return "NEWLINE";
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

        uint valid = !(m[25].rm_so != -1 || m[26].rm_so != -1);
        if (valid) 
            valid_count++;

        temp_str += m[0].rm_eo - m[0].rm_so;
    }

    if (m[SEMI].rm_so != -1)     return SEMI;
    if (m[ASSIGN].rm_so != -1)   return ASSIGN;
    if (m[COMMA].rm_so != -1)    return COMMA;
    if (m[LPAR].rm_so != -1)     return LPAR;
    if (m[RPAR].rm_so != -1)     return RPAR;
    if (m[LWING].rm_so != -1)    return LWING;
    if (m[RWING].rm_so != -1)    return RWING;
    if (m[LBRACKET].rm_so != -1) return LBRACKET;
    if (m[RBRACKET].rm_so != -1) return RBRACKET;
    if (m[ADD].rm_so != -1)      return ADD;
    if (m[SUB].rm_so != -1)      return SUB;
    if (m[MUL].rm_so != -1)      return MUL;
    if (m[DIV].rm_so != -1)      return DIV;
    if (m[MOD].rm_so != -1)      return MOD;
    if (m[EQ].rm_so != -1)       return EQ;
    if (m[NEQ].rm_so != -1)      return NEQ;
    if (m[LT].rm_so != -1)       return LT;
    if (m[LEQ].rm_so != -1)      return LEQ;
    if (m[GT].rm_so != -1)       return GT;
    if (m[GEQ].rm_so != -1)      return GEQ;
    if (m[IF].rm_so != -1)       return IF;
    if (m[WHILE].rm_so != -1)    return WHILE;
    if (m[ID].rm_so != -1)       return ID;
    if (m[NUM].rm_so != -1)      return NUM;
    if (m[EXPONENT].rm_so != -1) return EXPONENT;
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
        
        invalid = (m[25].rm_so != -1 || m[26].rm_so != -1);
        if (invalid) {
            if (m[25].rm_so != -1) {
                lex->col += m[0].rm_eo - m[0].rm_so;
            } else if (m[26].rm_so != -1) {
                lex->line++;
                lex->col = 1;
            }
            lex->file_text += m[0].rm_eo - m[0].rm_so;
        }
    }

    int token_len = m[0].rm_eo - m[0].rm_so;
    if (m[SEMI].rm_so != -1) { 
        token_type = SEMI;
    }
    else if (m[ASSIGN].rm_so != -1) { 
        token_type = ASSIGN;
    }
    else if (m[COMMA].rm_so != -1) { 
        token_type = COMMA;
    }
    else if (m[LPAR].rm_so != -1) { 
        token_type = LPAR;
    }
    else if (m[RPAR].rm_so != -1) { 
        token_type = RPAR;
    }
    else if (m[LWING].rm_so != -1) { 
        token_type = LWING;
    }
    else if (m[RWING].rm_so != -1) { 
        token_type = RWING;
    }
    else if (m[LBRACKET].rm_so != -1) { 
        token_type = LBRACKET;
    }
    else if (m[RBRACKET].rm_so != -1) { 
        token_type = RBRACKET;
    }
    else if (m[ADD].rm_so != -1) {
        token_type = ADD;
    }
    else if (m[SUB].rm_so != -1) {
        token_type = SUB;
    }
    else if (m[MUL].rm_so != -1) {
        token_type = MUL; 
    }
    else if (m[DIV].rm_so != -1) {
        token_type = DIV;
    }
    else if (m[MOD].rm_so != -1) {
        token_type = MOD;
    }
    else if (m[EQ].rm_so != -1) {
        token_type = EQ;
    }
    else if (m[NEQ].rm_so != -1) {
        token_type = NEQ;
    }
    else if (m[LT].rm_so != -1) {
        token_type = LT;
    }
    else if (m[LEQ].rm_so != -1) {
        token_type = LEQ;
    }
    else if (m[GT].rm_so != -1) {
        token_type = GT;
    }
    else if (m[GEQ].rm_so != -1) {
        token_type = GEQ;
    }
    else if (m[IF].rm_so != -1) {
        token_type = IF;
    }
    else if (m[WHILE].rm_so != -1) {
        token_type = WHILE;
    }
    else if (m[ID].rm_so != -1) { 
        token_type = ID;
        s = malloc(token_len + 1); 
        for (int i = 0; i < token_len; i++) 
            s[i] = lex->file_text[i];  // Optimize!!!
        
        s[token_len] = '\0';
    }
    else if (m[NUM].rm_so != -1) { 
        token_type = NUM;
        char * value_s = malloc(token_len + 1);
        for (int i = 0; i < token_len; i++) 
            value_s[i] = lex->file_text[i];  // Optimize!!!!    

        value_s[token_len] = '\0';
        value = atoi(value_s);
    }
    else if (m[EXPONENT].rm_so != -1) {
        token_type = EXPONENT;
    }
    else if (m[WHITESPACE].rm_so != -1) {
        token_type = WHITESPACE;
        lex->col += m[0].rm_eo - m[0].rm_so;
    }
    else if (m[NEWLINE].rm_so != -1) {
        token_type = NEWLINE;
        lex->line++;
        lex->col = 1;
    }
    
    token_t * t = init_token(token_type, lex->line, lex->col, s, value);
    lex->file_text += m[0].rm_eo - m[0].rm_so;
    lex->col += m[0].rm_eo - m[0].rm_so;       // recheck that this works with changes to invalid token, unit tests would be nice here
    return t;
}


int test_main() {
    char * file_text = "if (abs == 10) , A - B       print(10000); while (list) {a[]}";
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

