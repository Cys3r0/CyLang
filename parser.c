#include "scanner.h" // error is due to regex.h wsl thing
#include <stdlib.h>
#include <stdio.h>

#define MAX_ARGS 64

enum ExprType { EXPR_BINOP, EXPR_UNARY, EXPR_FUNC_CALL, EXPR_NUMERAL, EXPR_ID };

int is_binop(enum TokenType token_type) {
    //works for now
    int ret = token_type == TOKEN_ADD
            || token_type == TOKEN_MUL
            || token_type == TOKEN_SUB
            || token_type == TOKEN_DIV
            || token_type == TOKEN_MOD
            || token_type == TOKEN_EXPONENT;
    return ret;
}

int is_right_associative(enum TokenType token_type) {
    //works for now
    int ret = token_type == TOKEN_EXPONENT;
    return ret;
}

int is_unary(enum TokenType token_type) {
    //works for now
    int ret = token_type == TOKEN_SUB
            || token_type == TOKEN_ADD;
    return ret;
}

int is_atom(enum ExprType type) {
    return type == EXPR_NUMERAL
            || type == EXPR_ID
            || type == EXPR_FUNC_CALL;
}

int precedence_of(enum TokenType token_type) {
    switch (token_type){
        case TOKEN_ADD:      return 1;
        case TOKEN_SUB:      return 1;
        case TOKEN_MUL:      return 2;
        case TOKEN_DIV:      return 2;
        case TOKEN_MOD:      return 2;
        case TOKEN_EXPONENT: return 3;
        default: return -1;
    }
}

typedef struct expr expr_t;

typedef struct {
    enum TokenType op;
    expr_t * inner;
} unary_t;

typedef struct {
    enum TokenType op;
    expr_t * left; 
    expr_t * right;
} binop_t;

typedef struct {
    token_t * func_id;
    expr_t ** args;
    int arg_len;
} expr_func_call_t;

struct expr {
    enum ExprType tag;
    union {
        binop_t binop;
        unary_t unary;
        token_t id;
        token_t numeral;
        expr_func_call_t func_call;
    };
};


//TODO: 
//create a stmt type
//test parser for the input ()
//create a if, block, while, etc stmts.
//skip_token take wrapper? 

// LATER:
//Figure out how to propagate error messages
//use a ¤ as a pointer deref.
//create unit tests for scanner/parser
//implement pretty print for exprs
//test calling functions within functions.


char * expr_tag_to_str(enum ExprType tag) {
    switch (tag) {
        case EXPR_UNARY:     return "EXPR_UNARY";
        case EXPR_FUNC_CALL: return "EXPR_FUNC_CALL";
        case EXPR_NUMERAL:   return "EXPR_NUMERAL";
        case EXPR_ID:        return "EXPR_ID";
        case EXPR_BINOP:     return "EXPR_BINOP";
        default:             return "ERROR: NOT ATOM OR EXPR_BINOP";
    }
}

expr_t * create_binop_expr(enum TokenType op, expr_t * left, expr_t * right) {
    binop_t bin;
    bin.op = op;
    bin.left = left;
    bin.right = right;
    
    expr_t * expr = malloc(sizeof(expr_t));
    expr->tag = EXPR_BINOP;
    expr->binop = bin;
    return expr;
}

expr_t * create_atom_expr(token_t * tok) {
    expr_t * exp = malloc(sizeof(expr_t));
    if (tok->token_type == TOKEN_ID) {
        exp->tag = EXPR_ID;
        exp->id = *tok;

    } else if (tok->token_type == TOKEN_NUM) {
        exp->tag = EXPR_NUMERAL;
        exp->numeral = *tok;
    }

    return exp;
}


expr_t * parse_expr(lexer_t * lex);

