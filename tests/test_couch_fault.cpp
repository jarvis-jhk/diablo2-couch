// SPDX-License-Identifier: GPL-3.0-or-later
// Couch fault reports (couch/fault_report.hpp): only a released build
// reports, a session that never ended is reported once at the next start,
// and a line nobody could send waits for the launcher.
#include <fault_report.hpp>

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <system_error>

namespace fs = std::filesystem;
using namespace d2d::couch::fault;

int main() {
    // Released builds only: a tag's own describe, never a dev stamp.
    assert(released("v0.0.2"));
    assert(!released("v0.0.2-g1a2b3c4"));
    assert(!released("v0.0.2-g1a2b3c4-dirty"));
    assert(!released("1a2b3c4"));
    assert(!released("0.1.0-dev"));
    assert(!released(""));
    assert(enabled("v0.0.2", ""));
    assert(!enabled("v0.0.2", "0"));          // fault_reports = 0 in d2d.cfg
    assert(message("v0.0.2", "Linux Flatpak", "crash") == "Diablo 2 Couch v0.0.2 Linux Flatpak crash");

    std::error_code error;
    const fs::path dir = fs::temp_directory_path() / ("d2d-couch-fault-test-" + std::to_string(std::rand()));
    fs::remove_all(dir, error);
    fs::create_directories(dir);

    // A first start has nothing to report; one that ended cleanly neither.
    assert(!begin(dir, "v0.0.2"));
    assert(fs::exists(running_mark(dir)));
    end(dir);
    assert(!fs::exists(running_mark(dir)));
    assert(!begin(dir, "v0.0.2"));

    // It never ended (a crash): the next start names the version that crashed.
    const auto crashed = begin(dir, "v0.0.3");
    assert(crashed && *crashed == "v0.0.2");
    end(dir);
    assert(!begin(dir, "v0.0.3"));
    end(dir);

    // A pending line is taken once.
    assert(!take_pending(dir));
    leave_pending(dir, "Diablo 2 Couch v0.0.2 Linux crash");
    leave_pending(dir, "Diablo 2 Couch v0.0.2 Linux crash");   // a loop is still one line
    const auto line = take_pending(dir);
    assert(line && *line == "Diablo 2 Couch v0.0.2 Linux crash");
    assert(!take_pending(dir));

    fs::remove_all(dir, error);
    return 0;
}
