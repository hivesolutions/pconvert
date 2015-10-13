#pragma once

#define HASHSIZE 100

typedef struct nlist_t {
    struct nlist_t *next;
    char *key;
    void *value;
} nlist;

struct nlist_t *get_map(char *key);
struct nlist_t *put_map(char *key, void *value);
size_t hash_str(char *value);
char *copy_str(char *value);
