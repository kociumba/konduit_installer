#ifndef KONDUIT_INSTALLER_ENCODING_HANDLING_HPP
#define KONDUIT_INSTALLER_ENCODING_HANDLING_HPP

// #include <raylib.h>
// #include <raylib/rres-raylib.h>
// #include <raylib/rres.h>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <random>
#include <string>
#include "../main.hpp"

namespace encoding {

#include <archive.h>
#include <archive_entry.h>
// prevent windows.h from conflicting with raylib
#undef TRANSPARENT
#undef ERROR

using std::nullopt;
using std::optional;

struct ArchiveEntry {
    std::string path;
    int64_t size;

    std::vector<uint8_t> extract_data(struct archive* a) const {
        if (size <= 0)
            return {};

        std::vector<uint8_t> data(size);
        ssize_t read = archive_read_data(a, data.data(), size);

        if (read < 0)
            return {};
        if (read != size)
            data.resize(read);

        return data;
    }
};

// prototype for streaming interface for the archives
class ArchiveIterator {
   private:
    struct archive* archive_;
    struct archive_entry* current_entry_;
    bool valid_;

    void advance_to_next_file() {
        while (archive_read_next_header(archive_, &current_entry_) ==
               ARCHIVE_OK) {
            if (archive_entry_filetype(current_entry_) == AE_IFREG) {
                return;
            }
        }
        valid_ = false;
    }

   public:
    static optional<ArchiveIterator>
    from_memory(const unsigned char* buffer, size_t size) {
        struct archive* a = archive_read_new();
        if (!a)
            return nullopt;

        archive_read_support_filter_all(a);
        archive_read_support_format_tar(a);

        if (archive_read_open_memory(a, buffer, size) != ARCHIVE_OK) {
            archive_read_free(a);
            return nullopt;
        }

        ArchiveIterator iter(a);
        return iter.valid_ ? optional<ArchiveIterator>{std::move(iter)}
                           : nullopt;
    }

    static optional<ArchiveIterator> from_file(const std::string& path) {
        struct archive* a = archive_read_new();
        if (!a)
            return nullopt;

        archive_read_support_filter_all(a);
        archive_read_support_format_tar(a);

        if (archive_read_open_filename(a, path.c_str(), 10240) != ARCHIVE_OK) {
            archive_read_free(a);
            return nullopt;
        }

        ArchiveIterator iter(a);
        return iter.valid_ ? optional<ArchiveIterator>{std::move(iter)}
                           : nullopt;
    }

   private:
    explicit ArchiveIterator(struct archive* a)
        : archive_(a), current_entry_(nullptr), valid_(true) {
        advance_to_next_file();
    }

   public:
    ~ArchiveIterator() {
        if (archive_) {
            archive_read_free(archive_);
        }
    }

    ArchiveIterator(const ArchiveIterator&) = delete;
    ArchiveIterator& operator=(const ArchiveIterator&) = delete;

    ArchiveIterator(ArchiveIterator&& other) noexcept
        : archive_(other.archive_),
          current_entry_(other.current_entry_),
          valid_(other.valid_) {
        other.archive_ = nullptr;
        other.valid_ = false;
    }

    ArchiveIterator& operator=(ArchiveIterator&& other) noexcept {
        if (this != &other) {
            if (archive_)
                archive_read_free(archive_);
            archive_ = other.archive_;
            current_entry_ = other.current_entry_;
            valid_ = other.valid_;
            other.archive_ = nullptr;
            other.valid_ = false;
        }
        return *this;
    }

    [[nodiscard]] bool has_next() const { return valid_; }

    [[nodiscard]] ArchiveEntry current() const {
        if (!valid_)
            return {"", 0};

        const char* path = archive_entry_pathname(current_entry_);
        int64_t size = archive_entry_size(current_entry_);

        return {path ? path : "", size};
    }

    void next() {
        if (!valid_)
            return;

        archive_read_data_skip(archive_);
        advance_to_next_file();
    }

    std::vector<uint8_t> extract_current() {
        if (!valid_)
            return {};
        return current().extract_data(archive_);
    }
};

struct LoadedData {
    std::map<std::string, std::vector<uint8_t>> data;
};

std::optional<LoadedData>
load_resource_from_memory(const unsigned char* buffer, size_t buffer_size);

std::optional<LoadedData> load_resource(const std::string& path);

static std::vector<std::string> temp_files;

std::string write_to_temp_file(
    const unsigned char* data,
    size_t data_size,
    const std::string& ext
);
bool remove_temp_file(const std::string& path);
bool remove_all_temp_files();

}  // namespace encoding

#endif  // KONDUIT_INSTALLER_ENCODING_HANDLING_HPP