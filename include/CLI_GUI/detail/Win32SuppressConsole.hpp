#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdio>
#endif

/// Call at the very start of main().
/// On Windows with /SUBSYSTEM:WINDOWS, attach to the parent console if
/// launched from a terminal. Does nothing on double-click (no parent).
inline void cli_gui_init_console() {
#if defined(_WIN32)
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        // Re-open stdout/stderr to the attached console
        FILE* dummy;
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        freopen_s(&dummy, "CONOUT$", "w", stderr);
    }
#endif
}
