// SPDX-License-Identifier: GPL-3.0-or-later
// Diablo 2 Couch: anonymous fault reports (ROADMAP M2). A released build
// marks each session as running in the user dir and clears the mark when it
// ends; a mark still there at the next start is a session that crashed (or
// was killed), and that start reports it: one line, "Diablo 2 Couch <version>
// <platform> <category>", posted to a public ntfy topic that the
// fault-reports workflow turns into GitHub issues. Nothing else is sent: no
// names, paths, logs or saves. Reporting never waits for the network: d2d
// hands the line to a detached curl, or leaves it for the launcher (Qt
// Network) when there is no curl (the Flatpak runtime). Off for development
// builds, with D2COUCH_NO_REPORT=1, or with `fault_reports = 0` in d2d.cfg.
#pragma once

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

namespace d2d::couch::fault {

namespace fs = std::filesystem;

// Public on purpose: anyone may post, the workflow reads it. The maintainer's
// own endpoint is only in the repository's secrets.
inline constexpr std::string_view kTopicUrl = "https://ntfy.sh/diablo2-couch-faults-ae4dac602ec557e8";

inline fs::path running_mark(const fs::path& user_dir) { return user_dir / "d2d.running"; }
inline fs::path pending_file(const fs::path& user_dir) { return user_dir / "fault-report.txt"; }

// Only a tagged release reports: git describe gives "v0.0.2" for the tag
// itself and "v0.0.2-g1a2b3c4" (or a bare hash) for anything else.
inline bool released(std::string_view version) {
    return version.size() > 1 && version.front() == 'v' && version.find("-g") == std::string_view::npos
        && version.find("dirty") == std::string_view::npos && version.find("dev") == std::string_view::npos;
}

inline bool enabled(std::string_view version, std::string_view cfg_value) {
    const char* opt_out = std::getenv("D2COUCH_NO_REPORT");
    if (opt_out && std::string_view(opt_out) == "1") return false;
    return released(version) && cfg_value != "0";
}

// The platform as a word or two: what a fault depends on, nothing personal.
inline std::string platform() {
#if defined(_WIN32)
    std::string name = "Windows";
#elif defined(__APPLE__)
    std::string name = "macOS";
#else
    std::string name = "Linux";
#endif
    if (std::getenv("FLATPAK_ID")) name += " Flatpak";
    if (const char* deck = std::getenv("SteamDeck"); deck && std::string_view(deck) == "1") name += " Deck";
    return name;
}

inline std::string message(std::string_view version, std::string_view where, std::string_view category) {
    return "Diablo 2 Couch " + std::string(version) + " " + std::string(where) + " " + std::string(category);
}

// The start of a session: the version of a previous one that never ended,
// if any, and this one marked as running.
inline std::optional<std::string> begin(const fs::path& user_dir, std::string_view version) {
    std::optional<std::string> crashed;
    std::error_code error;
    if (fs::exists(running_mark(user_dir), error)) {
        std::ifstream input(running_mark(user_dir));
        std::string line;
        std::getline(input, line);
        crashed = line.empty() ? std::string(version) : line;
    }
    std::ofstream(running_mark(user_dir), std::ios::trunc) << version << '\n';
    return crashed;
}

// The session ended on its own: no report next time.
inline void end(const fs::path& user_dir) {
    std::error_code error;
    fs::remove(running_mark(user_dir), error);
}

// A line nobody could send yet (one at a time: a crash loop is one report).
inline void leave_pending(const fs::path& user_dir, std::string_view line) {
    std::ofstream(pending_file(user_dir), std::ios::trunc) << line << '\n';
}

inline std::optional<std::string> take_pending(const fs::path& user_dir) {
    std::error_code error;
    if (!fs::exists(pending_file(user_dir), error)) return std::nullopt;
    std::string line;
    {
        std::ifstream input(pending_file(user_dir));
        std::getline(input, line);
    }
    fs::remove(pending_file(user_dir), error);
    if (line.empty()) return std::nullopt;
    return line;
}

}  // namespace d2d::couch::fault
