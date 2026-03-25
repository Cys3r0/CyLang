#include "parser.h"
#include "scanner.h"
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define HASH_TABLE_INITIAL_CAPACITY 64
#define TEMP_MAX_SYMBOL_TABLE_SIZE 64
#define W 64

//TODO
//test hash_table 
//name checking, type checking, param - arguement len checking, return paths
//AST traversal, maybe with _Generic (?) 
//Test list traversal


//Better way to do this? Generate at compile time?
static const uint64_t a[65] = {
    0x9e3779b97f4a7c15ULL, 0xbf58476d1ce4e5b9ULL,
    0x94d049bb133111ebULL, 0xd6e8feb86659fd93ULL,
    0xa5a3564e27f5a1c3ULL, 0x8d58ac26afe12e47ULL,
    0xc3a5c85c97cb3127ULL, 0xb492b66fbe98f273ULL,
    0x9ae16a3b2f90404fULL, 0xc949d7c7509e6557ULL,
    0x86f1f6c8e2bde8d1ULL, 0xd8a8f03e6c8c9b3bULL,
    0xa24baed4963ee407ULL, 0x8f5ad8a2e7c1d9abULL,
    0xc0a3b1f3d2e4f567ULL, 0xb7e151628aed2a6bULL,
    0x9ddfea08eb382d69ULL, 0xc6bc279692b5c323ULL,
    0x8538ec9b8f6cfa35ULL, 0xda942042e4dd58b5ULL,
    0xa7c5ac471b478423ULL, 0x8b8b5d6c2f7c1aefULL,
    0xc2b2ae3d27d4eb4fULL, 0xb4e4d5c1a3f6e2ddULL,
    0x9f6a2c8e5d3b1a07ULL, 0xc8e35a7f4d9b2c11ULL,
    0x8703f2a1c6e9b8dfULL, 0xd9b54a2f8c3e7d6bULL,
    0xa3c59ac92f7d4e31ULL, 0x8e8e8e8e8e8e8e8dULL,
    0xc1f651c67c62c6e1ULL, 0xb6d1f5a7a9e3b12fULL,
    0x9c2f715f1bdbadf9ULL, 0xc5bf891b4ef6aa79ULL,
    0x84a1f3c5d7e9b12dULL, 0xdb7f5e3c1a9d6b47ULL,
    0xa8b8c8d8e8f8a8b9ULL, 0x8a7c6d5e4f3a291dULL,
    0xc4d3b2a1908f7e6dULL, 0xb3f1d2c4a5968779ULL,
    0x9e1f3d5b7a9cbedfULL, 0xc7a9b3d5f7091b2dULL,
    0x862a4c6e8fa1b3c5ULL, 0xdaf123456789abcdULL,
    0xa1b2c3d4e5f60719ULL, 0x8c9daebfcedfa1b3ULL,
    0xc0ffee123456789bULL, 0xbaddcafedeadbeefULL,
    0x9abcdef01234567bULL, 0xcafebabedeadfaceULL,
    0x876543210fedcba9ULL, 0xd15ea5e5ba11faceULL,
    0xa55aa55aa55aa55bULL, 0x8ffffffffffffffdULL,
    0xe7037ed1a0b428dbULL
};

enum EntryState { EMPTY, OCCUPIED, DELETED };

// Fix actually test this hash_table
typedef struct {
    enum EntryState state;
    char * key;
    void * value;
} entry_t;

typedef struct {
    entry_t * entries;
    int len;
    int cap;
} htable_t;

typedef struct {
    htable_t * func_id_to_func;
    htable_t * func_id_to_type;
    htable_t ** tables;
    stmt_func_decl_t * curr_func;
    int len;
    int cap;
} sym_stack_t;


uint64_t hash_str(const unsigned char *s) {
    // hashing function stolen from wikipedia
    __uint128_t h = (__uint128_t)(a[0]);
    
    for (int i = 0; i < MAX_LEXEME_LENGTH; i++) {
        h += (__uint128_t)(a[i + 1]) * (__uint128_t)(s[i]);
    }
    
    return (uint64_t)(h >> W);
}


htable_t * create_hash_table() {
    entry_t * entries = calloc(HASH_TABLE_INITIAL_CAPACITY, sizeof(entry_t)); // this should initialize to EMPTY
    htable_t * table = calloc(1, sizeof(htable_t));
    table->entries = entries;
    table->cap = HASH_TABLE_INITIAL_CAPACITY;

    return table;
}

