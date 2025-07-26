#ifndef KONDUIT_INSTALLER_ENCODING_HANDLING_HPP
#define KONDUIT_INSTALLER_ENCODING_HANDLING_HPP

#include <raylib.h>
#include <raylib/rres-raylib.h>
#include <raylib/rres.h>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <random>
#include <string>
#include "../main.hpp"

using std::nullopt;
using std::optional;

struct LoadedData {
    rresCentralDir dir;
    std::map<std::string, void*> data;
};

optional<LoadedData> load_resource(const std::string& path);

static std::vector<std::string> temp_files;

std::string write_to_temp_file(const unsigned char* data, size_t data_size);
bool remove_temp_file(const std::string& path);
bool remove_all_temp_files();

#endif  // KONDUIT_INSTALLER_ENCODING_HANDLING_HPP
