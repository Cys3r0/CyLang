#include "parser.h"
#include "scanner.h"
#include <stdint.h>
#include <string.h>


#define HASH_TABLE_INITIAL_CAPACITY 64
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


typedef struct {
    stmt_func_decl_t ** program;
    int curr_func;
    htable_t * func_names;
    htable_t * scope_names;
} context_t;

// what do I do per function? Type check during tree traversal or same that until the end of the function?

visit_stmt_id_decl(context_t * context, stmt_id_decl_t * id_decl) {
    add_name_and_type_in_func(context, id_decl->variable, id_decl->type);
    // how/when do I handle if the value assigned a correct type? 
}

void visit_stmt_assign(context_t * context, stmt_assign_t * assign) {
    char * var_type = (char *) ht_get(context->names, assign->variable->lexeme);
    int value = strcmp(var_type, assign->variable);
}


void visit_stmt_id_decl(context_t * context, stmt_while_t * while_stmt) {
    // type check condition against bool

    visit_block_stmt(while_stmt->block);
}

void visit_stmt_block(context_t * context, stmt_block_t * block) {
    for (size_t i = 0; i < block->len; i++) {
        visit_stmt(block->stmts[i]);
    }
}

void visit_stmt_return(context_t * context, expr_t * ret_expr) {
    // type check against the
} 

void typecheck(char * expected) {
    // basically a strcmp between expected and actual 
}

enum Primitives { VOID, I32, BOOL, }; 

enum Primitives binop_return_type(enum TokenType tok_type) {
    if (tok_type == TOKEN_LOG_AND) return BOOL;
    if (tok_type == TOKEN_LOG_OR)  return BOOL;
    if (tok_type == TOKEN_LOG_NOT) return BOOL;

    if (tok_type == TOKEN_LEQ) return BOOL;
    if (tok_type == TOKEN_LT)  return BOOL;
    if (tok_type == TOKEN_GEQ) return BOOL;
    if (tok_type == TOKEN_GT)  return BOOL;
    if (tok_type == TOKEN_EQ)  return BOOL;
    if (tok_type == TOKEN_NEQ) return BOOL;

    if (tok_type == TOKEN_ADD)      return I32;
    if (tok_type == TOKEN_SUB)      return I32;
    if (tok_type == TOKEN_MUL)      return I32;
    if (tok_type == TOKEN_DIV)      return I32;
    if (tok_type == TOKEN_EXPONENT) return I32;

    exit(EXIT_FAILURE);
}

enum Primitives binop_expected_type(enum TokenType tok_type) {
    if (tok_type == TOKEN_LOG_AND) return BOOL;
    if (tok_type == TOKEN_LOG_OR)  return BOOL;
    if (tok_type == TOKEN_LOG_NOT) return BOOL;

    if (tok_type == TOKEN_LEQ) return I32;
    if (tok_type == TOKEN_LT)  return I32;
    if (tok_type == TOKEN_GEQ) return I32;
    if (tok_type == TOKEN_GT)  return I32;
    if (tok_type == TOKEN_EQ)  return I32;
    if (tok_type == TOKEN_NEQ) return I32;

    if (tok_type == TOKEN_ADD)      return I32;
    if (tok_type == TOKEN_SUB)      return I32;
    if (tok_type == TOKEN_MUL)      return I32;
    if (tok_type == TOKEN_DIV)      return I32;
    if (tok_type == TOKEN_EXPONENT) return I32;

    if (tok_type == TOKEN_BIT_AND)     return I32;
    if (tok_type == TOKEN_BIT_OR)      return I32;
    if (tok_type == TOKEN_BIT_XOR)     return I32;

    exit(EXIT_FAILURE);
}

enum Primitives unary_return_type(enum TokenType tok_type) {
    if (tok_type == TOKEN_SUB) return I32;
    if (tok_type == TOKEN_LOG_NOT) return BOOL;
    if (tok_type == TOKEN_BIT_NOT) return I32;

    exit(EXIT_FAILURE);
}

enum Primitives unary_expected_type(enum TokenType tok_type) {
    if (tok_type == TOKEN_SUB) return I32;
    if (tok_type == TOKEN_LOG_NOT) return BOOL;
    if (tok_type == TOKEN_BIT_NOT) return I32;

    exit(EXIT_FAILURE);
}

enum Primitives unary_expected_type(enum TokenType tok_type) {
    if (tok_type == TOKEN_LOG_AND) return BOOL;
    if (tok_type == TOKEN_LOG_OR)  return BOOL;
    if (tok_type == TOKEN_LOG_NOT) return BOOL;

