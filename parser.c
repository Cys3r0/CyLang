#include "scanner.h" // error is due to regex.h wsl thing
#include "parser.h" 
#include <stdlib.h>
#include <stdio.h>

#define MAX_ARGS 64
#define MAX_STMTS_IN_BLOCK 1024
#define BLOCK_INITIAL_SIZE 64
#define MAX_STRUCT_MEMBER_COUNT 256



// add boolean expressions and the limitations of that.
// create unit tests.
// create program super struct or something with list of func_decls

// LATER:
//test parser for the input ()
//create REPL 
//Figure out how to propagate error messages
//create unit tests for scanner/parser
//implement pretty print for exprs
//test calling functions within functions.


int is_binop(enum TokenType token_type) {
    return     token_type == TOKEN_ADD
            || token_type == TOKEN_MUL
            || token_type == TOKEN_SUB
            || token_type == TOKEN_DIV
            || token_type == TOKEN_MOD
            || token_type == TOKEN_EXPONENT
            || token_type == TOKEN_EQ
            || token_type == TOKEN_NEQ
            || token_type == TOKEN_GEQ
            || token_type == TOKEN_GT
            || token_type == TOKEN_LEQ
            || token_type == TOKEN_LT
            || token_type == TOKEN_BIT_AND
            || token_type == TOKEN_BIT_OR
            || token_type == TOKEN_BIT_XOR
            || token_type == TOKEN_LOG_AND
            || token_type == TOKEN_LOG_OR;
}

int is_right_associative(enum TokenType token_type) {
    int ret = token_type == TOKEN_EXPONENT;
    return ret;
}

int is_unary(enum TokenType token_type) {
    return     token_type == TOKEN_SUB
            || token_type == TOKEN_ADD
            || token_type == TOKEN_ADDRESSOF
            || token_type == TOKEN_DEREF
            || token_type == TOKEN_LOG_NOT
            || token_type == TOKEN_BIT_NOT; 
}

