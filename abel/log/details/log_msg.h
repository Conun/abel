//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include <abel/log/common.h>
#include <abel/log/details/os.h>
#include <abel/thread/this_thread.h>
#include <string>
#include <utility>

namespace spdlog {
namespace details {
struct log_msg {
    log_msg () = default;
    log_msg (const std::string *loggers_name, level::level_enum lvl)
        : logger_name(loggers_name), level(lvl) {
#ifndef SPDLOG_NO_DATETIME
        time = abel::now();
#endif

#ifndef SPDLOG_NO_THREAD_ID
        thread_id = abel::thread_id();
#endif
    }

    log_msg (const log_msg &other) = delete;
    log_msg (log_msg &&other) = delete;
    log_msg &operator = (log_msg &&other) = delete;

    const std::string *logger_name {nullptr};
    level::level_enum level;
    abel::abel_time time;
    size_t thread_id;
    fmt::memory_buffer raw;
    size_t msg_id {0};
    // info about wrapping the formatted text with color
    mutable size_t color_range_start {0};
    mutable size_t color_range_end {0};
};
} // namespace details
} // namespace spdlog
