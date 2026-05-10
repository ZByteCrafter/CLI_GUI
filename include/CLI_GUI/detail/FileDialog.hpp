#pragma once

#ifdef CLI_GUI_HAS_GUI

#include <cstddef>
#include <cstdio>

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

/// Open a native file-open dialog. Writes selected path to buf as UTF-8, returns true.
inline bool open_file_dialog(char* buf, size_t size, const char* title) {
#ifdef _WIN32
    wchar_t wbuf[1024] = {};
    wchar_t wtitle[256] = {};
    MultiByteToWideChar(CP_UTF8, 0, title, -1, wtitle, 256);
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = wbuf;
    ofn.nMaxFile = 1024;
    ofn.lpstrTitle = wtitle;
    ofn.lpstrFilter = L"All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    if (GetOpenFileNameW(&ofn) != FALSE) {
        WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, buf, (int)size, NULL, NULL);
        return true;
    }
    return false;
#else
    (void)buf; (void)size; (void)title;
    std::fprintf(stderr, "[WARN] File open dialog not available on this platform.\n");
    return false;
#endif
}

/// Open a native file-save dialog. Writes path to buf as UTF-8, returns true.
inline bool save_file_dialog(char* buf, size_t size, const char* title) {
#ifdef _WIN32
    wchar_t wbuf[1024] = {};
    wchar_t wtitle[256] = {};
    MultiByteToWideChar(CP_UTF8, 0, title, -1, wtitle, 256);
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = wbuf;
    ofn.nMaxFile = 1024;
    ofn.lpstrTitle = wtitle;
    ofn.lpstrFilter = L"All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
    if (GetSaveFileNameW(&ofn) != FALSE) {
        WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, buf, (int)size, NULL, NULL);
        return true;
    }
    return false;
#else
    (void)buf; (void)size; (void)title;
    std::fprintf(stderr, "[WARN] File save dialog not available on this platform.\n");
    return false;
#endif
}

/// Open a native folder-picker dialog. Writes path to buf as UTF-8, returns true.
inline bool dir_picker_dialog(char* buf, size_t size, const char* title) {
#ifdef _WIN32
    wchar_t wtitle[256] = {};
    MultiByteToWideChar(CP_UTF8, 0, title, -1, wtitle, 256);
    BROWSEINFOW bi = {};
    bi.hwndOwner = nullptr;
    bi.lpszTitle = wtitle;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
    if (pidl) {
        wchar_t wbuf[1024] = {};
        SHGetPathFromIDListW(pidl, wbuf);
        CoTaskMemFree(pidl);
        WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, buf, (int)size, NULL, NULL);
        return true;
    }
    return false;
#else
    (void)buf; (void)size; (void)title;
    std::fprintf(stderr, "[WARN] Directory picker dialog not available on this platform.\n");
    return false;
#endif
}

} // namespace detail
} // namespace CLI_GUI

#endif // CLI_GUI_HAS_GUI
