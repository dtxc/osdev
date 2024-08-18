#include <string.h>
#include <asm/io.h>
#include <mm/kheap.h>

void memset(void *dst, uint8_t c, int n) {
    int i, m;
    unsigned long *wdst = dst;
    long c1 = c + (c << 8) + (c << 16) + (c << 24);

    for (i = 0, m = n / sizeof(long); i < m; i++) {
        *(wdst++) = c1;
    }

    for (i = 0, m = n % sizeof(long); i < m; i++) {
        *(((uint8_t *) wdst) + i) = c;
    }
}

void memcpy(void *dst, void *src, int num) {
    for (int i = 0; i < num; i++) {
        *((uint8_t *) (dst + i)) = *((uint8_t *) (src + i));
    }
}

void memmove(void *dst, void *src, uint32_t n) {
    uint8_t *d = (uint8_t *) dst;
    uint8_t *s = (uint8_t *) src;

    if (d > s && d < s + n) {
        d += n;
        s += n;
        while (n--) {
            *(--d) = *(--s);
        }
    } else {
        while (n--) {
            *d++ = *s++;
        }
    }
}

int strcpy(char *dst, const char *src) {
    int i = 0;
    while (src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0'; // set null terminator

    return i;
}

int strncpy(char *dst, const char *src, int n) {
    if (strlen(src) < n) {
        n = strlen(src);
    }

    int i;
    for (i = 0; i < n; i++) {
        dst[i] = src[i];
    }

    return i;
}

int strlen(const char *s) {
    int i = 0;
    while (*s != '\0') {
        i++;
        s++;
    }

    return i;
}

int strcmp(char *s, const char *s1) {
    int len = strlen(s);

    if (len != strlen(s1)) {
        return 0;
    }

    for (int i = 0; i < len; i++) {
        if (s[i] != s1[i]) {
            return 0;
        }
    }

    return 1;
}

int strncmp(char *s, const char *s1, int n) {
    for (int i = 0; i < n; i++) {
        if (s[i] != s1[i]) {
            return 0;
        }
    }

    return 1;
}

int itoa(char *dst, int n) {
    uint32_t start = 0;

    if (n == 0) {
        dst[0] = '0';
        dst[1] = '\0';
        return 0;
    }

    if (n < 0) {
        dst[start++] = '-';
        n = -n;
    }

    char buff[16];
    buff[15] = 0;
    uint32_t i = 14;

    while (n > 0) {
        uint32_t rem = n % 10;
        buff[i--] = '0' + rem;
        n /= 10;
    }

    strcpy(dst + start, buff + i + 1);
    return start + 14 - i;
}

int int2hex(char *dst, uint32_t n) {
    uint32_t start = 0;

    if (n == 0) {
        dst[0] = '0';
        dst[1] = '\0';
        return 0;
    }

    if (n < 0) {
        dst[start++] = '-';
        n = -n;
    }

    char buff[16];
    buff[15] = 0;
    uint32_t i = 14;

    while (n > 0) {
        uint32_t rem = n % 16;
        if (rem <= 9) {
            buff[i--] = '0' + rem;
        } else {
            buff[i--] = 'A' + (rem - 10);
        }
        n /= 16;
    }

    strcpy(dst + start, buff + i + 1);
    return start + 14 + i;
}

int split_string(char *str, char delimiter, char ***tokens) {
    int token_count = 0;
    int str_len = strlen(str);
    int i, j, start;

    // count the number of tokens
    for (i = 0; i < str_len; i++) {
        if (str[i] == delimiter) {
            token_count++;
        }
    }
    token_count++;

    *tokens = (char **) kmalloc(token_count * sizeof(char*));
    if (*tokens == NULL) {
        return -1;
    }

    // extract the tokens
    start = 0;
    j = 0;
    for (i = 0; i <= str_len; i++) {
        if (str[i] == delimiter || str[i] == '\0') {
            int token_len = i - start;
            (*tokens)[j] = (char *) kmalloc((token_len + 1) * sizeof(char));
            if ((*tokens)[j] == NULL) {
                return -1;
            }
            strncpy((*tokens)[j], &str[start], token_len);
            (*tokens)[j][token_len] = '\0';
            j++;
            start = i + 1;
        }
    }

    return token_count;
}
