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

blend_algorithm *get_blend_algorithm(char *algorithm) {
    if(algorithm == NULL || strcmp(algorithm, "alpha") == 0) {
        return blend_alpha;
    } else if(strcmp(algorithm, "multiplicative") == 0) {
        return blend_multiplicative;
    } else if(strcmp(algorithm, "source_over") == 0) {
        return blend_source_over;
    } else if(strcmp(algorithm, "disjoint_over") == 0) {
        return blend_disjoint_over;
    } else if(strcmp(algorithm, "disjoint_under") == 0) {
        return blend_disjoint_under;
    } else if(strcmp(algorithm, "disjoint_debug") == 0) {
        return blend_disjoint_debug;
    } else {
        abort_("[blend_images] Invalid algorithm value");
        return NULL;
    }
}

char is_multiplied(char *algorithm) {
    if(algorithm == NULL || strcmp(algorithm, "alpha") == 0) {
        return FALSE;
    } else if(strcmp(algorithm, "multiplicative") == 0) {
        return FALSE;
    } else if(strcmp(algorithm, "source_over") == 0) {
        return FALSE;
    } else if(strcmp(algorithm, "disjoint_over") == 0) {
        return TRUE;
    } else if(strcmp(algorithm, "disjoint_under") == 0) {
        return TRUE;
    } else if(strcmp(algorithm, "disjoint_debug") == 0) {
        return TRUE;
    } else {
        abort_("[blend_images] Invalid algorithm value");
        return FALSE;
    }
}

void blend_alpha(
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
) {
    png_byte r, g, b, a;

    float abf = 1.0f * (ab / 255.0f);
    float atf = 1.0f * (at / 255.0f);
    float af = atf + abf * (1.0f - atf);

    r = af == 0.0f ? 0 : (png_byte) ((rb * abf + rt * atf * (1.0f - abf)) / af);
    g = af == 0.0f ? 0 : (png_byte) ((gb * abf + gt * atf * (1.0f - abf)) / af);
    b = af == 0.0f ? 0 : (png_byte) ((bb * abf + bt * atf * (1.0f - abf)) / af);
    a = MAX(0, MIN(255, (png_byte) ((abf + atf * (1.0f - abf)) * 255.0f)));

    r = MAX(0, MIN(255, r));
    g = MAX(0, MIN(255, g));
    b = MAX(0, MIN(255, b));

    *result = r;
    *(result + 1) = g;
    *(result + 2) = b;
    *(result + 3) = a;
}

void blend_multiplicative(
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
) {
    png_byte r, g, b, a;

    float atf = 1.0f * (at / 255.0f);

    r = (png_byte) (rb * (1 - atf) + rt * atf);
    g = (png_byte) (gb * (1 - atf) + gt * atf);
    b = (png_byte) (bb * (1 - atf) + bt * atf);
    a = MAX(0, MIN(255, at + ab));

    r = MAX(0, MIN(255, r));
    g = MAX(0, MIN(255, g));
    b = MAX(0, MIN(255, b));

    *result = r;
    *(result + 1) = g;
    *(result + 2) = b;
    *(result + 3) = a;
}

void blend_source_over(
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
) {
    png_byte r, g, b, a;

    float abf = 1.0f * (ab / 255.0f);
    float atf = 1.0f * (at / 255.0f);
    float af = abf + atf * (1.0f - abf);

    r = af == 0.0f ? 0 : (png_byte) ((rb * abf + rt * atf * (1.0f - abf)) / af);
    g = af == 0.0f ? 0 : (png_byte) ((gb * abf + gt * atf * (1.0f - abf)) / af);
    b = af == 0.0f ? 0 : (png_byte) ((bb * abf + bt * atf * (1.0f - abf)) / af);
    a = MAX(0, MIN(255, (png_byte) (af * 255.0f)));

    r = MAX(0, MIN(255, r));
    g = MAX(0, MIN(255, g));
    b = MAX(0, MIN(255, b));

    *result = r;
    *(result + 1) = g;
    *(result + 2) = b;
    *(result + 3) = a;
}

void blend_disjoint_under(
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
) {
    png_byte r, g, b, a;

    float abf = 1.0f * (ab / 255.0f);
    float atf = 1.0f * (at / 255.0f);

    r = (png_byte) ((atf * abf) > 0.0f ? rt / atf * (1.0f - abf) + rb: rt * (1.0f - abf) + rb);
    g = (png_byte) ((atf * abf) > 0.0f ? gt / atf * (1.0f - abf) + gb: gt * (1.0f - abf) + gb);
    b = (png_byte) ((atf * abf) > 0.0f ? bt / atf * (1.0f - abf) + bb: bt * (1.0f - abf) + bb);
    a = MAX(0, MIN(255, at + ab));

    r = MAX(0, MIN(255, r));
    g = MAX(0, MIN(255, g));
    b = MAX(0, MIN(255, b));

    *result = r;
    *(result + 1) = g;
    *(result + 2) = b;
    *(result + 3) = a;
}

void blend_disjoint_over(
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
) {
    png_byte r, g, b, a;

    float abf = 1.0f * (ab / 255.0f);
    float atf = 1.0f * (at / 255.0f);

    r = (png_byte) ((atf + abf) < 1.0f ? rt + rb * (1.0f - atf) / abf : rt + rb);
    g = (png_byte) ((atf + abf) < 1.0f ? gt + gb * (1.0f - atf) / abf : gt + gb);
    b = (png_byte) ((atf + abf) < 1.0f ? bt + bb * (1.0f - atf) / abf : bt + bb);
    a = MAX(0, MIN(255, at + ab));

    r = MAX(0, MIN(255, r));
    g = MAX(0, MIN(255, g));
    b = MAX(0, MIN(255, b));

    *result = r;
    *(result + 1) = g;
    *(result + 2) = b;
    *(result + 3) = a;
}

void blend_disjoint_debug(
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
) {
    png_byte r, g, b, a;

    float abf = 1.0f * (ab / 255.0f);
    float atf = 1.0f * (at / 255.0f);

    r = (png_byte) (atf + abf < 1.0 ? 0 : 255);
    g = (png_byte) (atf + abf < 1.0 ? 255 : 0);
    b = (png_byte) (atf + abf < 1.0 ? 0 : 0);
    a = MAX(0, MIN(255, at + ab));

    r = MAX(0, MIN(255, r));
    g = MAX(0, MIN(255, g));
    b = MAX(0, MIN(255, b));

    *result = r;
    *(result + 1) = g;
    *(result + 2) = b;
    *(result + 3) = a;
}
