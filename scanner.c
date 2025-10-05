#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define MAX(a, b) ((a) > (b) ? (a) : (b))



typedef struct {
    char * str;
    int cap;
    int len;
} String;

//TODO: test if BOTH string_add functions work
//TODO: 
//TODO: set up github for compiler project.


void resize_string(String * s, int new_cap) {
    char * new_s = realloc(s->str, new_cap);
    if (!new_s) {
        printf("Could not allocate more memory while resizing string.\n");
        exit(EXIT_FAILURE);
    } 
    free(s->str);
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


void string_add_char(String * s1, String * s2) {    // does not free s2.
    if (s1->len + s2->len > s1->cap) {
        resize_string(s1, (s1->len + s2->len) << 1);
    }

    for (int i = 0; i < s2->len; i++) {      // optimize(!)
        s1->str[s1->len + i] = s2->str[i];
    }
    
    s1->len += s2->len;
    s1->str[s1->len+1] = '\0';
}

void init_string(String * s, char * start_string) {
    s->cap = 4;
    s->str = malloc(4);
    s->len = 0;
    strcpy_s(s->str, s->cap, start_string);
    // printf("%s", s->str);
}


int main() {
    String * s = malloc(sizeof(String));
    // printf("awdkajwndkjaw\n");
    // printf("Oh there's sober men a' plenty, And drunkards barely 20!\n");
    
    init_string(s, "");
    for (int i = 0; i < 20; i++){
        string_add_char(s, '0');
        printf("%s\n", s->str);
        printf("%d\n", s->cap);
    }
    
    

    printf("%s", s->str);
}