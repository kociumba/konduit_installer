#include "encoding_handling.hpp"

namespace encoding {

ZipReader::ZipReader() : owns_buffer(false), current_index(0), total_files(0) {
    mz_zip_zero_struct(&archive);
}

ZipReader::~ZipReader() {
    if (archive.m_zip_mode != MZ_ZIP_MODE_INVALID) {
        mz_zip_reader_end(&archive);
    }
}

ZipIterator::ZipIterator(ZipReader* r, uint32_t idx) : reader(r), index(idx) {}

ZipIterator& ZipIterator::operator++() {
    ++index;
    return *this;
}

bool ZipIterator::operator!=(const ZipIterator& other) const {
    return index != other.index || reader != other.reader;
}

std::optional<ZipFileInfo> ZipIterator::operator*() const {
    if (!reader || index >= reader->total_files) {
        return nullopt;
    }
    return zip_get_file_info(reader, index);
}

std::unique_ptr<ZipReader>
zip_init_from_buffer(const unsigned char* buffer, size_t size) {
    if (!buffer || size == 0) {
        error("Invalid buffer or size");
        return nullptr;
    }

    auto reader = std::make_unique<ZipReader>();
    if (!mz_zip_reader_init_mem(&reader->archive, buffer, size, 0)) {
        return nullptr;
    }
    reader->total_files = mz_zip_reader_get_num_files(&reader->archive);
    reader->owns_buffer = false;
    return reader;
}

std::unique_ptr<ZipReader> zip_init_from_file(std::string_view filepath) {
    std::ifstream file(filepath.data(), std::ios::binary | std::ios::ate);
    if (!file) {
        return nullptr;
    }
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    auto reader = std::make_unique<ZipReader>();
    reader->file_buffer.resize(size);
    if (!file.read(reinterpret_cast<char*>(reader->file_buffer.data()), size)) {
        return nullptr;
    }
    if (!mz_zip_reader_init_mem(
            &reader->archive,
            reader->file_buffer.data(),
            reader->file_buffer.size(),
            0
        )) {
        return nullptr;
    }
    reader->total_files = mz_zip_reader_get_num_files(&reader->archive);
    reader->owns_buffer = true;
    return reader;
}

std::optional<ZipFileInfo>
zip_get_file_info(ZipReader* reader, uint32_t index) {
    if (!reader || index >= reader->total_files)
        return nullopt;
    mz_zip_archive_file_stat file_stat;
    if (!mz_zip_reader_file_stat(&reader->archive, index, &file_stat)) {
        return nullopt;
    }
    ZipFileInfo info;
    info.filename = file_stat.m_filename;
    info.uncompressed_size = file_stat.m_uncomp_size;
    info.compressed_size = file_stat.m_comp_size;
    info.crc32 = file_stat.m_crc32;
    info.is_directory =
        mz_zip_reader_is_file_a_directory(&reader->archive, index);
    info.method = file_stat.m_method;
    info.mz_stat = file_stat;
    return info;
}

std::optional<ZipFileInfo> zip_current_file_info(ZipReader* reader) {
    return zip_get_file_info(reader, reader->current_index);
}

bool zip_next_file(ZipReader* reader) {
    if (!reader || reader->current_index >= reader->total_files - 1)
        return false;
    reader->current_index++;
    return true;
}

void zip_reset(ZipReader* reader) {
    if (reader)
        reader->current_index = 0;
}

bool zip_has_more_files(ZipReader* reader) {
    return reader && reader->current_index < reader->total_files;
}

uint32_t zip_get_file_count(ZipReader* reader) {
    return reader ? reader->total_files : 0;
}

std::optional<std::vector<uint8_t>> zip_extract_current_file(
    ZipReader* reader
) {
    if (!reader || reader->current_index >= reader->total_files)
        return std::nullopt;
    size_t uncomp_size;
    void* data = mz_zip_reader_extract_to_heap(
        &reader->archive, reader->current_index, &uncomp_size, 0
    );
    if (!data)
        return std::nullopt;
    std::vector<uint8_t> result(
        static_cast<uint8_t*>(data), static_cast<uint8_t*>(data) + uncomp_size
    );
    mz_free(data);
    return result;
}

std::optional<std::vector<uint8_t>>
zip_extract_file_by_index(ZipReader* reader, uint32_t index) {
    if (!reader || index >= reader->total_files)
        return std::nullopt;
    size_t uncomp_size;
    void* data =
        mz_zip_reader_extract_to_heap(&reader->archive, index, &uncomp_size, 0);
    if (!data)
        return std::nullopt;
    std::vector<uint8_t> result(
        static_cast<uint8_t*>(data), static_cast<uint8_t*>(data) + uncomp_size
    );
    mz_free(data);
    return result;
}

std::optional<std::vector<uint8_t>>
zip_extract_file_by_name(ZipReader* reader, std::string_view filename) {
    if (!reader)
        return std::nullopt;
    size_t uncomp_size;
    void* data = mz_zip_reader_extract_file_to_heap(
        &reader->archive, filename.data(), &uncomp_size, 0
    );
    if (!data)
        return std::nullopt;
    std::vector<uint8_t> result(
        static_cast<uint8_t*>(data), static_cast<uint8_t*>(data) + uncomp_size
    );
    mz_free(data);
    return result;
}

std::optional<uint32_t>
zip_find_file_index(ZipReader* reader, std::string_view filename) {
    if (!reader)
        return nullopt;
    int idx = mz_zip_reader_locate_file(
        &reader->archive, filename.data(), nullptr, 0
    );
    if (idx < 0)
        return nullopt;
    return static_cast<uint32_t>(idx);
}

ZipIterator begin(ZipReader* reader) {
    return {reader, 0};
}

ZipIterator end(ZipReader* reader) {
    return {reader, reader ? reader->total_files : 0};
}

std::optional<LoadedData>
load_resource_from_memory(const unsigned char* buffer, size_t buffer_size) {
    auto reader = zip_init_from_buffer(buffer, buffer_size);
    if (!reader) {
        error("Failed to initialize ZIP archive from memory");
        return std::nullopt;
    }

    LoadedData result;
    ZipReader* zip = reader.get();

    for (uint32_t i = 0; i < zip_get_file_count(zip); ++i) {
        auto info_opt = zip_get_file_info(zip, i);
        if (!info_opt)
            continue;

        const auto& info = *info_opt;
        info(
            std::format(
                "File entry: {} | Compressed Size: {} | Uncompressed Size: {}",
                info.filename,
                info.compressed_size,
                info.uncompressed_size
            )
                .c_str()
        );

        if (info.uncompressed_size > 0 && !info.is_directory) {
            auto data_opt = zip_extract_file_by_index(zip, i);
            if (!data_opt || data_opt->empty()) {
                error(
                    std::format("Failed to extract data for {}", info.filename)
                        .c_str()
                );
                continue;
            }

            result.data[info.filename] = std::move(*data_opt);
        }
    }

    if (result.data.empty()) {
        error("No valid files found in ZIP archive");
        return std::nullopt;
    }

    return result;
}

std::optional<LoadedData> load_resource(const std::string& path) {
    auto reader = zip_init_from_file(path);
    if (!reader) {
        error(
            "Failed to open ZIP file directly, trying to load file into memory"
        );

        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            error("Failed to open file");
            return std::nullopt;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(size);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            error("Failed to read file");
            return std::nullopt;
        }

        return load_resource_from_memory(buffer.data(), buffer.size());
    }

