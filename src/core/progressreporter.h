#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PORTE_CORE_PROGRESSREPORTER_H
#define PORTE_CORE_PROGRESSREPORTER_H

#include <core/porte.h>
#include <atomic>
#include <chrono>
#include <thread>

namespace porte {

class ProgressReporter {
public:
    ProgressReporter(int64_t totalWork, const std::string &title);
    ~ProgressReporter();
    void Update(int64_t num = 1) {
        if (num == 0 || PorteOptions.quiet) return;
        workDone += num;
    }
    Float ElapsedMS() const {
        std::chrono::system_clock::time_point now =
            std::chrono::system_clock::now();
        int64_t elapsedMS =
            std::chrono::duration_cast<std::chrono::milliseconds>(now -
                                                                  startTime)
                .count();
        return (Float)elapsedMS;
    }
    void Done();

  private:
    void PrintBar();

    const int64_t totalWork;
    const std::string title;
    const std::chrono::system_clock::time_point startTime;
    std::atomic<int64_t> workDone;
    std::atomic<bool> exitThread;
    std::thread updateThread;
};

}  // namespace porte

#endif  // PORTE_CORE_PROGRESSREPORTER_H
