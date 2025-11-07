#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>


//NEXT:


//TODO: 
//Create a parse_expr() 
//Create a parse_block() and parse_stmt()
//Make regex+filepos into a struct or make them global so I don't have to pass them around everywhere
//Create parser using peek token
//Figure out how to parse non-exp strings
//Figure out top-down operator precedence parsing


typedef struct {
    int token_id;
    int line;
    int col;

    char * str;         // union for str + value (?)
    int value;
} token_t;

typedef struct {
    int line;
    int col;
} file_position_t;




typedef enum {
    ID, NUM, ASSIGN, SEMI, LPAR, RPAR,
    LWING, RWING, LBRACKET, RBRACKET,
    ADD, SUB, MUL, DIV, MOD, EQ, NEQ,
    LT, LEQ, GT, GEQ, IF, WHILE, WHITESPACE,
    NEWLINE, 
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

char * token_to_str(token_id_enum t) {
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
        case WHITESPACE: return "WHITESPACE";
        case NEWLINE: return "NEWLINE";
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

token_t * init_token(int token_id, int line, int col, char * str, int value) {
    token_t * t = malloc(sizeof(token_t));
    t->token_id = token_id;
    t->line = line;
    t->col = col;

    t->str = str;
    t->value = value;
    return t;
}

token_id_enum peak_next_token(char ** str, regex_t regex, regmatch_t * m) { 
    // memoize last peak, pass amount of the number of peaks forward of this one
    // This for checking n values forward. memoize how though?
    if (regexec(&regex, *str, 25, m, 0) == 0) {
        if (m[1].rm_so != -1)  return SEMI;
        else if (m[2].rm_so != -1)  return ASSIGN;
        else if (m[3].rm_so != -1)  return LPAR;
        else if (m[4].rm_so != -1)  return RPAR;
        else if (m[5].rm_so != -1)  return LWING;
        else if (m[6].rm_so != -1)  return RWING;
        else if (m[7].rm_so != -1)  return LBRACKET;
        else if (m[8].rm_so != -1)  return RBRACKET;
        else if (m[9].rm_so != -1)  return ADD;
        else if (m[10].rm_so != -1)  return SUB;
        else if (m[11].rm_so != -1)  return MUL;
        else if (m[12].rm_so != -1)  return DIV;
        else if (m[13].rm_so != -1)  return MOD;
        else if (m[14].rm_so != -1)  return EQ;
        else if (m[15].rm_so != -1)  return NEQ;
        else if (m[16].rm_so != -1)  return LT;
        else if (m[17].rm_so != -1)  return LEQ;
        else if (m[18].rm_so != -1)  return GT;
        else if (m[19].rm_so != -1)  return GEQ;
        else if (m[20].rm_so != -1)  return IF;
        else if (m[21].rm_so != -1)  return WHILE;
        else if (m[22].rm_so != -1)  return ID;
        else if (m[23].rm_so != -1)  return NUM;
        else if (m[24].rm_so != -1)  return WHITESPACE;
        else if (m[25].rm_so != -1)  return NEWLINE;
    }
}




token_t * take_next_token(char ** str, regex_t regex, regmatch_t * m, file_position_t * file_pos) { 
    int token_id = -1;
    char * s = NULL;
    int value = 0;


    if (regexec(&regex, *str, 25, m, 0) == 0) {
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
        }
        else if (m[25].rm_so != -1) {
            token_id = NEWLINE;
            file_pos->line++;
            file_pos->col = 1;
        }
    }

    token_t * t = init_token(token_id, file_pos->line, file_pos->col, s, value);
    *str += m[0].rm_eo - m[0].rm_so;
    if (token_id != NEWLINE)
        file_pos->col += m[0].rm_eo - m[0].rm_so;
    return t;
}




int test_tokens() {
    file_position_t file_pos = {1, 1};

    //set up regex
    regex_t regex;
    regmatch_t m[26];
    regcomp(&regex, rules, REG_EXTENDED);

    char * file = "if (abs == 10) \n print(10000); while (list) {a[]}";
    token_t * t;
    for (int i = 0; i < 20; i++) {
        t = take_next_token(&file, regex, m, &file_pos); 
        printf("%s: ", token_to_str(t->token_id));
        printf("line %d, col %d\n", t->line, t->col);
        // printf("line %d, col %d\n", file_pos.line, file_pos.col);
        // if (t->token_id == NEWLINE) 
        //     printf("NEWLINE");

        
        // if (t->token_id == ID) {
        //     printf(": %s\n", t->str);
        // } else if (t->token_id == NUM) {
        //     printf(": %d\n", t->value);
        // }
    }
    
    printf("\nEXIT SUCCESS\n");
    exit(EXIT_SUCCESS);
}



void parse_if(char ** str, regex_t regex, regmatch_t * m, file_position_t * file_pos) { 
    //previous token should have been an IF for this to be called.
    token_t * t;
    t = take_next_token(&file, regex, m, &file_pos);
    if (t->token_id != LPAR) print_lex_error(t);

    // !!! parse_expression() call !!!

    t = take_next_token(&file, regex, m, &file_pos);
    if (t->token_id != RPAR) print_lex_error(t);

    t = take_next_token(&file, regex, m, &file_pos);
    if (t->token_id != LBRACKET) print_lex_error(t);
    
    // !!! parse_block() call !!!
    
    t = take_next_token(&file, regex, m, &file_pos);
    if (t->token_id != RBRACKET) print_lex_error(t);

    //@TODO figure out how else and else if works here
}


void print_lex_error(int line, int col) {
    printf("Incorrect token at line: %d, col: %d \n", line, col);
    exit(EXIT_FAILURE);
}


void parse_func_decl() {
    token_t type = take_next_token() 
    token_t func_name = take_next_token()
    take_next_token() // LPAR

    // probably shouldn't include newlines and whitespaces 
    if (peak_next_token != RPAR || peak_next_token != WHITESPACE || peak_next_token != WHITESPACE) { 
        parse_expression()
        while (peak_next_token != RPAR) {
            take_next_token(); // COMMA, oh fuck I need to add comma.
            // add a consume next token for tokens that don't create ast nodes? And maybe pass the expected node
            parse_expression();
        }    
    }

    parse_block();
    

}
void parse_block() {
    take_next_token(); //LBRACKET
    while (peak_next_token != RBRACKET) {
        parse_stmt();
    }
    take_next_token(); //RBRACKET
}


void parse_stmt() {
    switch (peak_next_token()) {
        case IF:
            parse_if();
            break;
        case WHILE:
            parse_while();
            break;
        case FOR:
            parse_for();
            /* code */
            break;
        case RETURN:
            parse_return()
            /* code */
            break;
        case ID:
            switch (peak_next_token()) { // this won't work due to how peak_next_token works
                case LPAR:
                    
                    break;
                case :
                    /* code */
                    break;
                case :
                    /* code */
                    break;
                
                default:
                    break;
            }
            parse_func_call_stmt();            
            parse_assign();
            parse_var_decl();
            /* code */
            break;
        
        default:
            break;
    }    

    // types of stmts:
    // func call stmt
    // while
    // for
    // assignment
    // variable declaration
    // return 





    // this function needs to contain at least the entry into all forms of stmt, such as if and func_call_stmt, decl etc;
    // We know that the next statement should be a function  (???) at least in the case of an if STMT.
}

// have a stmt type, expr type etc?
// how do I handle a tree of different pointers?? 
// I'd guess stmt, expr structs. etc




//represent if_stmt types with a union in the block above ??????
// block should probably just have an array of pointers just.

typedef struct {
    expr_t * expr;
    block_stmt_t * stmts;
} if_stmt_t;

typedef struct {
    expr_t * expr;
    block_stmt_t * stmts1;
    block_stmt_t * stmts2;
} if_else_stmt_t;       


// types of expression: 
// ID
// func call()
// binops (logical and arithmetic)
// unary minus