    LoadedData result;
    ZipReader* zip = reader.get();

    for (uint32_t i = 0; i < zip_get_file_count(zip); ++i) {
        auto info_opt = zip_get_file_info(zip, i);
        if (!info_opt)
            continue;

        const auto& info = *info_opt;
        info(
            std::format(
                "File entry: {} | Compressed Size: {} | Uncompressed Size: {}",
                info.filename,
                info.compressed_size,
                info.uncompressed_size
            )
                .c_str()
        );

        if (info.uncompressed_size > 0 && !info.is_directory) {
            auto data_opt = zip_extract_file_by_index(zip, i);
            if (!data_opt || data_opt->empty()) {
                error(
                    std::format("Failed to extract data for {}", info.filename)
                        .c_str()
                );
                continue;
            }

            result.data[info.filename] = std::move(*data_opt);
        }
    }

    if (result.data.empty()) {
        error("No valid files found in ZIP archive");
        return std::nullopt;
    }

    return result;
}

std::string write_to_temp_file(
    const unsigned char* data,
    size_t data_size,
    const std::string& ext
) {
    namespace fs = std::filesystem;

    std::string temp_name =
        "bundle_" + std::to_string(std::random_device{}()) + ext;
    fs::path temp_path = fs::temp_directory_path() / temp_name;

    std::ofstream out(temp_path, std::ios::binary);
    if (!out) {
        return "";
    }

    out.write(reinterpret_cast<const char*>(data), data_size);
    out.close();

    temp_files.push_back(temp_path.string());

    return temp_path.string();
}

bool remove_temp_file(const std::string& path) {
    if (path.empty())
        return true;
    std::error_code ec;
    return std::filesystem::remove(path, ec);
}

bool remove_all_temp_files() {
    int errors = 0;
    for (const auto& file : temp_files) {
        if (!remove_temp_file(file)) {
            errors++;
            error(std::format("error removing: {}", file).c_str());
        }
        info(std::format("{} removed correctly", file).c_str());
    }
    if (errors > 0) {
        return false;
    }

    return true;
}

}  // namespace encoding