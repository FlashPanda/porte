
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

// ������
template <typename... Args>
static std::string StringVaprintf(const std::string &fmt, va_list args) {
    // ���Ҫ������ٿռ䣻���������һ��'\0'�Ŀռ䡣
    va_list argsCopy;
    va_copy(argsCopy, args);
    size_t size = vsnprintf(nullptr, 0, fmt.c_str(), args) + 1;
    std::string str;
    str.resize(size);
    vsnprintf(&str[0], size, fmt.c_str(), argsCopy);
    str.pop_back();  // �Ƴ�β����NUL
    return str;
}

static void ProcessError(Loc *loc, const char *format, va_list args,
                         const char *errorType) {
    // ���������ĸ�ʽ�������ַ�����һ���Դ�ӡ������
    // �������������߳���Ҫһ���Դ�ӡ��Ϣ������֮�䲻�����
    std::string errorString;

    // ��ӡ�����ļ����кż�λ�ã��������Щ���ݵĻ�
    if (loc)
        errorString = StringPrintf("%s:%d:%d: ", loc->filename.c_str(),
                                   loc->line, loc->column);

    errorString += errorType;
    errorString += ": ";
    errorString += StringVaprintf(format, args);

    // ��ӡ������Ϣ�����ֻ��ӡһ�Σ�
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
