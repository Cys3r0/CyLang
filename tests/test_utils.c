#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

void read_file_stdin(char ** source, int * source_len) {
    size_t cap = 4096;
    size_t len = 0;
    char * buf = calloc(cap, sizeof(char));
    int n;

    while((n = read(0, buf + len, cap - len)) > 0) {
        len += n;
        if (len == cap) {
            cap *= 2;
            buf = realloc(buf, cap);
        }
    }

    buf = realloc(buf, len + 1);
    buf[len] = '\0';

    *source = buf;
    *source_len = len;
}
