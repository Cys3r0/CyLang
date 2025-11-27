#include "scanner.h" // error is due to regex.h wsl thing
#include <stdlib.h>
#include <stdio.h>

// a * b + c
//
// Below is what we want.
//
//          +
//         / \
//        *   c
//       / \
//      a   b
// 
// takes a, takes *, takes b, create *binop struct if + has higher precedence than *, set + left pointer to *. 
// for a + b * c
// take a, take +, b, create +binop struct, 

// in the case of a * b, we want:
//
//        *   
//       / \
//      a   b
// 
// I feel like I need to do some research
// How should a func call be represented/parsed?   
// Perhaps make a prototype top-down operator precedence parser work for only IDs?
// Decide on priorities. 


int is_binary_operator(int token_id) {
    //works for now
    int ret = token_id == ADD
            || token_id == MUL
            || token_id == SUB
            || token_id == DIV
            || token_id == MOD;
    printf("is_binary_operator: %d\n", ret);
    return ret;
}

int precedence_of(int token_id) {
    switch (token_id){
        case ADD: return 1;
        case SUB: return 1;
        case MUL: return 2;
        case DIV: return 2;
        case MOD: return 2;
        default: return -1;
    }
    
}

enum ExprType { BINOP, ATOM };
typedef struct expr expr_t;


typedef struct {
    int op;
    expr_t * left; 
    expr_t * right;
} binop_t;


struct expr {
    enum ExprType tag;
    union {
        binop_t binop;
        token_t token;
    } data;
};


//TODO: 
//Create pretty print expr function.
//test for simple arithmetic
//add right-associative operations
//add parentheses
//add unary ops
//add IDs and func calls.
//HERE FIX LOOKAHEAD FOR LEXER
//Create stmt tagged union?
//fix parse statement 

char * tag_to_str(enum ExprType tag) {
    switch (tag) {
    case ATOM: return "ATOM";
    case BINOP: return "BINOP";
    default: return "ERROR: NOT ATOM OR BINOP";
    }
}

expr_t * create_binop_expr(int op, expr_t * left, expr_t * right) {
    binop_t bin;
    bin.op = op;
    bin.left = left;
    bin.right = right;
    
    expr_t * exp = malloc(sizeof(expr_t));
    exp->tag = BINOP;
    exp->data.binop = bin;
    return exp;
}


expr_t * create_atom_expr(token_t * tok) {
    expr_t * exp = malloc(sizeof(expr_t));
    exp->tag = ATOM;
    exp->data.token = *tok;
    return exp;
}


expr_t * parse_expr_recursive(expr_t * lhs, int precedence, lexer_t * lex, int * lookahead, int count) {
    count++;
    printf("Count: %d\n", count);
    //pratt parsing pseudocode from wikipedia
    *lookahead = peak_token(1, lex);
    printf("%s\n", token_to_str(*lookahead));
    // while loop to continue 
    while (is_binary_operator(*lookahead) && precedence_of(*lookahead) > precedence) { // Clean this up later.

        int op = take_token(lex)->token_id; 
        expr_t * rhs = create_atom_expr(take_token(lex)); // ?check if take_token is numeral?

        *lookahead = peak_token(1, lex);
        precedence = precedence_of(*lookahead);
        // printf("%d\n", precedence);        
        // printf("%d\n", precedence);
        // "or a right-associative operator whose precedence is equal to op's." 
        // in this case we also need to increment the precedence of op passed into the recursion


        while (is_binary_operator(*lookahead) && precedence_of(*lookahead) > precedence) {
            printf("Has recursed\n");
            rhs = parse_expr_recursive(rhs, precedence_of(op), lex, lookahead, count);
            printf("RHS within recur: %s\n", tag_to_str(rhs->tag));
            printf("Finished recursion\n");
        }

        lhs = create_binop_expr(op, lhs, rhs);
    }
    return lhs;
}

expr_t * parse_expr(lexer_t * lex) {
    int lookahead;
    return parse_expr_recursive(create_atom_expr(take_token(lex)), 0, lex, &lookahead, 0);
}


    


int main(int argc, char const *argv[])
{
    char * file_text = "10 + 2 + 3;";
    regex_t regex;
    regmatch_t m[26];
    regcomp(&regex, rules, REG_EXTENDED);
    lexer_t * lex = init_lexer(file_text, regex, m);
    printf("%s\n", lex->file_text);

    expr_t * e = parse_expr(lex);
    int cond = 1;
    if (e->tag == ATOM) {
        printf("ATOM\n");
    } else if (e->tag == BINOP) {
        printf("BINOP\n");
    }
    

    return 0;
}

void print_expr(expr_t * expr) {
    if (expr->tag == ATOM) {
        // expr->data.token->token
        print(expr->data.token->token)
    }



}





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
