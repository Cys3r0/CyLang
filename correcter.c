#include "parser.h"
#include "scanner.h"
#include <stdint.h>

#define INITIAL_VALUE 250323
#define W 64

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


uint64_t hash_str(const unsigned char *s) {
    // @test this
    __uint128_t h = (__uint128_t)(a[0]);

    for (int i = 0; i < MAX_LEXEME_LENGTH; i++)
        h += (__uint128_t)(a[i + 1]) * (__uint128_t)(s[i]);

    return (uint64_t)(h >> W);
}


int main() {
    
    return 0;
}










