#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>



//TODO: 
//Priority is to make sure that scanner.c actually works as intended
//Fix the broken peak_next_token
//Figure out top-down operator precedence parsing
// !!! CREATE PARSE EXPRESSION !!! (highest importance)
//make operator precedence work for + - * / first, then add the others later.
//Join file string with lexer struct
//add an expected parameter to take_token, handle error there
//Add EOF to end of any input string
//Decide on tree structure
//Implement parse statement
//Implement parse expressions
//Create a parse_expr() 
//add ELSE, EXPONENT, (LOGICAL:) NOT AND OR, (BITWISE) NOT AND OR XOR BITSHIFTS, EXPONENT, INCREMENT, DECREMENT token
//Create a parse_block() and parse_stmt()
//Make regex+filepos into a struct or make them global so I don't have to pass them around everywhere





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



typedef enum {
    ID, NUM, ASSIGN, SEMI, LPAR, RPAR,
    LWING, RWING, LBRACKET, RBRACKET,
    ADD, SUB, MUL, DIV, MOD, EQ, NEQ,
    LT, LEQ, GT, GEQ, IF, WHILE, WHITESPACE,
    NEWLINE, ELSE,


    VOID,
} token_id_enum;


char * rules = 
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


void resize_string(String * s, int new_cap) {
    char * new_s = realloc(s->str, new_cap);
    if (!new_s) {
        printf("Could not allocate more memory while resizing string.\n");
        exit(EXIT_FAILURE);
    }

    s->str = new_s;
    s->cap = new_cap;
}

void string_add_char(String * s, char c) {
    if (s->len+1 == s->cap) {
        resize_string(s, s->cap << 1);
    }

    s->str[s->len] = c;
    s->str[s->len+1] = '\0';
    s->len++;
}


void string_add_string(String * s1, String * s2) {          // does not free s2.
    if (s1->len + 1 + s2->len + 1 > s1->cap) {
        resize_string(s1, (s1->len + s2->len) << 1);
    }

    for (int i = 0; i < s2->len; i++) {                     // optimize(!)
        s1->str[s1->len + i] = s2->str[i];
    }
    
    s1->len += s2->len;
    s1->str[s1->len] = '\0';
}

void init_string(String * s, int cap) {
    s->cap = cap;
    s->str = malloc(cap);
    s->len = 0;
}
    
    void string_cpy(String * string, char * str, int str_len) {
    if (str_len+1 > string->cap) {
        resize_string(string, str_len << 1);
    }

    for (int i = 0; i < str_len; i++) {                     // optimize(!)
        string->str[i] = str[i];
    }
    string->len = str_len;
}

int str_len(char * str) {
    int i = 0;
    while (str[i] != '\0') {    
        i++;
    }
    
    return i;
}

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

int peak_next_token(char ** str, int lookahead, lexer_t * lex) { 
    // @DEBUG

    // Inefficient, could memoize previously peeked values.
    // Could keep an int value of how far ahead the value has already looked (gloabl variable???)
    // and just regex from there on?

    // Doesn't make sense that the int index is held by the calling function.
    // keep that in the lexer_context
        
    while (1) {
        char * temp_str = lex->regex;
        // hmm this function doesn't work, like at all.
        for (int i = 0; i < lookahead; i++){
            if (regexec(lex->regex, *str, 25, m, 0) != 0) return -1;
        }

        if (m[1].rm_so != -1)       return SEMI;
        else if (m[2].rm_so != -1)  return ASSIGN;
        else if (m[3].rm_so != -1)  return LPAR;
        else if (m[4].rm_so != -1)  return RPAR;
        else if (m[5].rm_so != -1)  return LWING;
        else if (m[6].rm_so != -1)  return RWING;
        else if (m[7].rm_so != -1)  return LBRACKET;
        else if (m[8].rm_so != -1)  return RBRACKET;
        else if (m[9].rm_so != -1)  return ADD;
        else if (m[10].rm_so != -1) return SUB;
        else if (m[11].rm_so != -1) return MUL;
        else if (m[12].rm_so != -1) return DIV;
        else if (m[13].rm_so != -1) return MOD;
        else if (m[14].rm_so != -1) return EQ;
        else if (m[15].rm_so != -1) return NEQ;
        else if (m[16].rm_so != -1) return LT;
        else if (m[17].rm_so != -1) return LEQ;
        else if (m[18].rm_so != -1) return GT;
        else if (m[19].rm_so != -1) return GEQ;
        else if (m[20].rm_so != -1) return IF;
        else if (m[21].rm_so != -1) return WHILE;
        else if (m[22].rm_so != -1) return ID;
        else if (m[23].rm_so != -1) return NUM;
        else if (m[24].rm_so != -1) ;   // WHITESPACE
        else if (m[25].rm_so != -1) ;   // NEWLINE
    }
    
}

token_t * take_next_token(char ** str, int expected, lexer_t * lexer) { 
    // @DEBUG
    int token_id = -1;
    char * s = NULL;
    int value = 0;
    regmatch_t m = lexer->m;

    int invalid_token = 0;    // As in not newline or whitespace

    while (invalid_token) {     // not safe, should terminate on something or other
        if (regexec(&lexer->regex, *str, 25, m, 0) == 0) {
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
                for (int i = 0; i < token_len; i++) {
                    s[i] = (*str)[i];  // Optimize!!!
                }
                s[token_len] = '\0';
            }
            else if (m[23].rm_so != -1) { 
                token_id = NUM;
                char * value_s = malloc(token_len + 1);
                for (int i = 0; i < token_len; i++) 
                value_s[i] = (*str)[i];  // Optimize!!!!    
                value_s[token_len] = '\0';
                value = atoi(value_s);
            }
            else if (m[24].rm_so != -1) {
                token_id = WHITESPACE;
                lexer->col += m[0].rm_eo - m[0].rm_so;
            }
            else if (m[25].rm_so != -1) {
                token_id = NEWLINE;
                lexer->line++;
                lexer->col = 1;
            }
        }
        invalid_token = token_id == WHITESPACE || token_id == NEWLINE;
    }

    if (expected != token_id && expected != VOID) {
        printf("Incorrect token at line: %d, col: %d \n", lex->line, lex->col);
        exit(EXIT_FAILURE);
    }
    
    token_t * t = init_token(token_id, lexer->line, lexer->col, s, value);
    *str += m[0].rm_eo - m[0].rm_so;
    lexer->col += m[0].rm_eo - m[0].rm_so;       // recheck that this works with changes to invalid token, unit tests would be nice here
    return t;
}


int test_tokens() {
    //set up regex
    char * file_text = "if (abs == 10) \n print(10000); while (list) {a[]}";
    regex_t regex;
    regmatch_t m[26];
    regcomp(&regex, rules, REG_EXTENDED);

    lexer_t * lex = init_lexer(file_text, regex, m);


    // token_t * t;
    // for (int i = 0; i < 20; i++) {
    //     t = take_next_token(&file, regex, m, &file_pos); 
    //     printf("%s: ", token_to_str(t->token_id));
    //     printf("line %d, col %d\n", t->line, t->col);
    //     // printf("line %d, col %d\n", file_pos.line, file_pos.col);
    //     // if (t->token_id == NEWLINE) 
    //     //     printf("NEWLINE");

        
    //     // if (t->token_id == ID) {
    //     //     printf(": %s\n", t->str);
    //     // } else if (t->token_id == NUM) {
    //     //     printf(": %d\n", t->value);
    //     // }
    // }
    
    // printf("\nEXIT SUCCESS\n");
    // exit(EXIT_SUCCESS);
}
