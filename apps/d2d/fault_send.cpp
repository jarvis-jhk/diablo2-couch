// SPDX-License-Identifier: GPL-3.0-or-later
// Diablo 2 Couch: hands a fault report line (couch/fault_report.hpp) to a
// detached curl, so the game never waits for the network. False when there
// is no curl to hand it to; the caller then leaves it for the launcher.
#include "fault_send.hpp"

#include <fault_report.hpp>

#include <cstdlib>
#include <string>
#include <string_view>

#if defined(_WIN32)
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#  include <filesystem>
#else
#  include <sys/wait.h>
#  include <unistd.h>
#endif

namespace d2d::client {

#if defined(_WIN32)
bool send_fault_report(const std::string& line) {
    // Windows 10 1803 and later ship curl.exe in System32.
    wchar_t system[MAX_PATH];
    const UINT length = GetSystemDirectoryW(system, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) return false;
    const std::filesystem::path curl = std::filesystem::path(system) / L"curl.exe";
    std::error_code error;
    if (!std::filesystem::exists(curl, error)) return false;
    // The line is ours ("Diablo 2 Couch v0.0.2 Windows crash"): no quotes in it.
    std::wstring command = L"\"" + curl.wstring() + L"\" -fsS --max-time 8 -o NUL -d \"";
    for (const char c : line) command += wchar_t(static_cast<unsigned char>(c));
    command += L"\" ";
    for (const char c : d2d::couch::fault::kTopicUrl) command += wchar_t(c);
    STARTUPINFOW start{};
    PROCESS_INFORMATION process{};
    start.cb = sizeof(start);
    if (!CreateProcessW(nullptr, command.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW | DETACHED_PROCESS, nullptr, nullptr, &start, &process))
        return false;
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return true;
}
#else
static std::string find_curl() {
    const char* path = std::getenv("PATH");
    std::string_view rest = path ? path : "/usr/bin:/bin";
    while (!rest.empty()) {
        const auto colon = rest.find(':');
        const auto dir = rest.substr(0, colon);
        rest = colon == std::string_view::npos ? std::string_view{} : rest.substr(colon + 1);
        if (dir.empty()) continue;
        std::string candidate = std::string(dir) + "/curl";
        if (access(candidate.c_str(), X_OK) == 0) return candidate;
    }
    return {};
}

bool send_fault_report(const std::string& line) {
    const std::string curl = find_curl();
    if (curl.empty()) return false;
    const std::string url(d2d::couch::fault::kTopicUrl);
    // Double fork: curl outlives a short wait here without becoming a zombie.
    const pid_t child = fork();
    if (child < 0) return false;
    if (child == 0) {
        if (fork() == 0) {
            execl(curl.c_str(), "curl", "-fsS", "--max-time", "8", "-o", "/dev/null", "-d", line.c_str(), url.c_str(), static_cast<char*>(nullptr));
        }
        _exit(0);
    }
    waitpid(child, nullptr, 0);
    return true;
}
#endif

}  // namespace d2d::client
