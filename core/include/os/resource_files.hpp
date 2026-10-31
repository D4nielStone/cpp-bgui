#pragma once
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace bgui::resource_files {
using bytes = std::vector<unsigned char>;
struct provider {
    std::function<bool(const std::filesystem::path&)> exists;
    std::function<bytes(const std::filesystem::path&)> read;
};
inline provider mounted;
inline bool embedded() { return static_cast<bool>(mounted.read); }
inline bool exists(const std::filesystem::path& path) {
    if (embedded()) return mounted.exists(path);
    std::error_code error;
    return std::filesystem::is_regular_file(path, error);
}
inline bytes read(const std::filesystem::path& path) {
    if (embedded()) return mounted.read(path); // No loose-file override of embedded resources.
    std::ifstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("Could not read resource: " + path.string());
    bytes result{std::istreambuf_iterator<char>(file), {}};
    if (file.bad()) throw std::runtime_error("Could not read resource: " + path.string());
    return result;
}
inline std::string text(const std::filesystem::path& path) {
    const auto data = read(path);
    return {data.begin(), data.end()};
}
inline std::istringstream stream(const std::filesystem::path& path) {
    try { return std::istringstream(text(path)); }
    catch (const std::exception&) { std::istringstream failed; failed.setstate(std::ios::failbit); return failed; }
}
}
