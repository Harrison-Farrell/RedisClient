// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlog/sinks/rotating_file_sink.h"

#include <cerrno>
#include <mutex>
#include <sstream>
#include <string>
#include <tuple>

#include "spdlog/common.h"
#include "spdlog/details/file_helper.h"
#include "spdlog/details/os.h"

namespace spdlog {
namespace sinks {

template <typename Mutex>
rotating_file_sink<Mutex>::rotating_file_sink(filename_t base_filename,
                                              std::size_t max_size,
                                              std::size_t max_files,
                                              bool rotate_on_open,
                                              const file_event_handlers &event_handlers)
    : base_filename_(std::move(base_filename)),
      max_size_(max_size),
      max_files_(max_files),
      file_helper_{event_handlers} {
    if (max_size == 0) {
        throw_spdlog_ex("rotating sink constructor: max_size arg cannot be zero");
    }

    if (max_files > 200000) {
        throw_spdlog_ex("rotating sink constructor: max_files arg cannot exceed 200000");
    }
    file_helper_.open(calc_filename(base_filename_, 0));
    current_size_ = file_helper_.size();  // expensive. called only once
    if (rotate_on_open && current_size_ > 0) {
        rotate_();
        current_size_ = 0;
    }
}

// calc filename according to index and file extension if exists.
// e.g. calc_filename("logs/mylog.txt, 3) => "logs/mylog.3.txt".
template <typename Mutex>
filename_t rotating_file_sink<Mutex>::calc_filename(const filename_t &filename, std::size_t index) {
    if (index == 0U) {
        return filename;
    }

    filename_t basename;
    filename_t ext;
    std::tie(basename, ext) = details::os::split_by_extension(filename);
    std::basic_ostringstream<filename_t::value_type> oss;
    oss << basename.native() << '.' << index << ext.native();
    return oss.str();
}

template <typename Mutex>
filename_t rotating_file_sink<Mutex>::filename() {
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    return file_helper_.filename();
}

template <typename Mutex>
void rotating_file_sink<Mutex>::rotate_now() {
    rotate_();
}

template <typename Mutex>
void rotating_file_sink<Mutex>::sink_it_(const details::log_msg &msg) {
    memory_buf_t formatted;
    base_sink<Mutex>::formatter_->format(msg, formatted);
    auto new_size = current_size_ + formatted.size();

    // rotate if the new estimated file size exceeds max size.
    // rotate only if the real size > 0 to better deal with full disk (see issue #2261).
    // we only check the real size when new_size > max_size_ because it is relatively expensive.
    if (new_size > max_size_) {
        file_helper_.flush();
        if (file_helper_.size() > 0) {
            rotate_();
            new_size = formatted.size();
        }
    }
    file_helper_.write(formatted);
    current_size_ = new_size;
}

template <typename Mutex>
void rotating_file_sink<Mutex>::flush_() {
    file_helper_.flush();
}

// Rotate files:
// log.txt -> log.1.txt
// log.1.txt -> log.2.txt
// log.2.txt -> log.3.txt
// log.3.txt -> delete
template <typename Mutex>
void rotating_file_sink<Mutex>::rotate_() {
    using details::os::filename_to_str;
    using details::os::path_exists;

    file_helper_.close();
    for (auto i = max_files_; i > 0; --i) {
        filename_t src = calc_filename(base_filename_, i - 1);
        if (!path_exists(src)) {
            continue;
        }
        filename_t target = calc_filename(base_filename_, i);

        if (!rename_file_(src, target)) {
            // if failed try again after a small delay.
            // this is a workaround to a windows issue, where very high rotation
            // rates can cause the rename to fail with permission denied (because of
            // antivirus?).
            details::os::sleep_for_millis(100);
            if (!rename_file_(src, target)) {
                file_helper_.reopen(true);  // truncate the log file anyway to prevent it
                                            // to grow beyond its limit!
                current_size_ = 0;
                throw_spdlog_ex("rotating_file_sink: failed renaming " + filename_to_str(src) + " to " + filename_to_str(target),
                                errno);
            }
        }
    }
    file_helper_.reopen(true);
}

// delete the target if exists, and rename the src file  to target
// return true on success, false otherwise.
template <typename Mutex>
bool rotating_file_sink<Mutex>::rename_file_(const filename_t &src_filename, const filename_t &target_filename) noexcept {
    return details::os::rename(src_filename, target_filename);
}

}  // namespace sinks
}  // namespace spdlog

// template instantiations
#include "spdlog/details/null_mutex.h"
template class SPDLOG_API spdlog::sinks::rotating_file_sink<std::mutex>;
template class SPDLOG_API spdlog::sinks::rotating_file_sink<spdlog::details::null_mutex>;
