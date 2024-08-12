#include <string.h>

void *memset(void *dst, uint8_t c, int n) {
    int i, m;
    unsigned long *wdst = dst;
    long c1 = c + (c << 8) + (c << 16) + (c << 24);

    for (i = 0, m = n / sizeof(long); i < m; i++) {
        *(wdst++) = c1;
    }

    for (i = 0, m = n % sizeof(long); i < m; i++) {
        *(((uint8_t *) wdst) + i) = c;
    }

    return dst;
}

int strcpy(char *dst, const char *src) {
    int i = 0;
    while (1) {
        char c = *src;
        *dst = c;

        if (c == '\0') {
            break;
        }

        i++;
        dst++;
        src++;
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
