#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>



//TODO: 
//include a lookahead counter in lexer to avoid passing a lookahead int.
//add ELSE, EXPONENT, (LOGICAL) NOT AND OR, (BITWISE) NOT AND OR XOR BITSHIFTS, EXPONENT, INCREMENT, DECREMENT token
//Create EOF-token at end of file


// LATER:
// Optimize the string
// Include a python style "pass" keyword




typedef struct {
    int token_id;
    int line;
    int col;

    char * str;         // union for str + value (?)
    int value;
} token_t;


typedef struct {
    char * file_text;
    regex_t regex;
    regmatch_t * match;
    int line;
    int col;
} lexer_t;


int NUMBER_OF_TOKENS = 26;

typedef enum {
    ID, NUM, ASSIGN, SEMI, LPAR, RPAR,
    LWING, RWING, LBRACKET, RBRACKET,
    ADD, SUB, MUL, DIV, MOD, EQ, NEQ,
    LT, LEQ, GT, GEQ, IF, WHILE, WHITESPACE,
    NEWLINE, ELSE,


    VOID,
} token_id_enum;


const char * rules = 
    "(;)"
    "|(=)"
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
    "|([ ]+)"
    "|(\n)";

char * token_to_str(token_id_enum token_id) {
    switch (token_id) {
        case ID: return "ID";
        case NUM: return "NUM";
        case ASSIGN: return "ASSIGN";
        case SEMI: return "SEMI";
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
        case WHITESPACE: return "WHITESPACE";
        case NEWLINE: return "NEWLINE";
        default: return "NONE";
    }
}


int precedence_table[11] = {100, 101, 200, 201, 202};
                          //ADD, SUB, MUL, DIV, MOD
                          //the extra increments so that we always get the same parse tree.



typedef struct {
    char * str;
    int cap;
    int len;
} String;


lexer_t * init_lexer(char * file_text, regex_t regex, regmatch_t * m) {
    lexer_t * lexer = malloc(sizeof(lexer_t)); 
    lexer->file_text = file_text;
    lexer->regex = regex;
    lexer->match = m;

    lexer->line = 1;
    lexer->col = 1;
    return lexer;
}

token_t * init_token(int token_id, int line, int col, char * str, int value) {
    

    token_t * t = malloc(sizeof(token_t));
    t->token_id = token_id;
    t->line = line;
    t->col = col;

    t->str = str;
    t->value = value;
    return t; 
}

int peak_token(int lookahead, lexer_t * lex) { 
    // @DEBUG
    // Inefficient, could memoize previously peeked values.
    // Store some kind of pointer to last memoized/regexed in lexer_t.

    if (lookahead == 0) {
        printf("Lookahead must be greater than 0.\n");
        exit(EXIT_FAILURE);
    }
        
    char * temp_str = lex->file_text;
    regmatch_t * m = lex->match;
    uint valid_count = 0;
    int token_id;

    while (valid_count < lookahead) {
        memset(m, -1, sizeof(regmatch_t) * NUMBER_OF_TOKENS); 

        if (regexec(&lex->regex, temp_str, 25, m, 0) != 0) 
            return -1;
            
        uint valid = !(m[24].rm_so != -1 || m[25].rm_so != -1);
        if (valid) 
            valid_count++;

        temp_str += m[0].rm_eo - m[0].rm_so;
    }
    printf("%s\n", temp_str);

    if (m[1].rm_so != -1)  return SEMI;
    if (m[2].rm_so != -1)  return ASSIGN;
    if (m[3].rm_so != -1)  return LPAR;
    if (m[4].rm_so != -1)  return RPAR;
    if (m[5].rm_so != -1)  return LWING;
    if (m[6].rm_so != -1)  return RWING;
    if (m[7].rm_so != -1)  return LBRACKET;
    if (m[8].rm_so != -1)  return RBRACKET;
    if (m[9].rm_so != -1)  return ADD;
    if (m[10].rm_so != -1) return SUB;
    if (m[11].rm_so != -1) return MUL;
    if (m[12].rm_so != -1) return DIV;
    if (m[13].rm_so != -1) return MOD;
    if (m[14].rm_so != -1) return EQ;
    if (m[15].rm_so != -1) return NEQ;
    if (m[16].rm_so != -1) return LT;
    if (m[17].rm_so != -1) return LEQ;
    if (m[18].rm_so != -1) return GT;
    if (m[19].rm_so != -1) return GEQ;
    if (m[20].rm_so != -1) return IF;
    if (m[21].rm_so != -1) return WHILE;
    if (m[22].rm_so != -1) return ID;
    if (m[23].rm_so != -1) return NUM;
    return -1;
}

