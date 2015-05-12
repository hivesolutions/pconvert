#include "stdafx.h"

char *join_path(char *base, char *extra, char *result) {
    char *original = result;
    size_t base_l = strlen(base);
    size_t extra_l = strlen(extra);
    memcpy(result, base, base_l);
    result += base_l;
    memcpy(result, extra, extra_l);
    result += extra_l;
    *result = '\0';
    return original;
}