void ht_resize(htable_t * table, int make_bigger) {
    int new_cap = (make_bigger) ? table->cap << 1 : table->cap >> 1;
    entry_t * new_entries = calloc(new_cap, sizeof(entry_t));
    int new_idx = 0;
    entry_t curr_entry;
    int j;

    for (size_t i = 0; i < table->cap; i++) {
        if ((curr_entry = table->entries[i]).state == OCCUPIED) {
            j = hash_str((const unsigned char *) curr_entry.key) % new_cap;
            new_entries[j] = curr_entry; // expensive copy, use pointers instead?
        }
    }

    free(table->entries);
    table->entries = new_entries;
    table->cap = new_cap;
}


int put(htable_t * table, char * key, void * value) {
    // returns 0 if key was already in table
    entry_t curr_entry;
    int j;
    int dead_idx = -1;
    int hash_idx;
    
    if (table->len / table->cap > 0.7)
        { ht_resize(table, 1); }

    for (size_t i = 0; i < table->cap; i++) {
        hash_idx = (hash_str((const unsigned char *) key) + i) % table->cap;
        curr_entry = table->entries[hash_idx];

        if (curr_entry.state == OCCUPIED) {
            if (strcmp(curr_entry.key, key) == 0) {
                curr_entry.value = value;
                return 0;
            }
            continue;
        }

        if (curr_entry.state == DELETED && dead_idx == -1) {
            dead_idx = hash_idx; 
            continue;
        }

        if (curr_entry.state == EMPTY) {
            if (dead_idx != -1) 
                { hash_idx = dead_idx; } 
            else 
                { table->len++; }
            
            table->entries[hash_idx].state = OCCUPIED;
            table->entries[hash_idx].value = value;
            return 1;
        }
    }

    printf("PUT: HASH TABLE FULL");
    exit(EXIT_FAILURE);
}

void* get(htable_t * table, char * key) {
    entry_t curr_entry;
    int hash_idx;
    
    for (size_t i = 0; i < table->cap; i++) {
        hash_idx = (hash_str((const unsigned char *) key) + i) % table->cap;
        curr_entry = table->entries[hash_idx];

        if (curr_entry.state == EMPTY) 
            { return NULL; }
        if (curr_entry.state == DELETED) 
            { continue; }
        if (strcmp(key, curr_entry.key) != 0) 
            { continue; } 
        
        return curr_entry.value;
    }
    
    printf("GET: HASH TABLE FULL");
    exit(EXIT_FAILURE);   
}

int contains(htable_t * table, char * key) {
    entry_t curr_entry;
    int hash_idx;
    
    for (size_t i = 0; i < table->cap; i++) {
        hash_idx = (hash_str((const unsigned char *) key) + i) % table->cap;
        curr_entry = table->entries[hash_idx];

        if (curr_entry.state == EMPTY) 
            { return 0; }
        if (curr_entry.state == DELETED) 
            { continue; }
        if (strcmp(key, curr_entry.key) != 0) 
            { continue; } 
        
        return 1;
    }
    
    printf("CONTAINS: HASH TABLE FULL");
    exit(EXIT_FAILURE);  
} 

void * del(htable_t * table, char * key) {
    entry_t curr_entry;
    int hash_idx;
    
    if (table->len / table->cap < 0.3 && table->len > 64)
        { ht_resize(table, 0); }

    for (size_t i = 0; i < table->cap; i++) {
        hash_idx = (hash_str((const unsigned char *) key) + i) % table->cap;
        curr_entry = table->entries[hash_idx];

        if (curr_entry.state == EMPTY) 
            { return NULL; }
        if (curr_entry.state == DELETED) 
            { continue; }
        if (strcmp(key, curr_entry.key) != 0) 
            { continue; } 
        
        table->entries[hash_idx].state = DELETED;
        return table->entries[hash_idx].value;
    }
    
    printf("DEL: HASH TABLE FULL");
    exit(EXIT_FAILURE);   
}

sym_stack_t * create_symbol_stack() {
    sym_stack_t * syms = calloc(1, sizeof(sym_stack_t));
    syms->func_id_to_func = create_hash_table();
    syms->tables = calloc(TEMP_MAX_SYMBOL_TABLE_SIZE, sizeof(htable_t *));
    syms->tables[0] = create_hash_table();
    syms->len = 1;
    syms->cap = TEMP_MAX_SYMBOL_TABLE_SIZE;
    return syms;
}

