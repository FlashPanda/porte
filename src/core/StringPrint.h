#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_CORE_STRINGPRINT_H
#define PORTE_CORE_STRINGPRINT_H

#include <core/porte.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <inttypes.h>

namespace porte {

inline void stringPrintfRecursive(std::string *s, const char *fmt) {
    const char *c = fmt;
 
    while (*c) {
        if (*c == '%') {
            //CHECK_EQ(c[1], '%');
            ++c;
        }
        *s += *c++;
    }
}

// 1. 从 fmt 复制到 *s，直到下一个格式化指令。
// 2. 将 fmt 推进到下一个格式化指令之后，并将格式化指令作为字符串返回。
inline std::string copyToFormatString(const char **fmt_ptr, std::string *s) {
    const char *&fmt = *fmt_ptr;
    while (*fmt) {
        if (*fmt != '%') {
            *s += *fmt;
            ++fmt;
        } else if (fmt[1] == '%') {
            // "%%"; 让其通过
            *s += '%';
            *s += '%';
            fmt += 2;
        } else
            // fmt位于格式化指令的开头
            break;
    }

    std::string nextFmt;
    if (*fmt) {
        do {
            nextFmt += *fmt;
            ++fmt;
            // 格式化指令的不完整测试：一个新的格式化指令开始，我们点击空格，或者我们点击逗号。
        } while (*fmt && *fmt != '%' && !isspace(*fmt) && *fmt != ',' &&
                 *fmt != '[' && *fmt != ']' && *fmt != '(' && *fmt != ')');
    }

    return nextFmt;
}

template <typename T>
inline std::string formatOne(const char *fmt, T v) {
    // 算出需要分配多少空间，包括最后一个'\0'
    size_t size = snprintf(nullptr, 0, fmt, v) + 1;
    std::string str;
    str.resize(size);
    snprintf(&str[0], size, fmt, v);
    str.pop_back();  // 移除尾部的NUL
    return str;
}

// 通用版的stringPrintfRecursive；
// 将单个 StringPrintf() 参数的格式化输出添加到最终结果字符串*s 中。
template <typename T, typename... Args>
inline void stringPrintfRecursive(std::string *s, const char *fmt, T v,
                                  Args... args) {
    std::string nextFmt = copyToFormatString(&fmt, s);
    *s += formatOne(nextFmt.c_str(), v);
    stringPrintfRecursive(s, fmt, args...);
}

// 特化版本，浮点参数
template <typename... Args>
inline void stringPrintfRecursive(std::string *s, const char *fmt, float v,
                                  Args... args) {
    std::string nextFmt = copyToFormatString(&fmt, s);
    if (nextFmt == "%f")
        // 用足够的精度给出精确的浮点值，如果它是用来初始化一个float
        // https://randomascii.wordpress.com/2012/03/08/float-precisionfrom-zero-to-100-digits-2/
        *s += formatOne("%.9g", v);
    else
        // 如果不是用的"%f"，那么保留它
        *s += formatOne(nextFmt.c_str(), v);

    // 继续打印下一个参数
    stringPrintfRecursive(s, fmt, args...);
}

// 使用足够的精度来特化double
template <typename... Args>
inline void stringPrintfRecursive(std::string *s, const char *fmt, double v,
                                  Args... args) {
    std::string nextFmt = copyToFormatString(&fmt, s);
    if (nextFmt == "%f")
        *s += formatOne("%.17g", v);
    else
        *s += formatOne(nextFmt.c_str(), v);
    stringPrintfRecursive(s, fmt, args...);
}

// StringPrintf()是用来替代sprintf()的，返回值是一个std::string。这比用std::cout
// 更容易进行格式化输出。
// 
// 浮点输出（"%f"）已经特殊处理过了。
template <typename... Args>
inline std::string StringPrintf(const char *fmt, Args... args) {
    std::string ret;
    stringPrintfRecursive(&ret, fmt, args...);
    return ret;
}

}  // namespace porte

#endif  // PORTE_CORE_STRINGPRINT_H