expr_t * parse_expr_func_call(token_t * func_id, lexer_t * lex) {
    int arg_len = 0;
    take_token(lex); // LPAR
    token_t * next;
    expr_t ** args = NULL;
    
    if (peak_token(lex) != TOKEN_RPAR) {
        args = malloc(MAX_ARGS * sizeof(expr_t *));
        expr_t * first_arg = parse_expr(lex);
        
        args[arg_len] = first_arg;
        arg_len++;
        
        while ((next = take_token(lex))->token_type == TOKEN_COMMA) {
            if (arg_len >= MAX_ARGS) {
                printf("Function call may not exceed MAX_ARGS arguments.");
                exit(EXIT_FAILURE);
            }
            
            args[arg_len] = parse_expr(lex);
            arg_len++;
        }
    } else {
        next = take_token(lex);
    }
    
    if (next->token_type != TOKEN_RPAR) 
        printf("Did not take \")\" as last token\n");

    expr_func_call_t func_call;
    func_call.func_id = func_id;
    func_call.args = args;
    func_call.arg_len = arg_len;

    expr_t * exp = malloc(sizeof(expr_t));
    exp->tag = EXPR_FUNC_CALL;
    exp->func_call = func_call;

    return exp;
}

expr_t * parse_expr_atom(token_t * next, lexer_t * lex) {
    expr_t * exp = malloc(sizeof(expr_t));

    if (next->token_type == TOKEN_ID) {
        if (peak_token(lex) == TOKEN_LPAR) {
            exp = parse_expr_func_call(next, lex);
        } else {
            exp->tag = EXPR_ID;
            exp->id = *next;
        }
    } else if (next->token_type == TOKEN_NUM) {
        exp->tag = EXPR_NUMERAL;
        exp->numeral = *next;
    }
    
    return exp;
}

expr_t * parse_expr_paran(lexer_t * lex) {
    expr_t * inner = parse_expr(lex);
    take_token(lex); //Takes RPAR
    return inner;
}

expr_t * parse_expr_unary(enum TokenType tok_type, lexer_t * lex) {
    expr_t * inner;
    token_t * next = take_token(lex);
    if (next->token_type == TOKEN_LPAR) {
        inner = parse_expr_paran(lex);
    } else if (is_unary(next->token_type)){
        inner = parse_expr_unary(tok_type, lex);
    } else {
        // this should be a parse atom call instead or something
        inner = create_atom_expr(next);
    }

    unary_t unary;
    unary.op = tok_type;
    unary.inner = inner;
    
    expr_t * exp = malloc(sizeof(expr_t));
    exp->tag = EXPR_UNARY;
    exp->unary = unary;

    return exp;
}


expr_t * parse_expr_operand(token_t * next, lexer_t * lex) {
    expr_t * operand;

    if (next->token_type == TOKEN_LPAR) {
        operand = parse_expr_paran(lex);
    } else if (is_unary(next->token_type)) {
        operand = parse_expr_unary(next->token_type, lex);
    } else {
        operand = parse_expr_atom(next, lex);    
    }

    return operand;
}

expr_t * parse_expr_recursive(expr_t * lhs, int min_precedence, lexer_t * lex, enum TokenType * lookahead) {
    // pratt parsing pseudocode from wikipedia
    int right_assoc = 0;

    while (is_binop(*lookahead) && precedence_of(*lookahead) >= min_precedence) {
        enum TokenType op = take_token(lex)->token_type; 
        token_t * next = take_token(lex);
        expr_t * rhs;

        // This should be a parse_operand function
        rhs = parse_expr_operand(next, lex);
        
        *lookahead = peak_token(lex);

        while ((is_binop(*lookahead) && precedence_of(*lookahead) > precedence_of(op)) ||
                (is_right_associative(*lookahead) && (right_assoc = precedence_of(*lookahead) == precedence_of(op)))) {

            int inc = (right_assoc) ? 0 : 1;
            rhs = parse_expr_recursive(rhs, precedence_of(op) + inc, lex, lookahead);
        }

        lhs = create_binop_expr(op, lhs, rhs);
    }
    return lhs;
}

expr_t * parse_expr(lexer_t * lex) {
    token_t * next = take_token(lex);
    expr_t * expr;

    expr = parse_expr_operand(next, lex);
    enum TokenType lookahead = peak_token(lex);
    return parse_expr_recursive(expr, 0, lex, &lookahead);
}

