#include "encoding_handling.hpp"

namespace encoding {

std::optional<LoadedData>
load_resource_from_memory(const unsigned char* buffer, size_t buffer_size) {
    auto iterator = ArchiveIterator::from_memory(buffer, buffer_size);
    if (!iterator) {
        error("Failed to initialize ZIP archive from memory");
        return std::nullopt;
    }

    LoadedData res;

    while (iterator->has_next()) {
        auto entry = iterator->current();

        info(
            std::format(
                "File entry: {} | Compressed Size: {} | Uncompressed Size: {}",
                entry.path,
                entry.size,
                entry.uncompressed_size
            )
                .c_str()
        );

        if (entry.uncompressed_size > 0) {
            auto file_data = iterator->extract_current();

            if (file_data.empty()) {
                error(
                    std::format("Failed to extract data for {}", entry.path)
                        .c_str()
                );
            } else {
                res.data[entry.path] = std::move(file_data);
            }
        }

        iterator->next();
    }

    if (res.data.empty()) {
        error("No valid files found in ZIP archive");
        return std::nullopt;
    }

    return res;
}

std::optional<LoadedData> load_resource(const std::string& path) {
    auto iterator = ArchiveIterator::from_file(path);
    if (!iterator) {
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

        std::vector<unsigned char> buffer(size);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            error("Failed to read file");
            return std::nullopt;
        }

        return load_resource_from_memory(buffer.data(), buffer.size());
    }

    LoadedData res;

    while (iterator->has_next()) {
        auto entry = iterator->current();

        info(
            std::format(
                "File entry: {} | Compressed Size: {} | Uncompressed Size: {}",
                entry.path,
                entry.size,
                entry.uncompressed_size
            )
                .c_str()
        );

        if (entry.uncompressed_size > 0) {
            auto file_data = iterator->extract_current();

            if (file_data.empty()) {
                error(
                    std::format("Failed to extract data for {}", entry.path)
                        .c_str()
                );
            } else {
                res.data[entry.path] = std::move(file_data);
            }
        }

        iterator->next();
    }

    if (res.data.empty()) {
        error("No valid files found in ZIP archive");
        return std::nullopt;
    }

    return res;
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