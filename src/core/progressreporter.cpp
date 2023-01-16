#include <core/ProgressReporter.h>
#include <core/Parallel.h>
#include <core/Stats.h>
#ifdef PORTE_IS_WINDOWS
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#endif  // !PORTE_IS_WINDOWS

namespace prote {

static int TerminalWidth();

ProgressReporter::ProgressReporter(int64_t totalWork, const std::string &title)
    : totalWork(std::max((int64_t)1, totalWork)),
      title(title),
      startTime(std::chrono::system_clock::now()) {
    workDone = 0;
    exitThread = false;

        SuspendProfiler();
        std::shared_ptr<Barrier> barrier = std::make_shared<Barrier>(2);
        updateThread = std::thread([this, barrier]() {
            ProfilerWorkerThreadInit();
            ProfilerState = 0;
            barrier->Wait();
            PrintBar();
        });
        // 等待线程结束
        barrier->Wait();
        ResumeProfiler();
}

ProgressReporter::~ProgressReporter() {
	workDone = totalWork;
	exitThread = true;
	updateThread.join();
	printf("\n");
}

void ProgressReporter::PrintBar() {
    int barLength = TerminalWidth() - 28;
    int totalPlusses = std::max(2, barLength - (int)title.size());
    int plussesPrinted = 0;

    // 初始化进度字符串
    const int bufLen = title.size() + totalPlusses + 64;
    std::unique_ptr<char[]> buf(new char[bufLen]);
    snprintf(buf.get(), bufLen, "\r%s: [", title.c_str());
    char *curSpace = buf.get() + strlen(buf.get());
    char *s = curSpace;
    for (int i = 0; i < totalPlusses; ++i) *s++ = ' ';
    *s++ = ']';
    *s++ = ' ';
    *s++ = '\0';
    fputs(buf.get(), stdout);
    fflush(stdout);

    std::chrono::milliseconds sleepDuration(250);
    int iterCount = 0;
    while (!exitThread) {
        std::this_thread::sleep_for(sleepDuration);

        // 定期增加 sleepDuration 以减少更新开销。
        ++iterCount;
        if (iterCount == 10)
            sleepDuration *= 2;
        else if (iterCount == 70)
            sleepDuration *= 2;
        else if (iterCount == 520)
            sleepDuration *= 5;

        Float percentDone = Float(workDone) / Float(totalWork);
        int plussesNeeded = std::round(totalPlusses * percentDone);
        while (plussesPrinted < plussesNeeded) {
            *curSpace++ = '+';
            ++plussesPrinted;
        }
        fputs(buf.get(), stdout);

        // Update elapsed time and estimated time to completion
        Float seconds = ElapsedMS() / 1000.f;
        Float estRemaining = seconds / percentDone - seconds;
        if (percentDone == 1.f)
            printf(" (%.1fs)       ", seconds);
        else if (!std::isinf(estRemaining))
            printf(" (%.1fs|%.1fs)  ", seconds,
                   std::max((Float)0., estRemaining));
        else
            printf(" (%.1fs|?s)  ", seconds);
        fflush(stdout);
    }
}

void ProgressReporter::Done() {
    workDone = totalWork;
}

static int TerminalWidth() {
#ifdef PORTE_IS_WINDOWS
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE || !h) {
        fprintf(stderr, "GetStdHandle() call failed");
        return 80;
    }
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo = {0};
    GetConsoleScreenBufferInfo(h, &bufferInfo);
    return bufferInfo.dwSize.X;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) < 0) {
        // ENOTTY is fine and expected, e.g. if output is being piped to a file.
        if (errno != ENOTTY) {
            static bool warned = false;
            if (!warned) {
                warned = true;
                fprintf(stderr, "Error in ioctl() in TerminalWidth(): %d\n",
                        errno);
            }
        }
        return 80;
    }
    return w.ws_col;
#endif  // PORTE_IS_WINDOWS
}

}  // namespace porte