void push(sym_stack_t * syms) {
    if (syms->len+1 == syms->cap) {
        printf("ERROR: Symbol stack overfull.");
        exit(EXIT_FAILURE);
    }

    syms->tables[ syms->len++ ] = create_hash_table();
}

void pop(sym_stack_t * syms) {
    if (syms->len-1 < 0) {
        printf("ERROR: Symbol stack popped while empty.");
        exit(EXIT_FAILURE);
    }
    syms->len--;
    syms->tables[ syms->len ] = NULL;
}

// TODO:
// impl type_nodes here. 
// impl type_nodes comparison.

// solved a program vector that can be iterate through.
// add checks for visit expr

// NOTE:
// "In a standard C compiler, the lexer performs a lookup for every identifier it encounters
// to determine its token type. If the identifier is found in the Ordinary Namespace as a 
// typedef, the lexer returns a TYPE_NAME token; otherwise, it returns an IDENTIFIER token."

// for this to error, maybe pass a token to error at the note.

int stack_contains_name(sym_stack_t * syms, char * name) {
    for (size_t i = syms->len-1; i >= 0; i--) {
        if (contains(syms->tables[i], name)) return 1; 
    }
    return 0;
}

type_node_t* stack_get_type(sym_stack_t* syms, char* name) {
    for (size_t i = syms->len-1; i >= 0; i--) {
        if (contains(syms->tables[i], name)) return (type_node_t*) get(syms->tables[i], name); 
    }
    return NULL;   
}

int same_type(type_node_t* left, type_node_t* right) {
    if (left->info != right->info) return 0;
    if (left->next == NULL && right->next == NULL) return 1;
    if (left->next == NULL || right->next == NULL) return 0;

    return same_type(left->next, right->next);
}

void stack_contains_type(sym_stack_t * syms, type_node_t * type) { 
    // IMPL CORRECTLY WHEN IMPLING STRUCTS 
}

void deref(type_node_t** type) {
    if ((*type)->tag != POINTER) {
        // add error tracking here.
        printf("Can not deref non-pointer.\n");
        exit(EXIT_FAILURE);
    }
    *type = (*type)->next;
}

void addressof(type_node_t* type) {
    type_node_t* outer_ptr = create_type_node(POINTER, &ptr_info);
    outer_ptr->next = type;
}

void typecheck_expr(sym_stack_t * syms, expr_t * e, type_node_t * expected);

void visit_func_call(sym_stack_t* syms, expr_func_call_t* call) {    
    stmt_func_decl_t * func_decl = get(syms->func_id_to_func, call->func_id->lexeme);
    if (!func_decl)  { 
        printf("ERROR: function not defined.\n"); 
        exit(EXIT_FAILURE); 
    }
    assert(func_decl->param_len == call->arg_len);

    for (size_t i = 0; i < func_decl->param_len; i++) {
        typecheck_expr(syms, call->args[i], func_decl->params[i]->stmt_id_decl->type);
    }
}

