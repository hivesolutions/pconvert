#pragma once

#define HASHSIZE 100

typedef union typem_t {
    char boolean;
    long integer;
    float decimal;
    char *string;
} typem;

typedef struct param_t {
    char *key;
    union typem_t value;
} param;

typedef struct params_t {
    size_t length;
    struct param_t *params;
} params;

typedef struct nlist_t {
    struct nlist_t *next;
    char *key;
    void *value;
} nlist;

struct nlist_t *get_map(char *key);
struct nlist_t *set_map(char *key, void *value);
void *value_map(char *key);
size_t hash_str(char *value);
char *copy_str(char *value);
