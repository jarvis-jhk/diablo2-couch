// SPDX-License-Identifier: GPL-3.0-or-later
// Diablo 2 Couch: sending a fault report line (fault_send.cpp).
#pragma once

#include <string>

namespace d2d::client {

// Hands `line` to a detached curl; false when there is none.
bool send_fault_report(const std::string& line);

}  // namespace d2d::client
