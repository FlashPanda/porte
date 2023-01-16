#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_CORE_FILEUTIL_H
#define PORTE_CORE_FILEUTIL_H

#include <core/porte.h>
#include <string>
#include <cctype>
#include <string.h>

namespace porte {

bool IsAbsolutePath(const std::string &filename);
std::string AbsolutePath(const std::string &filename);
std::string ResolveFilename(const std::string &filename);
std::string DirectoryContaining(const std::string &filename);
void SetSearchDirectory(const std::string &dirname);

inline bool HasExtension(const std::string &value, const std::string &ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(
        ending.rbegin(), ending.rend(), value.rbegin(),
        [](char a, char b) { return std::tolower(a) == std::tolower(b); });
}

}  // namespace porte

#endif  // PORTE_CORE_FILEUTIL_H
