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

#include <miniz.h>

using std::nullopt;
using std::optional;

struct ArchiveEntry {
    std::string path;
    uint64_t size;
    uint64_t uncompressed_size;
    uint32_t file_index;

    std::vector<uint8_t> extract_data(mz_zip_archive* zip) const {
        if (uncompressed_size == 0)
            return {};

        std::vector<uint8_t> data(uncompressed_size);

        if (!mz_zip_reader_extract_to_mem(
                zip, file_index, data.data(), uncompressed_size, 0
            )) {
            return {};
        }

        return data;
    }
};

// prototype for streaming interface for the archives
class ArchiveIterator {
   private:
    mz_zip_archive zip_;
    uint32_t current_index_;
    uint32_t total_files_;
    bool valid_;

    void advance_to_next_file() {
        while (current_index_ < total_files_) {
            mz_zip_archive_file_stat file_stat;
            if (mz_zip_reader_file_stat(&zip_, current_index_, &file_stat)) {
                if (!mz_zip_reader_is_file_a_directory(&zip_, current_index_)) {
                    return;
                }
            }
            current_index_++;
        }
        valid_ = false;
    }

   public:
    static optional<ArchiveIterator>
    from_memory(const unsigned char* buffer, size_t size) {
        ArchiveIterator iter;

        memset(&iter.zip_, 0, sizeof(iter.zip_));

        // Use the buffer directly - no need to copy since caller manages
        // lifetime
        if (!mz_zip_reader_init_mem(&iter.zip_, buffer, size, 0)) {
            return nullopt;
        }

        iter.total_files_ = mz_zip_reader_get_num_files(&iter.zip_);
        iter.current_index_ = 0;
        iter.valid_ = iter.total_files_ > 0;

        if (iter.valid_) {
            iter.advance_to_next_file();
        }

        return iter.valid_ ? optional<ArchiveIterator>{std::move(iter)}
                           : nullopt;
    }

    static optional<ArchiveIterator> from_file(const std::string& path) {
        ArchiveIterator iter;

        memset(&iter.zip_, 0, sizeof(iter.zip_));

        if (!mz_zip_reader_init_file(&iter.zip_, path.c_str(), 0)) {
            return nullopt;
        }

        iter.total_files_ = mz_zip_reader_get_num_files(&iter.zip_);
        iter.current_index_ = 0;
        iter.valid_ = iter.total_files_ > 0;

        if (iter.valid_) {
            iter.advance_to_next_file();
        }

        return iter.valid_ ? optional<ArchiveIterator>{std::move(iter)}
                           : nullopt;
    }

   private:
    ArchiveIterator() : current_index_(0), total_files_(0), valid_(false) {
        memset(&zip_, 0, sizeof(zip_));
    }

   public:
    ~ArchiveIterator() {
        if (zip_.m_pState) {
            mz_zip_reader_end(&zip_);
        }
    }

    ArchiveIterator(const ArchiveIterator&) = delete;
    ArchiveIterator& operator=(const ArchiveIterator&) = delete;

    ArchiveIterator(ArchiveIterator&& other) noexcept
        : zip_(other.zip_),
          current_index_(other.current_index_),
          total_files_(other.total_files_),
          valid_(other.valid_) {
        other.zip_.m_pState = nullptr;
        other.valid_ = false;
    }

    ArchiveIterator& operator=(ArchiveIterator&& other) noexcept {
        if (this != &other) {
            if (zip_.m_pState) {
                mz_zip_reader_end(&zip_);
            }
            zip_ = other.zip_;
            current_index_ = other.current_index_;
            total_files_ = other.total_files_;
            valid_ = other.valid_;
            other.zip_.m_pState = nullptr;
            other.valid_ = false;
        }
        return *this;
    }
    [[nodiscard]] bool has_next() const { return valid_; }

    [[nodiscard]] ArchiveEntry current() const {
        if (!valid_)
            return {"", 0, 0, 0};

        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(
                const_cast<mz_zip_archive*>(&zip_), current_index_, &file_stat
            )) {
            return {"", 0, 0, 0};
        }

        return {
            file_stat.m_filename,
            file_stat.m_comp_size,
            file_stat.m_uncomp_size,
            current_index_
        };
    }

    void next() {
        if (!valid_)
            return;

        current_index_++;
        advance_to_next_file();
    }

    std::vector<uint8_t> extract_current() {
        if (!valid_)
            return {};
        return current().extract_data(&zip_);
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