void print_expr_recursive(expr_t * expr, int level) {
    if (expr->tag == EXPR_FUNC_CALL) {
        printf("L%d func_call: %s(", level, expr->func_call.func_id->str);
        if (expr->func_call.arg_len > 0) {
            printf("%d", expr->func_call.args[0]->numeral.value);
            for (int i = 1; i < expr->func_call.arg_len; i++) {
                printf(", %d", expr->func_call.args[i]->numeral.value);
            }
        }    
        printf(")\n");
    } else if (expr->tag == EXPR_NUMERAL) {
        printf("L%d atom: %s = %d\n", level, token_to_str(expr->numeral.token_type), expr->numeral.value);
    } else if (expr->tag == EXPR_ID) {
        printf("L%d atom: %s = %s\n", level, token_to_str(expr->id.token_type), expr->id.str);
    } else if (expr->tag == EXPR_BINOP) {
        printf("L%d binop: %s\n", level, token_to_str(expr->binop.op));
        print_expr_recursive(expr->binop.right, level+1);
        print_expr_recursive(expr->binop.left, level+1);
    } else if (expr->tag == EXPR_UNARY) {
        printf("L%d unary: %s\n", level, token_to_str(expr->unary.op));
        print_expr_recursive(expr->unary.inner, level+1);
    }
}

void print_expr(expr_t * expr) {
    print_expr_recursive(expr, 0);
}


int main(int argc, char const *argv[]) {
    char * file_text = "();";

    regex_t regex;
    regmatch_t m[NUMBER_OF_TOKENS + 1];
    regcomp(&regex, REGEX_RULES, REG_EXTENDED);
    lexer_t * lex = init_lexer(file_text, regex, m);
    printf("%s\n", lex->file_text);
    
    for (int i = 0; i < 9; i++) {
        printf("%s ", token_to_str(peak_n_tokens(i + 1, lex)));
    }
    printf("\n");
    // token_t * next = take_token(lex);
    expr_t * e = parse_expr(lex);

    print_expr(e);
    // printf("arg_len := %d \n", e->func_call.arg_len);


    // expr_t * e = parse_expr(lex);
    // print_expr(e);
    return 0;
}




enum StmtType { STMT_IF, STMT_ID_DECL, STMT_ASSIGN, STMT_FUNC_CALL, STMT_WHILE, STMT_RETURN};
typedef struct stmt stmt_t;

typedef struct {
    expr_t * cond;
    stmt_t ** then;
    stmt_t ** or_else;
} stmt_if_t;

typedef struct {
    token_t * type;
    token_t * variable;
    expr_t * value;
} stmt_id_decl_t;

typedef struct {
    token_t * variable;
    expr_t * value;
} stmt_assign_t;

typedef struct {
    expr_t * cond;
    stmt_t ** block;
} stmt_while_t;

struct stmt {
    enum StmtType tag;
    union {
        stmt_if_t * stmt_if;
        stmt_id_decl_t * stmt_id_decl;
        stmt_assign_t * stmt_assign;
        expr_t * func_call;
        stmt_while_t * stmt_while;
        expr_t * stmt_return;
    };    
};    


stmt_t * parse_stmt(lexer_t * lex);

stmt_t * parse_stmt_func_call(lexer_t * lex) {
    token_t * next = take_token(lex);
    expr_t * func_call = parse_expr_func_call(next, lex);
    take_token(lex); //SEMI
    
    stmt_t * stmt = malloc(sizeof(stmt_t));
    stmt->tag = STMT_FUNC_CALL;
    stmt->func_call = func_call;
    return stmt;
}

stmt_t * parse_stmt_id_decl(lexer_t * lex) {
    token_t * type = take_token(lex);
    token_t * variable = take_token(lex);
    enum TokenType next = take_token(lex)->token_type;
    expr_t * value = NULL;

    if (next == TOKEN_ASSIGN) {
        value = parse_expr(lex);
        take_token(lex); //SEMI
    } else if (next == TOKEN_SEMI) {
        value = NULL;
    }

    stmt_id_decl_t * id_decl = malloc(sizeof(stmt_assign_t));
    id_decl->type = type;
    id_decl->variable = variable;
    id_decl->value = value;

    stmt_t * stmt = malloc(sizeof(stmt_assign_t));
    stmt->tag = STMT_ID_DECL;
    stmt->stmt_id_decl = id_decl;
    return stmt;    
}

