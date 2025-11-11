#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <vector>
#include <regex>
#include <zip.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <chimera/chi.h>

extern "C" void call_jadx(const char *jadx_path, const char *dex, const char *output_dir) {
    std::string command = std::string(jadx_path) + " " + dex + " -d " + output_dir + " --deobf";
    system(command.c_str());
}

class APKLeaks {
public:
    APKLeaks(const std::string &file, const std::string &output, const std::string &pattern)
        : file(file), output(output), pattern(pattern) {
        prefix = "apkleaks-";
        tempdir = std::filesystem::temp_directory_path() / (prefix + std::to_string(rand()));
        std::filesystem::create_directory(tempdir);
        main_dir = std::filesystem::canonical(__FILE__).parent_path();
        if (output.empty()) {
            this->output = (std::filesystem::temp_directory_path() / (prefix + std::to_string(rand()) + ".txt")).string();
        }
        if (pattern.empty()) {
            this->pattern = (main_dir / "../config/regexes.json").string();
        }
        this->jadx = chi::find_executable("jadx");
        if (this->jadx.empty()) {
            this->jadx = (main_dir / "../jadx/bin/jadx").string();
        }
        chi::disable_existing_loggers();
    }

    void apk_info() {
        // Placeholder for APK info retrieval logic
    }

    void dependencies() {
        std::string exter = "https://github.com/skylot/jadx/releases/download/v1.2.0/jadx-1.2.0.zip";
        std::vector<char> buffer;
        chi::download_into_buffer(exter, buffer);
        chi::unzip_buffer(buffer, main_dir / "../jadx");
        std::filesystem::permissions(jadx, std::filesystem::perms::owner_all);
    }

    void write(const std::string &message, const std::string &color) {
        std::cout << color << message << chi::clr::ENDC;
    }

    void writeln(const std::string &message, const std::string &color) {
        write(message + "\n", color);
    }

    void integrity() {
        if (!std::filesystem::exists(jadx)) {
            writeln("Can't find jadx binary.", chi::clr::WARNING);
            std::string choice;
            while (true) {
                write("Do you want to download jadx? (Y/n) ", chi::clr::OKBLUE);
                std::getline(std::cin, choice);
                std::transform(choice.begin(), choice.end(), choice.begin(), ::tolower);
                if (choice.empty() || choice == "y" || choice == "yes") {
                    dependencies();
                    break;
                } else if (choice == "n" || choice == "no") {
                    std::exit(EXIT_FAILURE);
                } else {
                    writeln("\nPlease respond with 'yes' or 'no' (or 'y' or 'n').", chi::clr::WARNING);
                }
            }
        }
        // Placeholder for APK file validation
    }

    void decompile() {
        writeln("** Decompiling APK...", chi::clr::OKBLUE);
        std::string dex = (tempdir / (apk.package + ".dex")).string();
        // Placeholder for decompiling logic
        call_jadx(jadx.c_str(), dex.c_str(), tempdir.c_str());
    }

    std::vector<std::string> unique(const std::vector<std::string> &list) {
        std::set<std::string> s(list.begin(), list.end());
        return std::vector<std::string>(s.begin(), s.end());
    }

    std::vector<std::string> finder(const std::string &pattern, const std::filesystem::path &path) {
        std::regex matcher(pattern);
        std::vector<std::string> found;
        for (const auto &entry : std::filesystem::recursive_directory_iterator(path)) {
            if (std::filesystem::is_regular_file(entry.path())) {
                std::ifstream file(entry.path());
                std::string line;
                while (std::getline(file, line)) {
                    if (std::regex_search(line, matcher)) {
                        found.push_back(line);
                    }
                }
            }
        }
        return unique(found);
    }

    void extract(const std::string &name, const std::vector<std::string> &matches) {
        std::ofstream output_file(output, std::ios::app);
        if (!matches.empty()) {
            std::string stdout = "[" + name + "]";
            writeln("\n" + stdout, chi::clr::OKGREEN);
            output_file << stdout << "\n";
            for (const auto &secret : matches) {
                if (name == "LinkFinder" && std::regex_match(secret, std::regex(R"(^.(L[a-z]|application|audio|fonts|image|layout|multipart|plain|text|video).*\/.+)"))) {
                    continue;
                }
                stdout = "- " + secret;
                std::cout << stdout << std::endl;
                output_file << stdout << "\n";
            }
            output_file << "\n";
        }
    }

    void scanning() {
        writeln("\n** Scanning against '%s'" + apk.package, chi::clr::OKBLUE);
        std::ifstream regexes(pattern);
        nlohmann::json regex;
        regexes >> regex;
        for (const auto &item : regex.items()) {
            const std::string &name = item.key();
            const auto &patterns = item.value();
            if (patterns.is_array()) {
                for (const auto &pattern : patterns) {
                    std::thread t(&APKLeaks::extract, this, name, finder(pattern, tempdir));
                    t.detach();
                }
            } else {
                std::thread t(&APKLeaks::extract, this, name, finder(patterns, tempdir));
                t.detach();
            }
        }
    }

    ~APKLeaks() {
        std::cout << chi::clr::OKBLUE << "\n** Results saved into '" << chi::clr::ENDC << chi::clr::OKGREEN << output << chi::clr::OKBLUE << "'" << chi::clr::ENDC << std::endl;
        std::filesystem::remove_all(tempdir);
    }

private:
    std::string file;
    std::string output;
    std::string pattern;
    std::string prefix;
    std::filesystem::path tempdir;
    std::filesystem::path main_dir;
    std::string jadx;
    // Placeholder for APK class
    APK apk;
};