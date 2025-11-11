// Combining C++ with inline assembly

#include <iostream>
#include <string>
#include <regex>
#include <sstream>
#include <map>
#include <vector>
#include <functional>
#include <cassert>
#include <cctype>

namespace sqlparse {
    namespace tokens {
        enum _TokenType { Name, Comment, Assignment, Punctuation, Wildcard, Keyword, Newline, Whitespace, Number, String, Operator, Error, Text };
    }

    namespace keywords {
        std::map<std::string, tokens::_TokenType> KEYWORDS;
        std::map<std::string, tokens::_TokenType> KEYWORDS_COMMON;
    }

    class StringIO {
    public:
        StringIO(const std::string &str) : stream(str) {}
        std::stringstream stream;
    };
}

class include : public std::string {
public:
    using std::string::string;
};

class combined : public std::vector<std::string> {
public:
    using std::vector<std::string>::vector;
};

tokens::_TokenType is_keyword(const std::string &value) {
    auto test = value;
    for (auto &c : test) c = std::toupper(c);
    auto it = sqlparse::keywords::KEYWORDS_COMMON.find(test);
    if (it != sqlparse::keywords::KEYWORDS_COMMON.end()) return it->second;
    it = sqlparse::keywords::KEYWORDS.find(test);
    if (it != sqlparse::keywords::KEYWORDS.end()) return it->second;
    return sqlparse::tokens::Name;
}

template <typename Filter>
std::vector<std::pair<tokens::_TokenType, std::string>> apply_filters(
    std::vector<std::pair<tokens::_TokenType, std::string>> &stream, const std::vector<Filter> &filters, void *lexer = nullptr) {
    for (const auto &filter : filters) {
        std::vector<std::pair<tokens::_TokenType, std::string>> new_stream;
        for (const auto &token : stream) {
            auto filtered_tokens = filter(lexer, token);
            new_stream.insert(new_stream.end(), filtered_tokens.begin(), filtered_tokens.end());
        }
        stream = std::move(new_stream);
    }
    return stream;
}

class LexerMeta {
public:
    virtual void process_tokendef() {
        std::cout << "Processing token definitions" << std::endl;
    }
};

class Lexer : public LexerMeta {
public:
    Lexer() {
        filters = {};
    }

    void add_filter(std::function<std::vector<std::pair<tokens::_TokenType, std::string>>(void *, const std::pair<tokens::_TokenType, std::string> &)> filter) {
        filters.push_back(filter);
    }

    std::vector<std::pair<tokens::_TokenType, std::string>> get_tokens(const std::string &text, bool unfiltered = false) {
        std::vector<std::pair<tokens::_TokenType, std::string>> stream;
        stream.emplace_back(tokens::Text, text);
        if (!unfiltered) {
            stream = apply_filters(stream, filters, this);
        }
        return stream;
    }

private:
    std::vector<std::function<std::vector<std::pair<tokens::_TokenType, std::string>>(void *, const std::pair<tokens::_TokenType, std::string> &)>> filters;
};

extern "C" {
    bool isEven(int number) {
        int result;
        __asm__ (
            "movl %1, %%eax;"
            "andl $1, %%eax;"
            "movl %%eax, %0;"
            : "=r" (result)
            : "r" (number)
            : "%eax"
        );
        return result == 0;
    }
}

int main() {
    Lexer lexer;
    lexer.add_filter([](void *, const std::pair<tokens::_TokenType, std::string> &token) {
        return std::vector<std::pair<tokens::_TokenType, std::string>>{token};
    });

    auto tokens = lexer.get_tokens("SELECT * FROM table;");
    for (const auto &token : tokens) {
        std::cout << "Token: " << token.second << std::endl;
    }

    std::cout << "Is 4 even? " << isEven(4) << std::endl;
    return 0;
}