#include "encoding_handling.hpp"

namespace encoding {

std::optional<LoadedData>
load_resource_from_memory(const unsigned char* buffer, size_t buffer_size) {
    LoadedData res;
    struct archive* a = nullptr;
    struct archive_entry* entry;
    int r;

    a = archive_read_new();
    if (!a) {
        error("Failed to initialize archive");
        return std::nullopt;
    }
    archive_read_support_filter_all(a);
    archive_read_support_format_tar(a);

    r = archive_read_open_memory(a, buffer, buffer_size);
    if (r != ARCHIVE_OK) {
        error(archive_error_string(a));
        archive_read_free(a);
        return std::nullopt;
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char* entry_path = archive_entry_pathname(entry);
        if (!entry_path) {
            continue;
        }

        if (archive_entry_filetype(entry) != AE_IFREG) {
            continue;
        }

        info(
            std::format(
                "File entry: {} | Size: {}",
                entry_path,
                archive_entry_size(entry)
            )
                .c_str()
        );

        std::vector<uint8_t> file_data;
        int64_t entry_size = archive_entry_size(entry);

        if (entry_size > 0) {
            file_data.resize(entry_size);
            ssize_t size_read =
                archive_read_data(a, file_data.data(), entry_size);

            if (size_read < 0) {
                error(
                    std::format(
                        "Failed to read data for {}: {}",
                        entry_path,
                        archive_error_string(a)
                    )
                        .c_str()
                );
                continue;
            }

            if (size_read != entry_size) {
                file_data.resize(size_read);
            }

            res.data[entry_path] = std::move(file_data);
        }
    }
    archive_read_free(a);

    if (res.data.empty()) {
        error("No valid files found in archive");
        return std::nullopt;
    }

    return res;
}

std::optional<LoadedData> load_resource(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        error("Failed to open file");
        return std::nullopt;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        error("Failed to read file");
        return std::nullopt;
    }

    return load_resource_from_memory(buffer.data(), buffer.size());
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