#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define MAX(a, b) ((a) > (b) ? (a) : (b))

enum Token {
    ID,
    NUM,
    ASSIGN,
    PLUS,
    LPAR,
    RPAR,
    LBRACKET,
    RBRACKET,


};

typedef struct {
    char * str;
    int cap;
    int len;
} String;

//TODO: 
// Test if BOTH string_add functions work
// Set up enums for tokens 

void resize_string(String * s, int new_cap) {
    char * new_s = realloc(s->str, new_cap);
    if (!new_s) {
        printf("Could not allocate more memory while resizing string.\n");
        exit(EXIT_FAILURE);
    } 

    s->str = new_s;
    s->cap = new_cap;
}

void string_add_char(String * s, char c) {
    if (s->len+1 == s->cap) {
        resize_string(s, s->cap << 1);
    }

    s->str[s->len] = c;
    s->str[s->len+1] = '\0';
    s->len++;
}


void string_add_string(String * s1, String * s2) {          // does not free s2.
    if (s1->len + 1 + s2->len + 1 > s1->cap) {
        resize_string(s1, (s1->len + s2->len) << 1);
    }

    for (int i = 0; i < s2->len; i++) {                     // optimize(!)
        s1->str[s1->len + i] = s2->str[i];
    }
    
    s1->len += s2->len;
    s1->str[s1->len] = '\0';
}

void init_string(String * s, int cap) {
    s->cap = cap;
    s->str = malloc(cap);
    s->len = 0;
}

void string_cpy(String * string, char * str, int str_len) {
    if (str_len+1 > string->cap) {
        resize_string(string, string->cap << 1);
    }

    for (int i = 0; i < str_len; i++) {                     // optimize(!)
        string->str[i] = str[i];
    }
    string->len = str_len;
}


int main() {
    String * s1 = malloc(sizeof(String));
    String * s2 = malloc(sizeof(String));
    
    init_string(s1, 4);
    init_string(s2, 4);
    

    string_cpy(s1, "", 0);
    string_cpy(s2, "123", 3);

    for (int i = 0; i < 20; i++) {
        string_add_string(s1, s2);
    }

    exit(EXIT_SUCCESS);
}