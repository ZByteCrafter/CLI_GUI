#pragma once

#ifdef CLI_GUI_HAS_GUI

#include <streambuf>
#include <string>
#include <iostream>
#include <memory>
#include <functional>
#include <mutex>

namespace CLI_GUI {
namespace detail {

/// Thread-safe custom streambuf that redirects output to a callback.
class CaptureStreamBuf : public std::streambuf {
public:
    using Callback = std::function<void(const std::string&)>;
    explicit CaptureStreamBuf(Callback cb) : callback_(std::move(cb)) {}

protected:
    int overflow(int c) override {
        if (c != EOF) {
            std::lock_guard<std::mutex> lock(mtx_);
            buffer_ += static_cast<char>(c);
            if (c == '\n') flush_buffer_locked();
        }
        return c;
    }
    int sync() override {
        std::lock_guard<std::mutex> lock(mtx_);
        flush_buffer_locked();
        return 0;
    }

private:
    void flush_buffer_locked() {
        if (!buffer_.empty() && callback_) {
            if (buffer_.back() == '\n') buffer_.pop_back();
            if (!buffer_.empty() && buffer_.back() == '\r') buffer_.pop_back();
            if (!buffer_.empty()) callback_(buffer_);
            buffer_.clear();
        }
    }
    Callback callback_;
    std::string buffer_;
    std::mutex mtx_;
};

/// RAII guard that replaces cout's streambuf and restores on destruction.
class CoutRedirect {
public:
    explicit CoutRedirect(CaptureStreamBuf::Callback cb)
        : buf_(std::make_unique<CaptureStreamBuf>(std::move(cb))),
          old_buf_(std::cout.rdbuf(buf_.get())) {}
    ~CoutRedirect() { std::cout.rdbuf(old_buf_); }
private:
    std::unique_ptr<CaptureStreamBuf> buf_;
    std::streambuf* old_buf_;
};

} // namespace detail
} // namespace CLI_GUI

#endif