stmt_t * parse_stmt_assign(lexer_t * lex) {
    token_t * variable = take_token(lex);
    take_token(lex); // take ASSIGN
    expr_t * value = parse_expr(lex);
    take_token(lex); // take SEMI

    stmt_assign_t * assign = malloc(sizeof(stmt_assign_t));
    assign->variable = variable;
    assign->value = value;

    stmt_t * stmt = malloc(sizeof(stmt_t));
    stmt->tag = STMT_ASSIGN;
    stmt->stmt_assign = assign;
    return stmt;
}

stmt_t ** parse_stmt_block(lexer_t * lex) {
    stmt_t ** block = malloc(300 * sizeof(stmt_t*));
    take_token(lex); //LWING
    int i = 0;
    while(peak_token(lex) != TOKEN_RWING) {
        block[i] = parse_stmt(lex);
        i++;
    }
    take_token(lex); //RWING
    return block;
}

stmt_t * parse_stmt_while(lexer_t * lex) {
    take_token(lex); // WHILE
    take_token(lex); // LPAR
    expr_t * cond = parse_expr(lex);
    take_token(lex); // RPAR

    stmt_t ** block = parse_stmt_block(lex);
       
    stmt_while_t * while_stmt = malloc(sizeof(stmt_while_t));
    while_stmt->cond = cond;
    while_stmt->block = block;

    stmt_t * stmt = malloc(sizeof(stmt_t));
    stmt->tag = STMT_WHILE;
    stmt->stmt_while = while_stmt;
    return stmt;
}

stmt_t * parse_stmt_if(lexer_t * lex) {
    take_token(lex); //IF 
    take_token(lex); //LPAR

    expr_t * cond = parse_expr(lex);

    take_token(lex); //RPAR

    stmt_t ** then = parse_stmt_block(lex);
    stmt_t ** or_else = NULL;

    if (peak_token(lex) == TOKEN_ELSE) {
        take_token(lex); // ELSE

        or_else = parse_stmt_block(lex);

        take_token(lex); 
    }

    stmt_if_t * if_stmt = malloc(sizeof(stmt_while_t));
    if_stmt->cond = cond;
    if_stmt->then = then;
    if_stmt->or_else = or_else;

    stmt_t * stmt = malloc(sizeof(stmt_t));
    stmt->tag = STMT_IF;
    stmt->stmt_if = if_stmt;
    return stmt;
}

stmt_t * parse_stmt_return(lexer_t * lex) {
    take_token(lex); // RETURN
    expr_t * ret_expr = parse_expr(lex);
    take_token(lex); // SEMI

    stmt_t * stmt = malloc(sizeof(stmt_t));
    stmt->tag = STMT_RETURN;
    stmt->stmt_return = ret_expr;
    return stmt;
}


stmt_t * parse_stmt(lexer_t * lex) {
    stmt_t * stmt; 
    switch (peak_token(lex)) {
        case TOKEN_IF:
            stmt = parse_stmt_if(lex);
            break;
        case TOKEN_WHILE:
            stmt = parse_stmt_while(lex);
            break;
        case TOKEN_RETURN:
            stmt = parse_stmt_return(lex);
            break;
        case TOKEN_ID:
            switch (peak_n_tokens(2, lex)) { 
                case TOKEN_LPAR:
                    stmt = parse_stmt_func_call(lex);
                    break;
                case TOKEN_ASSIGN:
                    stmt = parse_stmt_assign(lex);
                    break;
                case TOKEN_ID:
                    stmt = parse_stmt_id_decl(lex);
                    break;
                default: break;
            }
            break;
        default: break;
    }    

    return stmt;
}





// ^^^^^^^^^^^^^^^^ In progress above ^^^^^^^^^^^^^^^^









// void parse_if(char ** str, lexer_t lexer) { 
//     take_next_token(lex); //IF 
//     take_next_token(lex); //LPAR
    
//     parse_expr(lex);
    
//     take_next_token(lex); //RPAR
    
//     // !!! parse_block() call !!!

//     if (peak_next_token(lex) == ELSE) {
//         take_next_token(lex); 
//         take_next_token(lex);

//         // !!! parse_block() call !!!

//         take_next_token(lex); 
//     }
// }



// void parse_assign() {
    
// }

