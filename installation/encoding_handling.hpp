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

struct ZipFileInfo {
    std::string filename;
    uint64_t uncompressed_size;
    uint64_t compressed_size;
    uint32_t crc32;
    bool is_directory;
    uint16_t method;
    mz_zip_archive_file_stat mz_stat;
};

struct ZipReader {
    mz_zip_archive archive;
    std::vector<uint8_t> file_buffer;
    bool owns_buffer;
    uint32_t current_index;
    uint32_t total_files;

    ZipReader();
    ~ZipReader();
    ZipReader(const ZipReader&) = delete;
    ZipReader& operator=(const ZipReader&) = delete;
};

struct ZipIterator {
    ZipReader* reader;
    uint32_t index;

    ZipIterator(ZipReader* r, uint32_t idx);
    ZipIterator& operator++();
    bool operator!=(const ZipIterator& other) const;
    std::optional<ZipFileInfo> operator*() const;
};

std::unique_ptr<ZipReader>
zip_init_from_buffer(const unsigned char* buffer, size_t size);
std::unique_ptr<ZipReader> zip_init_from_file(std::string_view filepath);

std::optional<ZipFileInfo> zip_get_file_info(ZipReader* reader, uint32_t index);
std::optional<ZipFileInfo> zip_current_file_info(ZipReader* reader);
bool zip_next_file(ZipReader* reader);
void zip_reset(ZipReader* reader);
bool zip_has_more_files(ZipReader* reader);
uint32_t zip_get_file_count(ZipReader* reader);

std::optional<std::vector<uint8_t>> zip_extract_current_file(ZipReader* reader);
std::optional<std::vector<uint8_t>>
zip_extract_file_by_index(ZipReader* reader, uint32_t index);
std::optional<std::vector<uint8_t>>
zip_extract_file_by_name(ZipReader* reader, std::string_view filename);
std::optional<uint32_t>
zip_find_file_index(ZipReader* reader, std::string_view filename);

ZipIterator begin(ZipReader* reader);
ZipIterator end(ZipReader* reader);

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