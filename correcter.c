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
} hash_table_t;



uint64_t hash_str(const unsigned char *s) {
    // hashing function stolen from wikipedia
    __uint128_t h = (__uint128_t)(a[0]);
    
    for (int i = 0; i < MAX_LEXEME_LENGTH; i++)
    h += (__uint128_t)(a[i + 1]) * (__uint128_t)(s[i]);
    
    return (uint64_t)(h >> W);
}


hash_table_t * create_hash_table() {
    entry_t * entries = calloc(HASH_TABLE_INITIAL_CAPACITY, sizeof(entry_t)); // this should initialize to EMPTY
    hash_table_t * table = calloc(1, sizeof(hash_table_t));
    table->entries = entries;
    table->cap = HASH_TABLE_INITIAL_CAPACITY;

    return table;
}

void hash_table_resize(hash_table_t * table, int make_bigger) {
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


int hash_table_put(hash_table_t * table, char * key, void * value) {
    // returns 0 if key was already in table
    entry_t curr_entry;
    int j;
    int dead_idx = -1;
    int hash_idx;
    
    if (table->len / table->cap > 0.7)
        { hash_table_resize(table, 1); }

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

void * hash_table_get(hash_table_t * table, char * key) {
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

int hash_table_contains(hash_table_t * table, char * key) {
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

void * hash_table_del(hash_table_t * table, char * key) {
    entry_t curr_entry;
    int hash_idx;
    
    if (table->len / table->cap < 0.3 && table->len > 64)
        { hash_table_resize(table, 0); }

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
    hash_table_t * func_names;
} context_t;


typedef struct {
    context_t * global;
    stmt_func_decl_t * func;
    hash_table_t * names;
} func_context_t;

void add_name_and_type_in_func(func_context_t * context, char * name, char * type) {
    // Maybe keep pointer to token to keep col and row for error messages
    // in id_decls and such instead of raw strings
    if (hash_table_contains(context->global->func_names, name)) 
        { println("Name conflict with function.\n"); exit(EXIT_FAILURE); }
    
    // Maybe create a put_if_none() ??
    if (hash_table_contains(context->names, name)) 
        { println("Name conflict within same function.\n"); exit(EXIT_FAILURE); }

    hash_table_put(context->names, name, type); 
}

void add_name_and_type_in_program(context_t * context, char * name, char * type) {
    if (hash_table_contains(context->func_names, name)) 
        { println("Name conflict with function.\n"); exit(EXIT_FAILURE); }

    hash_table_put(context->func_names, name, type); 
}

visit_stmt_id_decl(func_context_t * context, stmt_id_decl_t * id_decl) {
    add_name_and_type_in_func(context, id_decl->variable, id_decl->type);
    // how/when do I handle if the value assigned a correct type? 
}




visit_stmt_assign(stmt_assign_t * assign) {
    // We're going to assume that the decls have to come first in this language
    // This can be changed later easily, perhaps check assignments in a function last
    
    

}



void visit_stmt(stmt_t * stmt) {
    if (stmt->tag == STMT_IF) ;
    if (stmt->tag == STMT_ID_DECL) ;
    if (stmt->tag == STMT_ASSIGN) ;
    if (stmt->tag == STMT_FUNC_CALL) ;
    if (stmt->tag == STMT_WHILE) ;
    if (stmt->tag == STMT_RETURN) ;
    if (stmt->tag == STMT_FUNC_DECL) ;
    if (stmt->tag == STMT_BLOCK) ;
}




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










