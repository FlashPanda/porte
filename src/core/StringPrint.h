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

// 1. �� fmt ���Ƶ� *s��ֱ����һ����ʽ��ָ�
// 2. �� fmt �ƽ�����һ����ʽ��ָ��֮�󣬲�����ʽ��ָ����Ϊ�ַ������ء�
inline std::string copyToFormatString(const char **fmt_ptr, std::string *s) {
    const char *&fmt = *fmt_ptr;
    while (*fmt) {
        if (*fmt != '%') {
            *s += *fmt;
            ++fmt;
        } else if (fmt[1] == '%') {
            // "%%"; ����ͨ��
            *s += '%';
            *s += '%';
            fmt += 2;
        } else
            // fmtλ�ڸ�ʽ��ָ��Ŀ�ͷ
            break;
    }

    std::string nextFmt;
    if (*fmt) {
        do {
            nextFmt += *fmt;
            ++fmt;
            // ��ʽ��ָ��Ĳ��������ԣ�һ���µĸ�ʽ��ָ�ʼ�����ǵ���ո񣬻������ǵ�����š�
        } while (*fmt && *fmt != '%' && !isspace(*fmt) && *fmt != ',' &&
                 *fmt != '[' && *fmt != ']' && *fmt != '(' && *fmt != ')');
    }

    return nextFmt;
}

template <typename T>
inline std::string formatOne(const char *fmt, T v) {
    // �����Ҫ������ٿռ䣬�������һ��'\0'
    size_t size = snprintf(nullptr, 0, fmt, v) + 1;
    std::string str;
    str.resize(size);
    snprintf(&str[0], size, fmt, v);
    str.pop_back();  // �Ƴ�β����NUL
    return str;
}

// ͨ�ð��stringPrintfRecursive��
// ������ StringPrintf() �����ĸ�ʽ�������ӵ����ս���ַ���*s �С�
template <typename T, typename... Args>
inline void stringPrintfRecursive(std::string *s, const char *fmt, T v,
                                  Args... args) {
    std::string nextFmt = copyToFormatString(&fmt, s);
    *s += formatOne(nextFmt.c_str(), v);
    stringPrintfRecursive(s, fmt, args...);
}

// �ػ��汾���������
template <typename... Args>
inline void stringPrintfRecursive(std::string *s, const char *fmt, float v,
                                  Args... args) {
    std::string nextFmt = copyToFormatString(&fmt, s);
    if (nextFmt == "%f")
        // ���㹻�ľ��ȸ�����ȷ�ĸ���ֵ���������������ʼ��һ��float
        // https://randomascii.wordpress.com/2012/03/08/float-precisionfrom-zero-to-100-digits-2/
        *s += formatOne("%.9g", v);
    else
        // ��������õ�"%f"����ô������
        *s += formatOne(nextFmt.c_str(), v);

    // ������ӡ��һ������
    stringPrintfRecursive(s, fmt, args...);
}

// ʹ���㹻�ľ������ػ�double
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

// StringPrintf()���������sprintf()�ģ�����ֵ��һ��std::string�������std::cout
// �����׽��и�ʽ�������
// 
// ���������"%f"���Ѿ����⴦����ˡ�
template <typename... Args>
inline std::string StringPrintf(const char *fmt, Args... args) {
    std::string ret;
    stringPrintfRecursive(&ret, fmt, args...);
    return ret;
}

}  // namespace porte

#endif  // PORTE_CORE_STRINGPRINT_H
