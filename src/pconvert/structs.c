#include "stdafx.h"

static struct nlist_t *hashtab[HASHSIZE];

struct nlist_t *get_map(char *key) {
    struct nlist_t *np;
    for(np = hashtab[hash_str(key)]; np != NULL; np = np->next) {
        if(strcmp(key, np->key) != 0) { continue; }
        return np;
    }
    return NULL;
}

struct nlist_t *set_map(char *key, void *value) {
    struct nlist_t *np;
    unsigned hashval;
    np = get_map(key);
    if(np == NULL) {
        np = (struct nlist_t *) malloc(sizeof(*np));
        if(np == NULL || (np->key = copy_str(key)) == NULL) {
            return NULL;
        }
        hashval = hash_str(key);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;
    }
    np->value = value;
    return np;
}

void *value_map(char *key) {
    struct nlist_t *np;
    np = get_map(key);
    if(np == NULL) { return NULL; }
    return np->value;
}

size_t hash_str(char *value) {
    size_t hashval;
    for(hashval = 0; *value != '\0'; value++) {
        hashval = *value + 31 * hashval;
    }
    return hashval % HASHSIZE;
}

char *copy_str(char *value) {
    char *duplicate;
    duplicate = (char *) malloc(strlen(value) + 1);
    if(duplicate != NULL) { strcpy(duplicate, value); }
    return duplicate;
}