type_node_t * visit_expr(sym_stack_t * syms, expr_t * e) {
    // memory leaky with all the callocs for each type.
    // this needs a free_expr_chain. How would that be implemented
    type_node_t* expected = NULL;
    type_node_t* ret      = NULL; 
    switch (e->tag) {
        case EXPR_BINOP: {
            enum TokenType op_type = e->binop.op;

            type_node_t* left = visit_expr(syms, e->binop.left);
            type_node_t* right = visit_expr(syms, e->binop.right);

            if (is_binop(op_type) ) {
                expected = ret = &(type_node_t){PRIMITIVE, NULL, &i32_info}; 
            }
            else if (op_type == TOKEN_EQ  || op_type == TOKEN_NEQ
                  || op_type == TOKEN_GEQ || op_type == TOKEN_GT
                  || op_type == TOKEN_LEQ || op_type == TOKEN_LT) {
                expected = &(type_node_t){PRIMITIVE, NULL, &i32_info};
                ret = &(type_node_t){PRIMITIVE, NULL, &bool_info};
            } 
            else if (op_type == TOKEN_LOG_AND || op_type == TOKEN_LOG_OR) { 
                expected = ret = &(type_node_t){PRIMITIVE, NULL, &bool_info}; 
            }
            
            if (!same_type(left, expected) || !same_type(right, expected)) { 
                printf("ERROR: type doesn't match expected. \n"); 
                exit(EXIT_FAILURE); 
            }

            return create_type_node(PRIMITIVE, &bool_info);
        }
        case EXPR_FUNC_CALL:{
            stmt_func_decl_t * func_decl = get(syms->func_id_to_func, e->func_call.func_id->lexeme);
            if (!func_decl) 
                { printf("ERROR: function not defined.\n"); exit(EXIT_FAILURE); }

            visit_func_call(syms, &e->func_call);
            return func_decl->type;
        }
        case EXPR_NUMERAL:{
            return create_type_node(PRIMITIVE, &i32_info);
        }
        case EXPR_UNARY: {
            type_node_t * actual = visit_expr(syms, e->unary.inner);
            
            if (e->unary.op == TOKEN_ADD || e->unary.op == TOKEN_SUB || e->unary.op == TOKEN_BIT_NOT) { 
                expected = create_type_node(PRIMITIVE, &i32_info); 
            } else if (e->unary.op == TOKEN_LOG_NOT) { 
                expected = create_type_node(PRIMITIVE, &bool_info); 
            } else if (e->unary.op == TOKEN_DEREF) { 
                deref(&actual);
            } else if (e->unary.op == TOKEN_ADDRESSOF) { 
                addressof(actual);
            } else { 
                assert(0); 
            } 
            
            if (expected == NULL) { 
                return actual; 
            }
            
            if (!same_type(expected, actual)) { 
                printf("ERROR: inner type not expected"); 
                exit(EXIT_FAILURE); 
            }
            
            return actual;   
        }
        case EXPR_ID: {
            type_node_t* type = stack_get_type(syms, e->id.lexeme);
            if (type == NULL) {
                printf("ERROR: ID expr not in symstack."); 
                exit(EXIT_FAILURE);                 
            }
            
            return stack_get_type(syms, e->id.lexeme);
        }
    }    

    assert(0); 
}

void typecheck_expr(sym_stack_t * syms, expr_t * e, type_node_t * expected) {
    type_node_t * actual = visit_expr(syms, e);

    if (!same_type(expected, actual)) { 
        printf("ERROR: return type doesn't match.\n"); 
        exit(EXIT_FAILURE); 
    }
}

void visit_stmt(sym_stack_t * syms, stmt_t * stmt);

void visit_stmt_block(sym_stack_t * syms, stmt_block_t * block) {
    push(syms);
    for (size_t i = 0; i < block->len; i++) {
        visit_stmt(syms, block->stmts[i]);
    }
    pop(syms);
}

void visit_func_decl(sym_stack_t * syms, stmt_func_decl_t * func_decl) {
    if (contains(syms->func_id_to_func, func_decl->func_id->lexeme)) {
        printf("Func_decl already defined. \n"); 
        exit(EXIT_FAILURE); 
    }
    put(syms->func_id_to_type, func_decl->func_id->lexeme, func_decl->type);

    push(syms);
    for (size_t i = 0; i < func_decl->param_len; i++) {
        char * name = func_decl->params[i]->stmt_id_decl->name->lexeme;
        put(syms->tables[syms->len-1], name, func_decl->type);
    }
    
    for (size_t i = 0; i < func_decl->block->len; i++) {
        visit_stmt(syms, func_decl->block->stmts[i]);
    }
    pop(syms);
}


void visit_stmt_id_decl(sym_stack_t * syms, stmt_id_decl_t * id_decl) {
    if (id_decl->value) { 
        typecheck_expr(syms , id_decl->value, id_decl->type); 
    }

    if (contains(syms->tables[syms->len-1], id_decl->name->lexeme)) {
        printf("Variable already defined in scope. \n"); 
        exit(EXIT_FAILURE); 
    }
    
    put(syms->tables[syms->len-1], id_decl->name->lexeme, id_decl->type);
}

void visit_stmt_assign(sym_stack_t * syms, stmt_assign_t * assign) {
    if (!stack_contains_name(syms, assign->name->lexeme)) {
        printf("Variable not defined. \n"); 
        exit(EXIT_FAILURE); 
    }

    typecheck_expr(syms, assign->value, stack_get_type(syms, assign->name->lexeme));
}


void visit_stmt_while(sym_stack_t* syms, stmt_while_t* while_stmt) {    
    typecheck_expr(syms, while_stmt->cond, &(type_node_t){PRIMITIVE, NULL, &bool_info});
    visit_stmt_block(syms, while_stmt->block);
}