token_t * take_token(lexer_t * lex) { 
    // @DEBUG
    int token_id = -1;
    char * s;
    int value = 0;
    regmatch_t * m = lex->match;


    uint invalid = 1;    // As in not newline or whitespace
    while (invalid) {
        memset(m, -1, sizeof(regmatch_t) * NUMBER_OF_TOKENS); 
        
        if (regexec(&lex->regex, lex->file_text, 25, m, 0) != 0) {
            printf("Parse error.\n");
            exit(EXIT_FAILURE);
        }
        
        invalid = (m[24].rm_so != -1 || m[25].rm_so != -1);
        if (invalid) {
            if (m[24].rm_so != -1) {
                lex->col += m[0].rm_eo - m[0].rm_so;
            } else if (m[25].rm_so != -1) {
                lex->line++;
                lex->col = 1;
            }
            lex->file_text += m[0].rm_eo - m[0].rm_so;
        }
    }

    int token_len = m[0].rm_eo - m[0].rm_so;
    if (m[1].rm_so != -1) { 
        token_id = SEMI;
    }
    else if (m[2].rm_so != -1) { 
        token_id = ASSIGN;
    }
    else if (m[3].rm_so != -1) { 
        token_id = LPAR;
    }
    else if (m[4].rm_so != -1) { 
        token_id = RPAR;
    }
    else if (m[5].rm_so != -1) { 
        token_id = LWING;
    }
    else if (m[6].rm_so != -1) { 
        token_id = RWING;
    }
    else if (m[7].rm_so != -1) { 
        token_id = LBRACKET;
    }
    else if (m[8].rm_so != -1) { 
        token_id = RBRACKET;
    }
    else if (m[9].rm_so != -1) {
        token_id = ADD;
    }
    else if (m[10].rm_so != -1) {
        token_id = SUB;
    }
    else if (m[11].rm_so != -1) {
        token_id = MUL; 
    }
    else if (m[12].rm_so != -1) {
        token_id = DIV;
    }
    else if (m[13].rm_so != -1) {
        token_id = MOD;
    }
    else if (m[14].rm_so != -1) {
        token_id = EQ;
    }
    else if (m[15].rm_so != -1) {
        token_id = NEQ;
    }
    else if (m[16].rm_so != -1) {
        token_id = LT;
    }
    else if (m[17].rm_so != -1) {
        token_id = LEQ;
    }
    else if (m[18].rm_so != -1) {
        token_id = GT;
    }
    else if (m[19].rm_so != -1) {
        token_id = GEQ;
    }
    else if (m[20].rm_so != -1) {
        token_id = IF;
    }
    else if (m[21].rm_so != -1) {
        token_id = WHILE;
    }
    else if (m[22].rm_so != -1) { 
        token_id = ID;
        s = malloc(token_len + 1); 
        for (int i = 0; i < token_len; i++) 
            s[i] = lex->file_text[i];  // Optimize!!!
        
        s[token_len] = '\0';
    }
    else if (m[23].rm_so != -1) { 
        token_id = NUM;
        char * value_s = malloc(token_len + 1);
        for (int i = 0; i < token_len; i++) 
            value_s[i] = lex->file_text[i];  // Optimize!!!!    

        value_s[token_len] = '\0';
        value = atoi(value_s);
    }
    else if (m[24].rm_so != -1) {
        token_id = WHITESPACE;
        lex->col += m[0].rm_eo - m[0].rm_so;
    }
    else if (m[25].rm_so != -1) {
        token_id = NEWLINE;
        lex->line++;
        lex->col = 1;
    }
    
    token_t * t = init_token(token_id, lex->line, lex->col, s, value);
    lex->file_text += m[0].rm_eo - m[0].rm_so;
    lex->col += m[0].rm_eo - m[0].rm_so;       // recheck that this works with changes to invalid token, unit tests would be nice here
    return t;
}


void test_tokens() {
    char * file_text = "if (abs == 10)                print(10000); while (list) {a[]}";
    regex_t regex;
    regmatch_t m[26];
    regcomp(&regex, rules, REG_EXTENDED);
    lexer_t * lex = init_lexer(file_text, regex, m);
    printf("%s\n", lex->file_text);


    for (uint i = 1; i < 20; i++) {
        token_t * t = take_token(lex);
        if (t->token_id == NUM){
            printf("%s: ", token_to_str(NUM));
            printf("%d\n", t->value);
        } else if (t->token_id == ID){
            printf("%s: ", token_to_str(ID));
            printf("%s\n", t->str);
        } else {
            printf("%s\n", token_to_str(t->token_id));
        }
    }
}

// int main() {
//     return 0;    
// }

