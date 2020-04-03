//
// Created by HuBin on 2020/4/3.
//

#ifndef MINIFS_UTIL_H
#define MINIFS_UTIL_H

#include <cstdio>
#include <cstring>

inline bool startsWith(const char *pre, const char *str) {
    size_t lenpre = strlen(pre),
            lenstr = strlen(str);
    return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

#endif //MINIFS_UTIL_H