// void parse_var_decl(char ** str, lexer_t lexer) {
//     take_next_token(&file, ID, lexer); 
//     take_next_token(&file, ID, lexer); 
//     if (peak_next_token(&file, 1, lexer) == ASSIGN){

//         take_next_token(&file, ASSIGN, lexer);
//         // !!! parse_expression() call !!!
//     }
//     take_next_token(&file, SEMI, lexer); 
// }


// void parse_if(char ** str, lexer_t lexer) { 
//     take_next_token(&file, IF, lexer); 
//     take_next_token(&file, LPAR, lexer);
    
//     // !!! parse_expression() call !!!
    
//     take_next_token(&file, RPAR, lexer); 
//     take_next_token(&file, LBRACKET, lexer); 
    
//     // !!! parse_block() call !!!
    
//     take_next_token(&file, RBRACKET, lexer); 
//     if (peak_next_token(&file, 1, lexer) == ELSE) {
//         take_next_token(&file, ELSE, lexer); 
//         take_next_token(&file, LBRACKET, lexer);

//         // !!! parse_block() call !!!

//         take_next_token(&file, RBRACKET, lexer); 
//     }
// }




// void parse_func_decl() {
//     token_t type = take_next_token() 
//     token_t func_name = take_next_token()
//     take_next_token() // LPAR

//     // probably shouldn't include newlines and whitespaces 
//     // remember to memoize the parsed tokens somehow? or perhaps that is premature opt.
//     if (peak_next_token != RPAR || peak_next_token != WHITESPACE || peak_next_token != WHITESPACE) { 
//         parse_expression()
//         while (peak_next_token != RPAR) {
//             take_next_token(); // COMMA, oh fuck I need to add comma.
//             // add a consume next token for tokens that don't create ast nodes? And maybe pass the expected node
//             parse_expression();
//         }    
//     }

//     parse_block();
    
// }


// void parse_block() {
//     take_next_token(); //LBRACKET
//     while (peak_next_token != RBRACKET) {
//         parse_stmt();
//     }
//     take_next_token(); //RBRACKET
// }


// void parse_stmt() {
//     switch (peak_next_token()) {
//         case IF:
//             parse_if();
//             break;
//         case WHILE:
//             parse_while();
//             break;
//         case FOR:
//             parse_for();
//             break;
//         case RETURN:
//             parse_return();
//             break;
//         case ID:
//             switch (peak_next_token(2)) { 
//                 case LPAR:
//                     parse_func_call_stmt();
//                     break;
//                 case ASSIGN:
//                     parse_assign();
//                     break;
//                 case ID:
//                     parse_var_decl();
//                     break;
                
//                 default:
//                     break;
//             }
//             parse_func_call_stmt();            
//             parse_assign();
//             parse_var_decl();
//             break;
        
//         default:
//             break;
//     }    
// }

// // have a stmt type, expr type etc?
// // how do I handle a tree of different pointers?? 
// // I'd guess stmt, expr structs. etc


// // statement structs

// typedef struct {
//     expr_t * t;
// } func_call_stmt_t;

// typedef struct {
//     token_t * type;
//     token_t * identifier;
//     expr_t * expr;
// } assign_stmt_t;

// typedef struct {
//     void * stmts;
// } block_stmt_t;

// typedef struct {
//     expr_t * expr;
//     block_stmt_t * stmts;
// } while_stmt_t;

// typedef struct {
//     expr_t * expr_if;
//     block_stmt_t * then_block;
//     block_stmt_t * else_block;      //Could be NULL
// } if_stmt_t; 

// typedef struct { 
//     expr_t * expr;
// } return_stmt_t; 




// // expressions

// typedef struct {
//     bool parenthesis;
//     operator_t op;
//     expr_t * left;
//     expr_t * right;
// } bin_op_t;




// parse_expression() {
//     // IDK this whole thing should calla recursive descent thing.
//     switch (peak_next_token()) {
//         case ID:
//             if (peak_next_token() == LPAR) 
//                 parse_func_call();
            
//             //else create a new ID thing
                 

//             break;
//         case NUM:
            
//             break;
//         case LPAR:
//             // keep count of number of LPARs so we can know? 

//             break;

//         default:
//             break;
//     }
// }

// // types of expression: 
// // ID
// // func call()
// // binops (logical and arithmetic)
// // unary minus
