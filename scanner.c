#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>

//TODO: 
//Create structs for all tokens, i.e ID struct with string, NUM with number etc.
//Make peekToken method();
//Create parser using peek token
//Figure out how to parse non-exp strings
//Figure out top-down operator precedence parsing



typedef enum {
    ID,
    NUM,
    ASSIGN,
    SEMI,
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
    WS,
} Token;


char * enum_token_to_str(Token t) {
    switch (t) {
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
        case WS: return "WS";
        default: return "NONE";
    }
}

#define MAX(a, b) ((a) > (b) ? (a) : (b))

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

void tokenize(String * s, Token * tokens) {

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
    "|([ \n]+)";

    char c;
    int t_i = 0;
    
    Token * t = malloc(1000 * sizeof(Token));
    
    
    regex_t regex;
    regcomp(&regex, rules, REG_EXTENDED);
    
    for (int i = 0; i < s->len; i++) {        
        regmatch_t m[25];
        if (regexec(&regex, s->str, 25, m, 0) == 0) {
            if (m[1].rm_so != -1) { 
                printf("SEMI\n");
                tokens[t_i] = SEMI; 
            }
            else if (m[2].rm_so != -1) { 
                printf("ASSIGN\n");
                tokens[t_i] = ASSIGN; 
            }
            else if (m[3].rm_so != -1) { 
                printf("LPAR\n");
                tokens[t_i] = LPAR; 
            }
            else if (m[4].rm_so != -1) { 
                printf("RPAR\n");
                tokens[t_i] = RPAR; 
            }
            else if (m[5].rm_so != -1) { 
                printf("LWING\n");
                tokens[t_i] = LWING; 
            }
            else if (m[6].rm_so != -1) { 
                printf("RWING\n");
                tokens[t_i] = RWING; 
            }
            else if (m[7].rm_so != -1) { 
                printf("LBRACKET\n");
                tokens[t_i] = LBRACKET; 
            }
            else if (m[8].rm_so != -1) { 
                printf("RBRACKET\n");
                tokens[t_i] = RBRACKET; 
            }
            else if (m[9].rm_so != -1) {
                printf("ADD\n");
                tokens[t_i] = ADD;
            }
            else if (m[10].rm_so != -1) {
                printf("SUB\n");
                tokens[t_i] = SUB;
            }
            else if (m[11].rm_so != -1) {
                printf("MUL\n");
                tokens[t_i] = MUL;
            }
            else if (m[12].rm_so != -1) {
                printf("DIV\n");
                tokens[t_i] = DIV;
            }
            else if (m[13].rm_so != -1) {
                printf("MOD\n");
                tokens[t_i] = MOD;
            }
            else if (m[14].rm_so != -1) {
                printf("EQ\n");
                tokens[t_i] = EQ;
            }
            else if (m[15].rm_so != -1) {
                printf("NEQ\n");
                tokens[t_i] = NEQ;
            }
            else if (m[16].rm_so != -1) {
                printf("LT\n");
                tokens[t_i] = LT;
            }
            else if (m[17].rm_so != -1) {
                printf("LEQ\n");
                tokens[t_i] = LEQ;
            }
            else if (m[18].rm_so != -1) {
                printf("GT\n");
                tokens[t_i] = GT;
            }
            else if (m[19].rm_so != -1) {
                printf("GEQ\n");
                tokens[t_i] = GEQ;
            }
            else if (m[20].rm_so != -1) {
                printf("IF\n");
                tokens[t_i] = IF;
            }
            else if (m[21].rm_so != -1) {
                printf("WHILE\n");
                tokens[t_i] = WHILE;
            }
            else if (m[22].rm_so != -1) { 
                printf("ID\n");
                tokens[t_i] = ID; 
            }
            else if (m[23].rm_so != -1) { 
                printf("NUM\n");
                tokens[t_i] = NUM; 
            }
            else if (m[24].rm_so != -1) {
                printf("WS\n");
                t_i--;
                // tokens[t_i] = WS;
            }
        }
        s->str = s->str + (m[0].rm_eo - m[0].rm_so);
        printf("%s\n", s->str);
        if (m[0].rm_eo - m[0].rm_so == 0) break; 
        
        t_i++;
    }
}

int main() {
    String * s = malloc(sizeof(String));
    Token * t = malloc(1000 * sizeof(Token));

    init_string(s, 100);
    
    
    char * file = "if (abs == 10) print(10000); while (list) {a[]}";
    string_cpy(s, file, str_len(file));
    tokenize(s, t);
    printf("%s\n", file);
    for (int i = 0; i < 20; i++) {
        printf("%s ", enum_token_to_str(t[i]));
    }
    printf("\n");
    printf("EXIT SUCCESS\n");
    exit(EXIT_SUCCESS);
}