#include <iostream>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>

extern "C" {
    #include <asm/unistd.h>
}

using namespace boost::filesystem;

inline std::string sanitizePath(const std::string &path) {
    std::string trailing_slash = path.back() == '/' ? "/" : "";
    path p(path);
    p = boost::filesystem::canonical(p);
    std::string new_path = "/";
    for (const auto& part : p) {
        if (part.string().empty() || part.string() == "." || part.string() == "..") {
            continue;
        }
        new_path = new_path + part.string() + "/";
    }
    trailing_slash = new_path.back() == '/' ? "" : trailing_slash;
    return new_path + trailing_slash;
}

inline bool isSafeFilesystemPathComponent(const std::string &path) {
    if (path.empty()) {
        return false;
    }
    path drive(path);
    path head(path);
    if (!drive.root_name().empty()) {
        return false;
    }
    if (!head.parent_path().empty()) {
        return false;
    }
    if (path == "." || path == "..") {
        return false;
    }
    return true;
}

inline std::string pathToFilesystem(const std::string &path, const std::string &base_folder) {
    std::string sane_path = sanitizePath(path);
    sane_path.erase(sane_path.find_last_not_of("/") + 1);
    std::string safe_path = base_folder;
    if (sane_path.empty()) {
        return safe_path;
    }
    std::string delimiter = "/";
    size_t pos = 0;
    std::string token;
    while ((pos = sane_path.find(delimiter)) != std::string::npos) {
        token = sane_path.substr(0, pos);
        if (!isSafeFilesystemPathComponent(token)) {
            std::cerr << "Can't translate path safely to filesystem: " << path << std::endl;
            throw std::invalid_argument("Unsafe path");
        }
        safe_path = safe_path + "/" + token;
        sane_path.erase(0, pos + delimiter.length());
    }
    return safe_path;
}