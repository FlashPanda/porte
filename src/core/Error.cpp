
#include <core/error.h>
#include <core/StringPrint.h>
#include <core/parallel.h>
#include <core/progressreporter.h>
#include <core/parser.h>

#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <stdarg.h>
#include <chrono>
#include <thread>
#include <memory>
#include <string>
#include <vector>

namespace porte {

// 报错函数
template <typename... Args>
static std::string StringVaprintf(const std::string &fmt, va_list args) {
    // 算出要分配多少空间；包括额外的一个'\0'的空间。
    va_list argsCopy;
    va_copy(argsCopy, args);
    size_t size = vsnprintf(nullptr, 0, fmt.c_str(), args) + 1;
    std::string str;
    str.resize(size);
    vsnprintf(&str[0], size, fmt.c_str(), argsCopy);
    str.pop_back();  // 移除尾部的NUL
    return str;
}

static void ProcessError(Loc *loc, const char *format, va_list args,
                         const char *errorType) {
    // 构建完整的格式化错误字符串，一次性打印出来。
    // 这样，如果多个线程需要一次性打印消息，它们之间不会混淆
    std::string errorString;

    // 打印输入文件的行号及位置，如果有这些数据的话
    if (loc)
        errorString = StringPrintf("%s:%d:%d: ", loc->filename.c_str(),
                                   loc->line, loc->column);

    errorString += errorType;
    errorString += ": ";
    errorString += StringVaprintf(format, args);

    // 打印错误信息（最多只打印一次）
    static std::string lastError;
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    if (errorString != lastError) {
        LOG(INFO) << errorString;
        fprintf(stderr, "%s\n", errorString.c_str());
        lastError = errorString;
    }
}

void Warning(const char *format, ...) {
    if (PbrtOptions.quiet) return;
    va_list args;
    va_start(args, format);
    ProcessError(parserLoc, format, args, "Warning");
    va_end(args);
}

void Error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    ProcessError(parserLoc, format, args, "Error");
    va_end(args);
}

}  // namespace porte
