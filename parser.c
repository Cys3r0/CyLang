#include <scanner.h> // error is due to regex.h wsl thing












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
//
//
//

// I feel like I need to do some research
// How should a func call be represented/parsed?   
// Perhaps make a prototype top-down operator precedence parser work for only IDs?
// Decide on priorities. 


u8 is_binary_operator(token_t * tok) {
    //works for now
    return tok->token_id == ADD
            || tok->token_id == MUL
            || tok->token_id == SUB
            || tok->token_id == DIV
            || tok->token_id == MOD   
}


void parse_expr(char ** file, lexer_t lex) {
    // to begin with, work with a * b + c, no parens.
    token_t * current_tok = take_next_token(file, NUM, lex);
    token_t * t2 = take_next_token(file, void, lex);
    
    if (is_binary_operator(peak_next_token())) { // Clean this up later.
        
    }

    
    
    

}




void parse_assign() {
    
}

void parse_var_decl(char ** str, lexer_t lexer) {
    take_next_token(&file, ID, lexer); 
    take_next_token(&file, ID, lexer); 
    if (peak_next_token(&file, 1, lexer) == ASSIGN){

        take_next_token(&file, ASSIGN, lexer);
        // !!! parse_expression() call !!!
    }
    take_next_token(&file, SEMI, lexer); 
}


void parse_if(char ** str, lexer_t lexer) { 
    take_next_token(&file, IF, lexer); 
    take_next_token(&file, LPAR, lexer); 
    
    // !!! parse_expression() call !!!
    
    take_next_token(&file, RPAR, lexer); 
    take_next_token(&file, LBRACKET, lexer); 
    
    // !!! parse_block() call !!!
    
    take_next_token(&file, RBRACKET, lexer); 
    if (peak_next_token(&file, 1, lexer) == ELSE) {
        take_next_token(&file, ELSE, lexer); 
        take_next_token(&file, LBRACKET, lexer);

        // !!! parse_block() call !!!

        take_next_token(&file, RBRACKET, lexer); 
    }
}




void parse_func_decl() {
    token_t type = take_next_token() 
    token_t func_name = take_next_token()
    take_next_token() // LPAR

    // probably shouldn't include newlines and whitespaces 
    // remember to memoize the parsed tokens somehow? or perhaps that is premature opt.
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
            break;
        case RETURN:
            parse_return();
            break;
        case ID:
            switch (peak_next_token(2)) { 
                // this won't work due to how peak_next_token works
                // and newlines/whitespaces can't be allowed if this is the strategy
                case LPAR:
                    parse_func_call_stmt();
                    break;
                case ASSIGN:
                    parse_assign();
                    break;
                case ID:
                    parse_var_decl();
                    break;
                
                default:
                    break;
            }
            parse_func_call_stmt();            
            parse_assign();
            parse_var_decl();
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


// statement structs

typedef struct {
    expr_t * t;
} func_call_stmt_t;

typedef struct {
    token_t * type;
    token_t * identifier;
    expr_t * expr;
} assign_stmt_t;

typedef struct {
    void * stmts;
} block_stmt_t;

typedef struct {
    expr_t * expr;
    block_stmt_t * stmts;
} while_stmt_t;

typedef struct {
    expr_t * expr_if;
    block_stmt_t * then_block;
    block_stmt_t * else_block;      //Could be NULL
} if_stmt_t; 

typedef struct { 
    expr_t * expr;
} return_stmt_t; 




// expressions

typedef struct {
    bool parenthesis;
    operator_t op;
    expr_t * left;
    expr_t * right;
} bin_op_t;




parse_expression() {
    // IDK this whole thing should calla recursive descent thing.
    switch (peak_next_token()) {
        case ID:
            if (peak_next_token() == LPAR) 
                parse_func_call();
            
            //else create a new ID thing
                 

            break;
        case NUM:
            
            break;
        case LPAR:
            // keep count of number of LPARs so we can know? 

            break;

        default:
            break;
    }
}

// types of expression: 
// ID
// func call()
// binops (logical and arithmetic)
// unary minus