    if (tok_type == TOKEN_LEQ) return I32;
    if (tok_type == TOKEN_LT)  return I32;
    if (tok_type == TOKEN_GEQ) return I32;
    if (tok_type == TOKEN_GT)  return I32;
    if (tok_type == TOKEN_EQ)  return I32;
    if (tok_type == TOKEN_NEQ) return I32;

    if (tok_type == TOKEN_ADD)      return I32;
    if (tok_type == TOKEN_SUB)      return I32;
    if (tok_type == TOKEN_MUL)      return I32;
    if (tok_type == TOKEN_DIV)      return I32;
    if (tok_type == TOKEN_EXPONENT) return I32;

    exit(EXIT_FAILURE);
}


void visit_expr_func_call(context_t * context, expr_func_call_t * func_call) { 
    
    // add check that func name is in context hash table 
    
}


// We want to chain some expected types
char * visit_expr(expr_t * e, enum Primitive expected_type) {
    // Use binop funcs above for this.
    //recursion yippie :)
    switch (e->tag) {
    case EXPR_BINOP:
        if (!(binop_return_type(e->binop.op) == expected_type)) 
            { printf("ERROR: wrong type for binop"); exit(EXIT_FAILURE); }
        
        enum Primitive expected = binop_expected_type(e->binop.op);
        visit_expr(e->binop.left, expected);
        visit_expr(e->binop.right, expected);
        break;
    case EXPR_FUNC_CALL:
        
        break;
    case EXPR_NUMERAL:
        if (expected_type != I32) 
            { printf("ERROR: wrong type for numeral"); exit(EXIT_FAILURE); }  
        break;
    case EXPR_UNARY:
        if (!(unary_expected_type(e->unary.op) == expected_type)) 
            { printf("ERROR: wrong type for unary"); exit(EXIT_FAILURE); }

        visit_expr(e->unary.inner, unary_expected_type(e->unary.op));
        break;
    case EXPR_ID:
        /* code */
        break;
        
    default:
        break;
    }

}







// For each function we have a hash_table with key being a variable name and 
// value being the type of said name. All names require a type. 
// 
// We go through the AST, where we find declaration everywhere. At a declaration
// we add the name to the hash_table, and it's type. 
//
// Any assignment to said variable must be of the same type of the variable, so 
// we must type check the values assigning. How do we do this?
// - for IDs (id_use (rename maybe?)) we can just check the type since we assume 
//   non-id assignment has been type checked. 
// - for non-id values, i.e Numerals for the moment (but floats and such later on)
//   a set of primitive types like char, i8, u8, f16, etc. are required. 
// - for binops, a check is needed partially to check whether the given exprs 
//   are valud for the op (bool + bool, or u8 + i8). Some kind of lookup table is 
//   needed here. In any case a lookup table has to be declared for handling this.
//      - should the type of binop be stored with the binop? probably poorer 
//        if so, no? 
// - for unary ops, an additional table is needed  (e.g. is - bool valid?)
//
// How does this look? A DFS into the lowest node, taking it's type, and then 
// validating up from there? Perhaps that could work.
//
// What am I going to do:
// 1. Create lookup tables the types of binops, i.e their return type. Another 
//    table for inputs to the binop.
// 2. Introduce primitives like char, bool, i64, etc. ,
// 3. Give the type of numerals here in the correcter file,
//      a. look at the value given 
//      b. if it's a numeral, give it a i64 as standard
//      c. if it's a logical binop, give it bool
//  
// START HERE:
// The right idea is probably: go through the whole AST, find all names and all types,
// put them in a table. After the whole AST has been searched, do type checking. 
// And here type checking mean checking whether types of names match assignments,
// whether inputs to "if" and "while" conditions are boolean, whether inputs to 
// binops of correct types, etc.  
//
// a get type for all nodes. Or is that too much OOP?





void visit_stmt(stmt_t * stmt) {
    // these need to be implemented
    // if (stmt->tag == STMT_IF) {
    //     visit_stmt_if();
    // }
    // if (stmt->tag == STMT_ID_DECL) {
    //     visit_stmt_id_decl();
    // }
    // if (stmt->tag == STMT_ASSIGN) {
    //     visit_stmt_assign();
    // }
    // if (stmt->tag == STMT_FUNC_CALL) {
    //     visit_stmt_func_call();
    // }
    // if (stmt->tag == STMT_WHILE) {
    //     visit_stmt_while();
    // }
    // if (stmt->tag == STMT_RETURN) {
    //     visit_stmt_return();
    // }
    // if (stmt->tag == STMT_BLOCK) {
    //     visit_stmt_block();
    // }
}
// this should only be for program
// if (stmt->tag == STMT_FUNC_DECL) {
//     visit_stmt_func_decl()
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

