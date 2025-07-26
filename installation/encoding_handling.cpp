#include "encoding_handling.hpp"

// this will need to use something else eventually or just changing the library
// right now we have too much disk overhead for big bundles,
// but I can not do anything about it without digging in the rres code 🤷

rresResourceChunk chunk = {0};
rresResourceMulti multi = {0};

optional<LoadedData> load_resource(const std::string& path) {
    LoadedData res{};
    rresResourceChunkInfo* infos = nullptr;
    unsigned int chunkCount = 0;
    unsigned int prevId = 0;

    if (IsFileExtension(path.c_str(), ".rres")) {
        int r = 0;
        res.dir = rresLoadCentralDirectory(path.c_str());
        if (res.dir.count == 0) {
            error("no central dir in loaded resource");
            return nullopt;
        } else {
            for (unsigned int i = 0; i < res.dir.count; i++) {
                TraceLog(
                    LOG_INFO,
                    "RRES: CDIR: File entry %03i: %s | Resource(s) id: "
                    "0x%08x "
                    "| Offset: 0x%08x",
                    i + 1,
                    res.dir.entries[i].fileName,
                    res.dir.entries[i].id,
                    res.dir.entries[i].offset
                );
            }
            infos = rresLoadResourceChunkInfoAll("resources.rres", &chunkCount);
        }
        for (unsigned int i = 0; i < chunkCount; i++) {
            for (unsigned int j = 0; j < res.dir.count; j++) {
                if ((infos[i].id == res.dir.entries[j].id) &&
                    (infos[i].id != prevId)) {
                    chunk = rresLoadResourceChunk(
                        path.c_str(),
                        rresGetResourceId(res.dir, res.dir.entries[j].fileName)
                    );
                    r = UnpackResourceChunk(&chunk);
                    if (r == 0) {
                        unsigned int dataSize = 0;
                        res.data[res.dir.entries[j].fileName] =
                            LoadDataFromResource(chunk, &dataSize);
                    }
                    rresUnloadResourceChunk(chunk);
                }
            }
        }
    }

    RRES_FREE(infos);
    return res;
}

std::string write_to_temp_file(const unsigned char* data, size_t data_size) {
    namespace fs = std::filesystem;

    std::string temp_name =
        "bundle_" + std::to_string(std::random_device{}()) + ".rres";
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
