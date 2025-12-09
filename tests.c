#include "scanner.h" // error is due to regex.h wsl thing
#include "parser.h" 

void print_level(int level) {
    for (int i = 0; i < count; i++) 
        print("  ");
}

void print_expr_binop(expr_binop_t * binop, int level) {
    print_level(level); printf("op: %s\n", token_to_str(binop->op));
    
    print_level(level); printf("left:\n");
    print_expr(binop->left, level+1);

    print_level(level); printf("right:\n");
    print_expr(binop->right, level+1);
}

void print_expr_id(token_t * tok, int level) {
    print_level(level);
    printf("str: %s\n", tok->str); 
}

void print_expr_numeral(token_t * tok, int level) {
    print_level(level);
    printf("value: %s\n", tok->value); 
}

void print_expr_unary(unary_t * unary, int level) {
    print_level(level);
    printf("op: %s\n", token_to_str(unary->op));
    print_level(level);
    printf("inner:\n", token_to_str(unary->op));
    print_expr(unary->inner, level+1);
}

void print_expr_func_call(expr_func_call_t * func_call, int level) {
    print_level(level); printf("func_id: %s\n", func_call->func_id);

    print_level(level); printf("args_len: %d\n", func_call->arg_len);

    for (int i = 0; i < args_len; i++) {
        print_level(level); printf("args[%d]: \n", i);
        print_expr(args[i], level+1);
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
            print("_ID\n");
            print_expr_id(expr->id, level+1);
            break;
        case EXPR_UNARY:
            print("_UNARY\n");
            print_expr_unary(expr->unary, level+1)
            break;
        case EXPR_NUMERAL:
            print("_NUMERAL\n");
            print_expr_numeral(expr->numeral, level+1);
            break;
        case EXPR_FUNC_CALL:
            print("_FUNC_CALL");
            print_expr_numeral(expr->func_call, level+1);
            break;

        default:
            break;
    }
    

}
























