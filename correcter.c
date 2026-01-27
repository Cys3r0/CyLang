#include "parser.h"
#include "scanner.h"
#include <stdint.h>
#include <string.h>


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
    htable_t * type_table;
    htable_t ** tables;
    stmt_func_decl_t * curr_func;
    int len;
    int cap;
} symbol_stack_t;


uint64_t hash_str(const unsigned char *s) {
    // hashing function stolen from wikipedia
    __uint128_t h = (__uint128_t)(a[0]);
    
    for (int i = 0; i < MAX_LEXEME_LENGTH; i++)
    h += (__uint128_t)(a[i + 1]) * (__uint128_t)(s[i]);
    
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


int ht_put(htable_t * table, char * key, void * value) {
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

void * ht_get(htable_t * table, char * key) {
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

int ht_contains(htable_t * table, char * key) {
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

void * ht_del(htable_t * table, char * key) {
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

symbol_stack_t * create_symbol_stack() {
    symbol_stack_t * syms = calloc(1, sizeof(symbol_stack_t));
    syms->func_id_to_func = create_hash_table();
    syms->tables = calloc(TEMP_MAX_SYMBOL_TABLE_SIZE, sizeof(htable_t *));
    syms->tables[0] = create_hash_table();
    syms->len = 1;
    syms->cap = TEMP_MAX_SYMBOL_TABLE_SIZE;
    return syms;
}

void sym_stack_push(symbol_stack_t * syms) {
    if (syms->len+1 == syms->cap) {
        printf("ERROR: Symbol stack overfull.");
        exit(EXIT_FAILURE);
    }

    syms->tables[ syms->len++ ] = create_hash_table();
}

void sym_stack_pop(symbol_stack_t * syms) {
    if (syms->len-1 < 0) {
        printf("ERROR: Symbol stack popped while empty.");
        exit(EXIT_FAILURE);
    }

    syms->tables[ --syms->len ] = NULL;
}

typedef struct {
    type_id_t * type;
    size_t byte_size;
    int nesting; // level of nesting
    // offset func decl 
} type_info_t;

type_id_t * create_type_id(int ptr, enum TokenType type) {
    type_id_t * new_type = calloc(1, sizeof(type_id_t));
    new_type->ptr = ptr;
    new_type->type = type;
}



// TODO:
// fix all the visit stmt function
// fix the visit_expr 

// NOTE:
// "In a standard C compiler, the lexer performs a lookup for every identifier it encounters
// to determine its token type. If the identifier is found in the Ordinary Namespace as a 
// typedef, the lexer returns a TYPE_NAME token; otherwise, it returns an IDENTIFIER token."


type_info_t * create_type_info (type_id_t * type, int nesting) {
    type_info_t * type_info = calloc(1, sizeof(type_info_t));
    type_info->type = type;
    type_info->byte_size = 4; // TEMPORARY
    type_info->nesting = nesting;
    return type_info;
}


// for this to error, maybe pass a token to error at the note.
int sym_contains_name(symbol_stack_t * syms, char * name) {
    for (size_t i = syms->len-1; i >= 0; i--) {
        if (ht_contains(syms->tables[i], name)) {
            return 1; }}
    
    return 0;
}

int sym_contains_type(symbol_stack_t * syms, type_id_t * type) {
    if (!(ht_contains(syms->type_table, type) || is_primitive(type->type))) 
        { printf("ERROR: type not defined."); exit(EXIT_FAILURE); }
    return 1;
}

type_id_t * sym_get_type(symbol_stack_t * syms, char * name) {
    type_info_t * t;
    for (size_t i = syms->len-1; i >= 0; i--) {
        if (t = ht_get(syms->tables[i], name)) 
            { return t->type; } }

    printf("ERROR: name not defined."); 
    exit(EXIT_FAILURE);
}


void visit_stmt_block(symbol_stack_t * syms, stmt_block_t * block) {
    sym_stack_push(syms);
    for (size_t i = 0; i < block->len; i++) {
        visit_stmt(syms, block->stmts[i]);
    }
    sym_stack_pop(syms);
}

void visit_func_decl(symbol_stack_t * syms, stmt_func_decl_t * func_decl) {
    sym_contains_type(syms, func_decl->type);
    if (!ht_contains(syms->tables[0], func_decl->func_id->lexeme))
        { ht_put(syms->tables[0], func_decl->func_id->lexeme, func_decl->type); }

    sym_stack_push(syms);
    for (size_t i = 0; i < func_decl->param_len; i++) {
        char * name = func_decl->params[i]->stmt_id_decl->variable;
        ht_put(syms->tables[syms->len-1], name, func_decl->type);
    }
    
    for (size_t i = 0; i < func_decl->block->len; i++) {
        visit_stmt(syms, func_decl->block->stmts[i]);
    }
    sym_stack_pop(syms);
}




void visit_stmt_id_decl(symbol_stack_t * syms, stmt_id_decl_t * id_decl) {
    sym_contains_type(syms, id_decl->type);

    // avoids int i = i; where 'i' already exists in an outer scope. I think.
    if (id_decl->value) 
        { visit_expr(syms , id_decl->value, id_decl->type); }
    
    if (ht_contains(syms->tables[syms->len - 1], id_decl->variable->lexeme))
        { ht_put(syms->tables[syms->len - 1], id_decl->variable, id_decl->type); }

    ht_put(syms->tables[syms->len-1], id_decl->variable, id_decl->type);
}

void visit_stmt_assign(symbol_stack_t * syms, stmt_assign_t * assign) {
    sym_contains_name(syms, assign->variable->lexeme);
    visit_expr(assign->value, sym_get_type(syms, assign->variable->lexeme));
}


void visit_stmt_while(symbol_stack_t * syms, stmt_while_t * while_stmt) {
    visit_expr(while_stmt->cond, &(type_id_t){0, TOKEN_BOOL});
    visit_block_stmt(while_stmt->block);
}

void visit_stmt_if(symbol_stack_t * syms, stmt_if_t * if_stmt) {
    visit_expr(if_stmt->cond, &(type_id_t){0, TOKEN_BOOL});
    visit_block_stmt(if_stmt->then);
    if (if_stmt->or_else) 
        { visit_block_stmt(if_stmt->or_else); }
}

void visit_stmt_return(symbol_stack_t * syms, expr_t * ret_expr) { 
    visit_expr(syms, ret_expr, syms->curr_func->type);
} 

void visit_unary(symbol_stack_t * syms, expr_t * e) {


}

//NEXT: FIX THIS MESS
type_id_t * visit_expr(symbol_stack_t * syms, expr_t * e, type_id_t * expected) {
    // Use binop funcs above for this.
    //recursion yippie :)
    switch (e->tag) {
    case EXPR_BINOP:
        if (!(binop_return_type(e->binop.op) == expected))
            { printf("ERROR: wrong type for binop"); exit(EXIT_FAILURE); }
        
        enum Primitive expected = binop_expected_type(e->binop.op);
        type_id_t * left = visit_expr(syms, e->binop.left, expected);
        type_id_t * left = visit_expr(syms, e->binop.right, expected);
        break;
    case EXPR_FUNC_CALL:
        stmt_func_decl_t * func_decl = ht_get(syms->func_id_to_func, e->func_call.func_id);
        if (!func_decl) 
            { printf("ERROR: function not defined.\n"); exit(EXIT_FAILURE); }
        return func_decl->type;
    case EXPR_NUMERAL:
        return create_type_id(0, TOKEN_NUM);
    case EXPR_UNARY:
        e->unary;
        if (!(unary_expected_type(e->unary.op) == expected_type)) 
            { printf("ERROR: wrong type for unary"); exit(EXIT_FAILURE); }

        visit_expr(e->unary.inner, unary_expected_type(e->unary.op));
        break;
    case EXPR_ID:
        return sym_get_type(syms, e->id.lexeme);
        
    default:
        break;
    }

}

void visit_stmt(symbol_stack_t * syms, stmt_t * stmt) {
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
// this should only be for program
// if (stmt->tag == STMT_FUNC_DECL) {
//     visit_stmt_func_decl();
// }

int main() {
    char * str = calloc(64, sizeof(char));
    for (size_t i = 0; i < 50; i++) {
        str[i] = 'a';
        
    }
    str[50] = '\0';

    const unsigned char * ustr = (const unsigned char *) str;    
    printf("%lu", hash_str(ustr));
    
    return 0;
}

