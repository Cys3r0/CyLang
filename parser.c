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
// I feel like I need to do some research
// How should a func call be represented/parsed?   
// Perhaps make a prototype top-down operator precedence parser work for only IDs?
// Decide on priorities. 


int is_binary_operator(int token_id) {
    //works for now
    return token_id == ADD
            || token_id == MUL
            || token_id == SUB
            || token_id == DIV
            || token_id == MOD;
}

int precedence_of(int token_id) {
    switch (token_id){
        case ADD:
            return 1;
            break;
        case SUB:
            return 1;
            break;
        case MUL:
            return 2;
            break;
        case DIV:
            return 2;
            break;
        case MOD:
            return 2;
            break;
        
        default:
            return -1;
    }
    
}



typedef struct {
    bool parenthesis;
    int op;
    expr_t * left;
    expr_t * right;
} bin_op_t;



void parse_expr(lexer_t lex) {
    return parse_expr_recursive(next_token(lex), 0, lex);
}

//TODO: 
//create objects in pseudocode for binops 
//test for simple arithmetic
//add right-associative operations
//add parentheses
//add unary ops

void parse_expr_recursive(token_t lhs, int precedence, lexer_t lex) {
    //from wikipedia pseudocode
    int lookahead = peak_token(1, lex);
    while (is_binary_operator(lookahead)) { // Clean this up later.
        int op = take_token(lex)->token_id; 
        token_t rhs = take_token(lex);
        lookahead = peak_token();
        int recurse = is_binary_operator(lookahead) && precedence_of(lookahead) > precedence;
        // "or a right-associative operator whose precedence is equal to op's."
        // in this case we also need to increment the precedence of op passed into the recusrion 

        while (recurse) {
            rhs = parse_expr_recursive(rhs, precedence_of(op));
        }
        // lhs = result of applying op with operands lhs and rhs
    }
    // return lhs
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
