#include "../scanner.h" // error is due to regex.h wsl thing
#include "../parser.h" 
#include <string.h>


// todo
// 

void print_level(int level) {
    for (int i = 0; i < level; i++) 
        printf("  ");
}

void print_expr(expr_t * expr, int level) ;

void print_expr_binop(binop_t * binop, int level) {
    print_level(level); printf("op: %s\n", token_to_str(binop->op));
    
    print_level(level); printf("left:\n");
    print_expr(binop->left, level+1);

    print_level(level); printf("right:\n");
    print_expr(binop->right, level+1);
}

void print_token_str(token_t * tok, int level) {
    print_level(level);
    printf("str: %s\n", tok->lexeme); 
}

void print_token_value(token_t * tok, int level) {
    print_level(level);
    printf("value: %d\n", tok->value); 
}

void print_expr_unary(unary_t * unary, int level) {
    print_level(level);
    printf("op: \n");
    print_level(level);
    printf("inner:\n");
    print_expr(unary->inner, level+1);
}

void print_expr_func_call(expr_func_call_t * func_call, int level) {
    print_level(level); printf("func_id: \n");
    print_token_str(func_call->func_id, level+1);

    print_level(level); printf("args_len: %d\n", func_call->arg_len);

    for (int i = 0; i < func_call->arg_len; i++) {
        print_level(level); printf("args[%d]: \n", i);
        print_expr(func_call->args[i], level+1);
    }
}


void print_expr(expr_t * expr, int level) {
    print_level(level);
    printf("EXPR");
    switch (expr->tag) {
        case EXPR_BINOP:
            printf("_BINOP\n");
            print_expr_binop(&expr->binop, level+1);
            break;
        case EXPR_ID:
            printf("_ID\n");
            print_token_str(&expr->id, level+1);
            break;
        case EXPR_UNARY:
            printf("_UNARY\n");
            print_expr_unary(&expr->unary, level+1);
            break;
        case EXPR_NUMERAL:
            printf("_NUMERAL\n");
            print_token_value(&expr->numeral, level+1);
            break;
        case EXPR_FUNC_CALL:
            printf("_FUNC_CALL\n");
            print_expr_func_call(&expr->func_call, level+1);
            break;

        default:
            break;
    }
    return;
}



void print_stmt(stmt_t * stmt, int level) ;

void print_stmt_block_inner(stmt_block_t * block, int level) {
    print_level(level); printf("len: %d\n", block->len);
    for (int i = 0; i < block->len; i++) {
        print_stmt(block->stmts[i], level); 
    }
}


void print_stmt_if(stmt_if_t * if_stmt, int level) {
    print_level(level); printf("cond: \n");
    print_expr(if_stmt->cond, level+1);

    print_level(level); printf("then: \n");
    print_stmt_block_inner(if_stmt->then, level+1);

    if (if_stmt->or_else) {
        print_level(level); printf("then: \n");
        print_stmt_block_inner(if_stmt->or_else, level+1);
    }
}

void print_stmt_id_decl(stmt_id_decl_t * id_decl, int level) {
    print_level(level); printf("type:\n");
    print_token_str(id_decl->type, level+1);
    
    print_level(level); printf("variable:\n");
    print_token_str(id_decl->variable, level+1);

    if (id_decl->value){
        print_level(level); printf("value:\n");
        print_expr(id_decl->value, level+1);
    }
}

void print_stmt_assign(stmt_assign_t * assign, int level) {
    print_level(level); printf("variable:\n");
    print_token_str(assign->variable, level+1);

    print_level(level); printf("value:\n");
    print_expr(assign->value, level+1);
}

void print_stmt_while(stmt_while_t * while_stmt, int level) {
    print_level(level); printf("cond: \n");
    print_expr(while_stmt->cond, level+1);

    print_level(level); printf("block: \n");
    print_stmt_block_inner(while_stmt->block, level+1);
}


void print_stmt_func_decl(stmt_func_decl_t * func_decl, int level) {
    print_level(level); printf("func_id: \n");
    print_token_str(func_decl->func_id, level+1);

    print_level(level); printf("args_len: %d\n", func_decl->param_len);
    
    for (int i = 0; i < func_decl->param_len; i++) {
        print_level(level); printf("params[%d]: \n", i);
        print_stmt(func_decl->params[i], level+1);
    }

    print_stmt_block_inner(func_decl->block, level+1);
}



void print_stmt(stmt_t * stmt, int level) {
    print_level(level); printf("STMT");
    switch (stmt->tag) {
        case STMT_IF:
            printf("_IF\n");
            print_stmt_if(stmt->stmt_if, level+1);
            break;
        case STMT_ID_DECL:
            printf("_ID_DECL\n");
            print_stmt_id_decl(stmt->stmt_id_decl, level+1);
            break;
        case STMT_ASSIGN:
            printf("_ASSIGN\n");
            print_stmt_assign(stmt->stmt_assign, level+1);
            break;
        case STMT_FUNC_CALL:
            printf("_FUNC_CALL\n");
            print_expr_func_call(&stmt->func_call->func_call, level+1);
            break;
        case STMT_WHILE:
            printf("_WHILE\n");
            print_stmt_while(stmt->stmt_while, level+1);
            break;
        case STMT_RETURN:
            printf("_RETURN\n");
            print_expr(stmt->stmt_return, level+1);
            break;
        case STMT_FUNC_DECL:
            printf("_FUNC_DECL\n");
            print_stmt_func_decl(stmt->stmt_func_decl, level+1);
            break;
        case STMT_BLOCK:
            printf("_BLOCK\n");
            print_stmt_block_inner(stmt->stmt_block, level+1);
            break;

        default:
            break;
    }
}


int main(int argc, char *argv[]) {

    int name_buf_size = 1<<7;
    int file_buf_size = 1<<11;
    // char file_name[name_buf_size];
    // char file_text[file_buf_size];
    
    // while (1) {
    //     line = fgets(file_name, name_buf_size, stdin)
    //     line[strcspn(line, "\n")] = '\0';
    //     line[strcspn(line, "\n")] = '\0';
    //     line[strcspn(line, "\n")] = '\0';



    //     size_t n = fread(file_text, 1, sizeof(file_text)-1, stdin);
    //     file_text[n] = '\0';
    // }
    
    char * source = "{ int i = 11; f(1, 2, 3); }";
    
    lexer_t * lex = create_lexer(source, strlen(source));

    
    // if (!fgets(file_text, sizeof(file_text), stdin))  
    //     return 1;
        
    printf("%s\n", lex->source);
    
    
    stmt_t * stmt = parse_stmt(lex);
    print_stmt(stmt, 0);
    return 0;
}