void visit_stmt_func_call(sym_stack_t* syms, expr_t* e) { 
    visit_func_call(syms, &e->func_call);
}

void visit_stmt_if(sym_stack_t * syms, stmt_if_t * if_stmt) {
    typecheck_expr(syms, if_stmt->cond, &(type_node_t){PRIMITIVE, NULL, &bool_info});
    visit_stmt_block(syms, if_stmt->then);
    if (if_stmt->or_else) { 
        visit_stmt_block(syms, if_stmt->or_else); 
    }
}

void visit_stmt_return(sym_stack_t * syms, expr_t * ret_expr) { 
    type_node_t * expected = syms->curr_func->type;
    typecheck_expr(syms, ret_expr, expected);
} 

enum TokenType binop_expected(enum TokenType t) { 
    if (t == TOKEN_SUB || t == TOKEN_ADD) return TOKEN_NUM;
    if (t == TOKEN_DEREF) return TOKEN_NUM;
    assert(0);
}

/*
    Current with the compiler:
    1. A REPL usage would be impossible since pure expr calls are not allowed (only stmt_func_call)
    2. Type checking is too messy.
        A. The parsing of pointers is not good. Instead of a counter, use type_nodes instead. 
        This has easier bookkeeping for size and such. Recurse down both LHS and RHS when typechecking
    3. DONE Scanner doesn't even properly implement the "->>>>" syntax. 
        A. This will require a check for the number of ">" used in expressions such that "->(-> b)" or "->> b" as double addressof.
        const ->> 
    4. There should be an id_use expr, I can't remember why though but look it up.
    5. func_calls, numerals and id_uses should be treated as an atom expr. 
    6. Primitive types should carry a pointer to const global structs describing byte size, etc. (basically type_info_t).
    7. Non-primitives type_info should be calculated in visit_struct_decl.
    8. Accessing members of a struct via dot-notation not implemented, i.e human.height.
        A. also decide whether to deref automatically (->node.next) or implicitly ( (->)? Node node = node.next; ) 
    9. Error handling is non-existant right now. 

    NEXT: Fix type-matching parsing.
*/

typedef struct {
    char* name;
    type_node_t type;
    int offset;
} member_record_t;
// TODO:
// 1. Figure out alignment in structs.
void visit_struct_decl(sym_stack_t * syms, stmt_struct_decl_t * struct_decl) {
    // IMPLEMENT WHEN ACTUALLY USING STRUCTS
    // // this is where type_info should be added.
    // type_info_t** members = calloc(struct_decl->member_len, sizeof(type_info_t));
    
    // for (size_t i = 0; i < struct_decl->member_len; i++) {
    //     members[i] = struct_decl->members[i]->type;
    //     type_node_t* type = struct_decl->members[i]->type;

    //     if (contains(syms->tables[0], name)) {
    //         printf("\n");
    //     }
    // }
    // put(syms->tables[0], struct_decl->name, );
}

void visit_stmt(sym_stack_t * syms, stmt_t * stmt) {
    if (stmt->tag == STMT_IF) {
        visit_stmt_if(syms, stmt->stmt_if);
    }
    if (stmt->tag == STMT_ID_DECL) {
        visit_stmt_id_decl(syms, stmt->stmt_id_decl);
    }
    if (stmt->tag == STMT_ASSIGN) {
        visit_stmt_assign(syms, stmt->stmt_assign);
    }
    if (stmt->tag == STMT_FUNC_CALL) {
        visit_stmt_func_call(syms, stmt->func_call);
    }
    if (stmt->tag == STMT_WHILE) {
        visit_stmt_while(syms, stmt->stmt_while);
    }
    if (stmt->tag == STMT_RETURN) {
        visit_stmt_return(syms, stmt->stmt_return);
    }
    if (stmt->tag == STMT_BLOCK) {
        visit_stmt_block(syms, stmt->stmt_block);
    }
}

void visit_program(stmt_block_t * program) {
    sym_stack_t* syms = create_symbol_stack();
    for (size_t i = 0; i < program->len; i++) {
        visit_func_decl(syms, (stmt_func_decl_t*) program->stmts[i]);
    }
}

int main() {
    // char * str = calloc(64, sizeof(char));
    // for (size_t i = 0; i < 50; i++) {
    //     str[i] = 'a';
        
    // }
    // str[50] = '\0';

    // const unsigned char * ustr = (const unsigned char *) str;    
    // printf("%lu", hash_str(ustr));
    
    return 0;
}