int is_atom(enum ExprType type) {
    return     type == EXPR_NUMERAL
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
    skip_token(lex, TOKEN_LPAR);
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
    skip_token(lex, TOKEN_RPAR);
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


stmt_t * parse_stmt(lexer_t * lex);

type_id_t * create_type(int ptr, token_t * type) {
    type_id_t * new_type = calloc(1, sizeof(type_id_t));
    new_type->ptr  = ptr;
    new_type->type = type;
    return new_type;
}

int is_primitive(enum TokenType type) {
    return type == TOKEN_BOOL || type == TOKEN_I32;
}

type_id_t * parse_type(lexer_t * lex) {
    token_t * tok = take_token(lex); // can be either pointer or type
    int ptr = 0;

    if (tok->token_type == TOKEN_ADDRESSOF) {
        tok = take_token(lex);
        ptr = 1;
    }
    
    if (!is_primitive(tok->token_type) &&  tok->token_type != TOKEN_ID) {
        printf("ERROR: incorrect type. \n");
        exit(EXIT_FAILURE);
    }

    return create_type(ptr, tok);    
}


void block_resize(stmt_block_t * vec) {
    int new_cap = vec->cap << 1;
    stmt_t ** tmp = realloc(vec->stmts, new_cap * sizeof(*vec->stmts));
    if (!tmp) {
        printf("ERROR: not enough memory left, buy more RAM please.\n");
        exit(EXIT_FAILURE);
    }
    vec->stmts = tmp;
    vec->cap = vec->cap << 1;
}

void block_add(stmt_t * item, stmt_block_t * block) {
    block->stmts[block->len] = item;
    block->len++;

    if(block->len == block->cap) {
        block_resize(block); }
}

stmt_block_t * create_block() {
    void * stmts = calloc(BLOCK_INITIAL_SIZE, sizeof(stmt_t*));
    stmt_block_t * block = calloc(1, sizeof(stmt_block_t));

    block->stmts = stmts;
    block->cap = BLOCK_INITIAL_SIZE;
    return block;
}


stmt_t * parse_stmt_func_call(lexer_t * lex) {
    token_t * next = take_token(lex);
    expr_t * func_call = parse_expr_func_call(next, lex);
    skip_token(lex, TOKEN_SEMI);
    
    stmt_t * stmt = malloc(sizeof(stmt_t));
    stmt->tag = STMT_FUNC_CALL;
    stmt->func_call = func_call;
    return stmt;
}

stmt_t * parse_stmt_id_decl(lexer_t * lex) {
    type_id_t * type = parse_type(lex);
    token_t * variable = take_token(lex);
    enum TokenType next = take_token(lex)->token_type;
    expr_t * value = NULL;
    
    if (next == TOKEN_ASSIGN) {
        value = parse_expr(lex);
        skip_token(lex, TOKEN_SEMI);
    } else if (next == TOKEN_SEMI) {
        value = NULL;
    }
    
    stmt_id_decl_t * id_decl = malloc(sizeof(stmt_id_decl_t));
    id_decl->type = type;
    id_decl->variable = variable;
    id_decl->value = value;
    
    stmt_t * stmt = malloc(sizeof(stmt_t));
    stmt->tag = STMT_ID_DECL;
    stmt->stmt_id_decl = id_decl;
    return stmt;    
}

stmt_t * parse_stmt_assign(lexer_t * lex) {
    token_t * variable = take_token(lex);
    skip_token(lex, TOKEN_ASSIGN);
    expr_t * value = parse_expr(lex);
    skip_token(lex, TOKEN_SEMI);
    
    stmt_assign_t * assign = malloc(sizeof(stmt_assign_t));
    assign->variable = variable;
    assign->value = value;
    
    stmt_t * stmt = malloc(sizeof(stmt_t));
    stmt->tag = STMT_ASSIGN;
    stmt->stmt_assign = assign;
    return stmt;
}

stmt_block_t * parse_stmt_block_inner(lexer_t * lex) {    
    stmt_t ** stmts = malloc(MAX_STMTS_IN_BLOCK * sizeof(stmt_t*));
    stmt_block_t * block = create_block();
    
    skip_token(lex, TOKEN_LWING);
    int i = 0;
    while(peak_token(lex) != TOKEN_RWING) {
        block_add(parse_stmt(lex), block);
    }
    skip_token(lex, TOKEN_RWING);
    
    return block;
}

stmt_t * parse_stmt_block(lexer_t * lex) {
    stmt_t * stmt =  malloc(sizeof(stmt_t));
    stmt->tag = STMT_BLOCK;
    stmt->stmt_block = parse_stmt_block_inner(lex);
    return stmt;
}


stmt_t * parse_stmt_while(lexer_t * lex) {
    skip_token(lex, TOKEN_WHILE);
    skip_token(lex, TOKEN_LPAR);
    expr_t * cond = parse_expr(lex);
    skip_token(lex, TOKEN_RPAR);
    
    stmt_block_t * block = parse_stmt_block_inner(lex);
    
    stmt_while_t * while_stmt = malloc(sizeof(stmt_while_t));
    while_stmt->cond = cond;
    while_stmt->block = block;
    
    stmt_t * stmt = malloc(sizeof(stmt_t));
    stmt->tag = STMT_WHILE;
    stmt->stmt_while = while_stmt;
    return stmt;
}

stmt_t * parse_stmt_if(lexer_t * lex) {
    skip_token(lex, TOKEN_IF );
    skip_token(lex, TOKEN_LPAR);
    
    expr_t * cond = parse_expr(lex);
    
    skip_token(lex, TOKEN_RPAR);
    
    stmt_block_t * then = parse_stmt_block_inner(lex);
    stmt_block_t * or_else = NULL;
    
    if (peak_token(lex) == TOKEN_ELSE) {
        skip_token(lex, TOKEN_ELSE);
        
        or_else = parse_stmt_block_inner(lex);
        
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
    skip_token(lex, TOKEN_RETURN);
    expr_t * ret_expr = parse_expr(lex);
    skip_token(lex, TOKEN_SEMI);
    
    stmt_t * stmt = malloc(sizeof(stmt_t));
    stmt->tag = STMT_RETURN;
    stmt->stmt_return = ret_expr;
    return stmt;
}

stmt_struct_decl_t * parse_struct_decl(lexer_t * lex) {
    skip_token(lex, TOKEN_STRUCT);
    token_t * name = take_token(lex);
    skip_token(lex, TOKEN_LWING);
    stmt_id_decl_t ** members = calloc(MAX_STRUCT_MEMBER_COUNT, sizeof(stmt_id_decl_t *));
    int i = 0;

    while (peak_token(lex) != TOKEN_RWING ) {
        if (i > MAX_STRUCT_MEMBER_COUNT) 
            { printf("Too many members."); exit(EXIT_FAILURE); }
        members[i++] = parse_stmt_id_decl(lex)->stmt_id_decl; // holy ugly
    }

    skip_token(lex, TOKEN_RWING);
    stmt_struct_decl_t * struct_decl = calloc(1, sizeof(stmt_struct_decl_t));
    struct_decl->name = name;
    struct_decl->members = members;
    struct_decl->member_len = i;

    return struct_decl;
}

stmt_t * parse_stmt_func_decl(lexer_t * lex) {
    // add check for TOKEN_EOF
    type_id_t * type = parse_type(lex);
    token_t * name = take_token(lex);
    skip_token(lex, TOKEN_LPAR);
    enum TokenType peak = peak_token(lex);
    stmt_t ** params = NULL;
    stmt_block_t * block;
    int i = 0;
    token_t * next;
    
    if (peak != TOKEN_RPAR) { 
        params = malloc(MAX_ARGS * sizeof(stmt_t));
        params[i] = parse_stmt_id_decl(lex);
        i++;
        peak = peak_token(lex);
        
        while (peak_token(lex) == TOKEN_COMMA) {
            if (i >= MAX_ARGS) {
                printf("Function call may not exceed MAX_ARGS arguments.");
                exit(EXIT_FAILURE);
            }
            skip_token(lex, TOKEN_COMMA);
            params[i] = parse_stmt_id_decl(lex);
            i++;
        }
        skip_token(lex, TOKEN_RPAR);
    } else {
        skip_token(lex, TOKEN_RPAR);
    }
    block = parse_stmt_block_inner(lex);
    
    stmt_func_decl_t * func_decl = malloc(sizeof(stmt_func_decl_t));
    func_decl->type = type;
    func_decl->func_id = name;
    func_decl->params = params;
    func_decl->param_len = i;
    func_decl->block = block;
    
    stmt_t * stmt = malloc(sizeof(stmt_t));
    stmt->tag = STMT_FUNC_DECL;
    stmt->stmt_func_decl = func_decl;
    
    return stmt;
}

stmt_t * parse_stmt(lexer_t * lex) {
    stmt_t * stmt; 
    switch (peak_token(lex)) {
        case TOKEN_ADDRESSOF:
            stmt = parse_stmt_id_decl(lex);
            break;
        case TOKEN_IF:
            stmt = parse_stmt_if(lex);
            break;
        case TOKEN_LWING:
            stmt = parse_stmt_block(lex);
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
        default: break; // some kind of goto error or something
    }
    
    return stmt;
}

stmt_t * parse_outer(lexer_t * lex) {
    if (peak_token(lex) == TOKEN_STRUCT) 
        { return parse_struct_decl(lex); } 
    return parse_stmt_func_decl(lex);
}

stmt_block_t * parse_program(lexer_t * lex) {
    stmt_block_t * program = create_block();
    stmt_t * stmt;
    while ((stmt = parse_outer(lex))) 
        block_add(stmt, program);
    
    return program;
}

// int main(int argc, char const *argv[]) {
//     char * file_text = "int a = 10;";

//     regex_t regex;
//     regmatch_t m[NUMBER_OF_TOKENS + 1];
//     regcomp(&regex, REGEX_RULES, REG_EXTENDED);
//     lexer_t * lex = init_lexer(file_text, regex, m);
//     printf("%s\n", lex->file_text);
    
//     for (int i = 0; i < 9; i++) {
//         printf("%s ", token_to_str(peak_n_tokens(i + 1, lex)));
//     }
//     printf("\n");

//     stmt_t * stmt = parse_stmt(lex);
//     if (stmt->tag == STMT_ID_DECL) {
//         printf("STMT_ID_DECL\n");
//         printf("type: %s, name: %s, value: %d \n", 
//             stmt->stmt_id_decl->type->str, stmt->stmt_id_decl->variable->str, stmt->stmt_id_decl->value->numeral.value);
//     }
//     // token_t * next = take_token(lex);
//     // expr_t * e = parse_expr(lex);

//     // print_expr(e);
//     // printf("arg_len := %d \n", e->func_call.arg_len);


//     // expr_t * e = parse_expr(lex);
//     // print_expr(e);
//     return 0;
// }



