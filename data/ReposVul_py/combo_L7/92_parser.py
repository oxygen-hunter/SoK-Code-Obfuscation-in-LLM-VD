// SPDX-License-Identifier: ZPL-2.1

#include <iostream>
#include <string>
#include <regex>
#include <map>
#include <vector>
#include <memory>
#include <sstream>
#include <algorithm>
#include <cctype>

extern "C" {
    #include <waitress/buffers.h>
    #include <waitress/compat.h>
    #include <waitress/receiver.h>
    #include <waitress/utilities.h>
}

#define HTTP_1_0 "1.0"
#define HTTP_1_1 "1.1"

class ParsingError : public std::exception {
    std::string message;
public:
    explicit ParsingError(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

class HTTPRequestParser {
    bool completed;
    bool empty;
    bool expect_continue;
    bool headers_finished;
    std::string header_plus;
    bool chunked;
    int content_length;
    int header_bytes_received;
    int body_bytes_received;
    ChunkedReceiver* body_rcv;
    std::string version;
    ParsingError* error;
    bool connection_close;

    std::map<std::string, std::string> headers;
    Adjustments adj;

public:
    explicit HTTPRequestParser(Adjustments adj)
        : completed(false), empty(false), expect_continue(false), headers_finished(false),
          header_plus(""), chunked(false), content_length(0), header_bytes_received(0),
          body_bytes_received(0), body_rcv(nullptr), version(HTTP_1_0), error(nullptr),
          connection_close(false), adj(adj) {}

    int received(const std::string& data) {
        if (completed) return 0;

        size_t datalen = data.length();
        if (body_rcv == nullptr) {
            std::string s = header_plus + data;
            size_t index = find_double_newline(s);
            if (index != std::string::npos) {
                std::string header_plus = s.substr(0, index);
                int consumed = datalen - (s.length() - index);

                header_plus.erase(header_plus.begin(), std::find_if(header_plus.begin(), header_plus.end(),
                    [](unsigned char ch) { return !std::isspace(ch); }));

                if (header_plus.empty()) {
                    empty = true;
                    completed = true;
                } else {
                    try {
                        parse_header(header_plus);
                    } catch (ParsingError& e) {
                        error = new ParsingError(e.what());
                        completed = true;
                    } else {
                        if (body_rcv == nullptr) {
                            completed = true;
                        }
                        if (content_length > 0) {
                            int max_body = adj.max_request_body_size;
                            if (content_length >= max_body) {
                                error = new ParsingError("exceeds max_body of " + std::to_string(max_body));
                                completed = true;
                            }
                        }
                    }
                }
                headers_finished = true;
                return consumed;
            } else {
                header_bytes_received += datalen;
                int max_header = adj.max_request_header_size;
                if (header_bytes_received >= max_header) {
                    parse_header("GET / HTTP/1.0\n");
                    error = new ParsingError("exceeds max_header of " + std::to_string(max_header));
                    completed = true;
                }
                header_plus = s;
                return datalen;
            }
        } else {
            int consumed = body_rcv->received(data);
            body_bytes_received += consumed;
            int max_body = adj.max_request_body_size;
            if (body_bytes_received >= max_body) {
                error = new ParsingError("exceeds max_body of " + std::to_string(max_body));
                completed = true;
            } else if (body_rcv->has_error()) {
                error = body_rcv->error;
                completed = true;
            } else if (body_rcv->is_completed()) {
                completed = true;
                if (chunked) {
                    headers["CONTENT_LENGTH"] = std::to_string(body_rcv->length());
                }
            }
            return consumed;
        }
    }

    void parse_header(const std::string& header_plus) {
        size_t index = header_plus.find("\r\n");
        if (index != std::string::npos) {
            std::string first_line = header_plus.substr(0, index);
            first_line.erase(std::remove_if(first_line.begin(), first_line.end(), ::isspace), first_line.end());
            std::string header = header_plus.substr(index + 2);

            if (first_line.find("\r") != std::string::npos || first_line.find("\n") != std::string::npos) {
                throw ParsingError("Bare CR or LF found in HTTP message");
            }

            auto lines = get_header_lines(header);
            for (const auto& line : lines) {
                size_t index = line.find(":");
                if (index != std::string::npos) {
                    std::string key = line.substr(0, index);
                    if (key.find("_") != std::string::npos) {
                        continue;
                    }
                    std::string value = line.substr(index + 1);
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t") + 1);
                    std::transform(key.begin(), key.end(), key.begin(), ::toupper);
                    std::replace(key.begin(), key.end(), '-', '_');
                    if (headers.find(key) != headers.end()) {
                        headers[key] += ", " + value;
                    } else {
                        headers[key] = value;
                    }
                }
            }

            auto [method, uri, version] = crack_first_line(first_line);
            this->version = version;
            std::transform(command.begin(), command.end(), command.begin(), ::toupper);
            this->command = command;
            auto [proxy_scheme, proxy_netloc, path, query, fragment] = split_uri(uri);
            this->url_scheme = adj.url_scheme;
            std::string connection = headers["CONNECTION"];

            if (version == HTTP_1_0) {
                if (connection != "keep-alive") {
                    connection_close = true;
                }
            }

            if (version == HTTP_1_1) {
                std::string te = headers["TRANSFER_ENCODING"];
                if (te == "chunked") {
                    chunked = true;
                    auto buf = std::make_shared<OverflowableBuffer>(adj.inbuf_overflow);
                    body_rcv = new ChunkedReceiver(buf);
                }
                std::string expect = headers["EXPECT"];
                expect_continue = expect == "100-continue";
                if (connection == "close") {
                    connection_close = true;
                }
            }

            if (!chunked) {
                try {
                    content_length = std::stoi(headers["CONTENT_LENGTH"]);
                } catch (std::exception&) {
                    content_length = 0;
                }
                if (content_length > 0) {
                    auto buf = std::make_shared<OverflowableBuffer>(adj.inbuf_overflow);
                    body_rcv = new FixedStreamReceiver(content_length, buf);
                }
            }
        } else {
            throw ParsingError("HTTP message header invalid");
        }
    }

    std::shared_ptr<std::istream> get_body_stream() {
        if (body_rcv != nullptr) {
            return body_rcv->getfile();
        } else {
            return std::make_shared<std::stringstream>();
        }
    }

    void close() {
        if (body_rcv != nullptr) {
            body_rcv->getbuf()->close();
        }
    }
};

std::tuple<std::string, std::string, std::string, std::string, std::string> split_uri(const std::string& uri) {
    std::string scheme, netloc, path, query, fragment;

    if (uri.substr(0, 2) == "//") {
        path = uri;
        size_t hash_pos = path.find("#");
        if (hash_pos != std::string::npos) {
            fragment = path.substr(hash_pos + 1);
            path = path.substr(0, hash_pos);
        }
        size_t query_pos = path.find("?");
        if (query_pos != std::string::npos) {
            query = path.substr(query_pos + 1);
            path = path.substr(0, query_pos);
        }
    } else {
        try {
            std::tie(scheme, netloc, path, query, fragment) = urlparse::urlsplit(uri);
        } catch (std::exception&) {
            throw ParsingError("Bad URI");
        }
    }

    return std::make_tuple(scheme, netloc, unquote_bytes_to_wsgi(path), query, fragment);
}

std::vector<std::string> get_header_lines(const std::string& header) {
    std::vector<std::string> r;
    std::istringstream stream(header);
    std::string line;
    while (std::getline(stream, line, '\r')) {
        if (line.find("\r") != std::string::npos || line.find("\n") != std::string::npos) {
            throw ParsingError("Bare CR or LF found in header line \"" + line + "\"");
        }

        if (line.front() == ' ' || line.front() == '\t') {
            if (r.empty()) {
                throw ParsingError("Malformed header line \"" + line + "\"");
            }
            r.back() += line;
        } else {
            r.push_back(line);
        }
    }
    return r;
}

std::tuple<std::string, std::string, std::string> crack_first_line(const std::string& line) {
    std::regex first_line_re(R"(^([^ ]+) ((?:[^ :?#]+://[^ ?#/]*(?:[0-9]{1,5})?)?[^ ]+)( (HTTP/([0-9.]+))$|$))");
    std::smatch match;
    if (std::regex_match(line, match, first_line_re)) {
        std::string version = match[5].str();
        std::string method = match[1].str();
        if (method != method) {
            throw ParsingError("Malformed HTTP method \"" + method + "\"");
        }
        std::string uri = match[2].str();
        return std::make_tuple(method, uri, version);
    } else {
        return std::make_tuple("", "", "");
    }
}