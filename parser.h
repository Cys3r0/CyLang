#ifndef PARSER_H
#define PARSER_H

#include "scanner.h" // error is due to regex.h wsl thing
#include <stdlib.h>
#include <stdio.h>

#define MAX_ARGS 64

enum ExprType { 
    EXPR_BINOP, 
    EXPR_UNARY, 
    EXPR_FUNC_CALL, 
    EXPR_NUMERAL, 
    EXPR_ID 
};

int is_binop(enum TokenType token_type) ;

int is_right_associative(enum TokenType token_type) ;

int is_unary(enum TokenType token_type) ;

int is_atom(enum ExprType type) ;

int precedence_of(enum TokenType token_type) ;

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

char * expr_tag_to_str(enum ExprType tag) ;

expr_t * create_binop_expr(enum TokenType op, expr_t * left, expr_t * right) ;

expr_t * create_atom_expr(token_t * tok) ;

expr_t * parse_expr_func_call(token_t * func_id, lexer_t * lex) ;

expr_t * parse_expr_atom(token_t * next, lexer_t * lex) ;

expr_t * parse_expr_paran(lexer_t * lex) ;

expr_t * parse_expr_unary(enum TokenType tok_type, lexer_t * lex) ;

expr_t * parse_expr_operand(token_t * next, lexer_t * lex) ;

expr_t * parse_expr_recursive(expr_t * lhs, int min_precedence, lexer_t * lex, enum TokenType * lookahead) ;

expr_t * parse_expr(lexer_t * lex) ;




//stmts

enum StmtType { 
    STMT_IF, 
    STMT_ID_DECL, 
    STMT_ASSIGN, 
    STMT_FUNC_CALL, 
    STMT_WHILE, 
    STMT_RETURN, 
    STMT_FUNC_DECL,
    STMT_BLOCK
};

typedef struct stmt stmt_t;

typedef struct {
    stmt_t ** stmts;
    int stmt_count;
} stmt_block_t;

typedef struct {
    expr_t * cond;
    stmt_block_t * then;
    stmt_block_t * or_else;
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
    token_t * type;
    token_t * func_id;
    stmt_t ** params; // id_decls
    int param_len;
    stmt_block_t * block;
} stmt_func_decl_t;

typedef struct {
    expr_t * cond;
    stmt_block_t * block;
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
        stmt_func_decl_t * stmt_func_decl;
        stmt_block_t * stmt_block;
    };    
};    


stmt_t * parse_stmt_func_call(lexer_t * lex) ;

stmt_t * parse_stmt_assign(lexer_t * lex) ;

stmt_t * parse_stmt_block(lexer_t * lex) ;

stmt_t * parse_stmt_while(lexer_t * lex) ;

stmt_t * parse_stmt_if(lexer_t * lex) ;

stmt_t * parse_stmt_return(lexer_t * lex) ;

stmt_t * parse_stmt_func_decl(lexer_t * lex) ;

stmt_t * parse_stmt(lexer_t * lex) ;


#endif 
