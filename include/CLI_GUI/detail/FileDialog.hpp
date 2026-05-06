#pragma once

#ifdef CLI_GUI_HAS_GUI

#include <cstddef>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#endif

namespace CLI_GUI {
namespace detail {

/// Open a native file-open dialog. Writes selected path to buf, returns true.
inline bool open_file_dialog(char* buf, size_t size, const char* title) {
#ifdef _WIN32
    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFile = buf;
    ofn.nMaxFile = static_cast<DWORD>(size);
    ofn.lpstrTitle = title;
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    return GetOpenFileNameA(&ofn) != FALSE;
#else
    (void)buf; (void)size; (void)title;
    return false;
#endif
}

/// Open a native file-save dialog. Writes path to buf, returns true.
inline bool save_file_dialog(char* buf, size_t size, const char* title) {
#ifdef _WIN32
    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFile = buf;
    ofn.nMaxFile = static_cast<DWORD>(size);
    ofn.lpstrTitle = title;
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
    return GetSaveFileNameA(&ofn) != FALSE;
#else
    (void)buf; (void)size; (void)title;
    return false;
#endif
}

/// Open a native folder-picker dialog. Writes path to buf, returns true.
inline bool dir_picker_dialog(char* buf, size_t size, const char* title) {
#ifdef _WIN32
    BROWSEINFOA bi = {};
    bi.hwndOwner = GetActiveWindow();
    bi.lpszTitle = title;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (pidl) {
        SHGetPathFromIDListA(pidl, buf);
        CoTaskMemFree(pidl);
        return true;
    }
    return false;
#else
    (void)buf; (void)size; (void)title;
    return false;
#endif
}

} // namespace detail
} // namespace CLI_GUI

#endif // CLI_GUI_HAS_GUI
