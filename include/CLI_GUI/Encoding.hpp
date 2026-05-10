#pragma once

#include <string>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace CLI_GUI {

/// Convert a UTF-8 string to the system ANSI code page (e.g. GBK on Chinese Windows).
/// On non-Windows platforms, returns the input unchanged.
/// Non-convertible characters are replaced with '?'.
inline std::string utf8_to_ansi(const std::string& utf8) {
#ifdef _WIN32
    if (utf8.empty()) return utf8;
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    if (wlen <= 0) return utf8;
    std::vector<wchar_t> wbuf(wlen);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wbuf.data(), wlen);
    int alen = WideCharToMultiByte(CP_ACP, 0, wbuf.data(), -1, NULL, 0, NULL, NULL);
    if (alen <= 0) return utf8;
    std::vector<char> abuf(alen);
    WideCharToMultiByte(CP_ACP, 0, wbuf.data(), -1, abuf.data(), alen, NULL, NULL);
    return std::string(abuf.data());
#else
    return utf8;
#endif
}

/// Convert a system ANSI code page string (e.g. GBK) to UTF-8.
/// On non-Windows platforms, returns the input unchanged.
inline std::string ansi_to_utf8(const std::string& ansi) {
#ifdef _WIN32
    if (ansi.empty()) return ansi;
    int wlen = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, NULL, 0);
    if (wlen <= 0) return ansi;
    std::vector<wchar_t> wbuf(wlen);
    MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, wbuf.data(), wlen);
    int ulen = WideCharToMultiByte(CP_UTF8, 0, wbuf.data(), -1, NULL, 0, NULL, NULL);
    if (ulen <= 0) return ansi;
    std::vector<char> ubuf(ulen);
    WideCharToMultiByte(CP_UTF8, 0, wbuf.data(), -1, ubuf.data(), ulen, NULL, NULL);
    return std::string(ubuf.data());
#else
    return ansi;
#endif
}

} // namespace CLI_GUI
