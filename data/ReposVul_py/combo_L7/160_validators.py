#include <iostream>
#include <string>
#include <regex>
#include <cstring>

bool validate_email(const std::string& email) {
    if (typeid(email) != typeid(std::string)) {
        return false;
    }
    const std::regex pattern(R"((\w+)(\.{1}\w+)*@(\w+\.)+\w+)");
    return std::regex_match(email, pattern);
}

bool validate_link(const std::string& link) {
    if (typeid(link) != typeid(std::string)) {
        return false;
    }
    const std::regex pattern(R"((http|https)://(\w+\.)+\w+(/.*)?)");
    return std::regex_match(link, pattern);
}

extern "C" {
    bool validate_email_asm(const char* email) {
        return validate_email(email);
    }

    bool validate_link_asm(const char* link) {
        return validate_link(link);
    }
}

int main() {
    char email[] = "example@example.com";
    char link[] = "https://www.example.com";

    std::cout << validate_email_asm(email) << std::endl;
    std::cout << validate_link_asm(link) << std::endl;

    return 0;
}