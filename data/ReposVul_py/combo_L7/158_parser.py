#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <sstream>
#include <iterator>
#include <map>
#include <nlohmann/json.hpp>
#include <solidity-ast/ast.h>
#include <solidity-ast/visitor.h>
#include <boost/algorithm/string.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

extern "C"
{
#include <yaml.h>
}

using json = nlohmann::json;
namespace py = pybind11;

using namespace std;

class Dependency
{
public:
    string name;
    string specs;
    string line;
    vector<string> extras;
    string dependency_type;
    vector<string> hashes;
    string index_server;
    string section;
};

class DependencyFile
{
public:
    string content;
    string path;
    string sha;
    tuple<vector<string>, vector<string>> marker;
    string file_type;
    function<void(DependencyFile &)> parser;
    vector<Dependency> dependencies;
    vector<string> resolved_files;

    void parse()
    {
        if (parser)
        {
            parser(*this);
        }
    }
};

class Parser
{
public:
    string obj_content;
    vector<string> _lines;

    Parser(string content) : obj_content(content), _lines({})
    {
    }

    vector<string> iter_lines(size_t lineno = 0)
    {
        vector<string> lines;
        for (size_t i = lineno; i < _lines.size(); ++i)
        {
            lines.push_back(_lines[i]);
        }
        return lines;
    }

    vector<string> lines()
    {
        if (_lines.empty())
        {
            istringstream stream(obj_content);
            string line;
            while (getline(stream, line))
            {
                _lines.push_back(line);
            }
        }
        return _lines;
    }

    bool is_marked_file()
    {
        for (size_t n = 0; n < iter_lines().size(); ++n)
        {
            string line = iter_lines()[n];
            for (const string &marker : get<0>(marker))
            {
                if (line.find(marker) != string::npos)
                {
                    return true;
                }
            }
            if (n >= 2)
            {
                break;
            }
        }
        return false;
    }

    bool is_marked_line(const string &line)
    {
        for (const string &marker : get<1>(marker))
        {
            if (line.find(marker) != string::npos)
            {
                return true;
            }
        }
        return false;
    }

    static pair<string, vector<string>> parse_hashes(const string &line)
    {
        vector<string> hashes;
        regex hash_regex(R"((sha256|md5)=\S+)");
        smatch match;
        string line_copy = line;
        while (regex_search(line_copy, match, hash_regex))
        {
            hashes.push_back(match.str());
            line_copy = match.suffix();
        }
        return {regex_replace(line, hash_regex, ""), hashes};
    }

    static string parse_index_server(const string &line)
    {
        vector<string> groups;
        boost::split(groups, line, boost::is_any_of("= "), boost::token_compress_on);
        if (groups.size() >= 2)
        {
            return (groups[1].back() == '/') ? groups[1] : groups[1] + "/";
        }
        return "";
    }

    static string resolve_file(const string &file_path, const string &line)
    {
        string modified_line = line;
        boost::replace_all(modified_line, "-r ", "");
        boost::replace_all(modified_line, "--requirement ", "");
        vector<string> parts;
        boost::split(parts, file_path, boost::is_any_of("/"), boost::token_compress_on);
        if (modified_line.find(" #") != string::npos)
        {
            modified_line = modified_line.substr(0, modified_line.find("#"));
            boost::trim(modified_line);
        }
        if (parts.size() == 1)
        {
            return modified_line;
        }
        else
        {
            ostringstream oss;
            copy(parts.begin(), parts.end() - 1, ostream_iterator<string>(oss, "/"));
            oss << modified_line;
            return oss.str();
        }
    }
};

class RequirementsTXTParser : public Parser
{
public:
    RequirementsTXTParser(string content) : Parser(content)
    {
    }

    void parse()
    {
        string index_server;
        for (const string &line : iter_lines())
        {
            string trimmed_line = line;
            boost::trim(trimmed_line);
            if (trimmed_line.empty() || trimmed_line[0] == '#')
            {
                continue;
            }
            if (boost::starts_with(trimmed_line, "-i") || boost::starts_with(trimmed_line, "--index-url") || boost::starts_with(trimmed_line, "--extra-index-url"))
            {
                index_server = parse_index_server(trimmed_line);
                continue;
            }
            if (path && (boost::starts_with(trimmed_line, "-r") || boost::starts_with(trimmed_line, "--requirement")))
            {
                resolved_files.push_back(resolve_file(path, trimmed_line));
            }
            else if (boost::starts_with(trimmed_line, "-f") || boost::starts_with(trimmed_line, "--find-links") || boost::starts_with(trimmed_line, "--no-index") || boost::starts_with(trimmed_line, "--allow-external") || boost::starts_with(trimmed_line, "--allow-unverified") || boost::starts_with(trimmed_line, "-Z") || boost::starts_with(trimmed_line, "--always-unzip"))
            {
                continue;
            }
            else if (is_marked_line(trimmed_line))
            {
                continue;
            }
            else
            {
                try
                {
                    string parseable_line = trimmed_line;
                    if (trimmed_line.find("\\") != string::npos)
                    {
                        boost::replace_all(parseable_line, "\\", "");
                        for (const string &next_line : iter_lines())
                        {
                            string next_trimmed_line = next_line;
                            boost::trim(next_trimmed_line);
                            if (next_trimmed_line.find("\\") != string::npos)
                            {
                                parseable_line += next_trimmed_line;
                                continue;
                            }
                            break;
                        }
                        if (is_marked_line(parseable_line))
                        {
                            continue;
                        }
                    }

                    vector<string> hashes;
                    if (parseable_line.find("--hash") != string::npos)
                    {
                        tie(parseable_line, hashes) = parse_hashes(parseable_line);
                    }

                    auto req = RequirementsTXTLineParser::parse(parseable_line);
                    if (req)
                    {
                        req.hashes = hashes;
                        req.index_server = index_server;
                        req.line = trimmed_line;
                        dependencies.push_back(req);
                    }
                }
                catch (const exception &e)
                {
                    continue;
                }
            }
        }
    }
};

class ToxINIParser : public Parser
{
public:
    ToxINIParser(string content) : Parser(content)
    {
    }

    void parse()
    {
        ConfigParser parser;
        istringstream stream(obj_content);
        parser.readfp(stream);
        for (const auto &section : parser.sections())
        {
            try
            {
                string content = parser.get(section, "deps");
                istringstream content_stream(content);
                string line;
                while (getline(content_stream, line))
                {
                    if (is_marked_line(line))
                    {
                        continue;
                    }
                    if (!line.empty())
                    {
                        auto req = RequirementsTXTLineParser::parse(line);
                        if (req)
                        {
                            req.dependency_type = file_type;
                            dependencies.push_back(req);
                        }
                    }
                }
            }
            catch (const NoOptionError &e)
            {
            }
        }
    }
};

class CondaYMLParser : public Parser
{
public:
    CondaYMLParser(string content) : Parser(content)
    {
    }

    void parse()
    {
        yaml_parser_t parser;
        yaml_document_t document;

        yaml_parser_initialize(&parser);
        yaml_parser_set_input_string(&parser, (const unsigned char *)obj_content.c_str(), obj_content.size());

        if (!yaml_parser_load(&parser, &document))
        {
            yaml_parser_delete(&parser);
            return;
        }

        yaml_node_t *root = yaml_document_get_root_node(&document);
        if (root && root->type == YAML_MAPPING_NODE)
        {
            for (yaml_node_pair_t *pair = root->data.mapping.pairs.start; pair < root->data.mapping.pairs.top; ++pair)
            {
                yaml_node_t *key = yaml_document_get_node(&document, pair->key);
                yaml_node_t *value = yaml_document_get_node(&document, pair->value);

                if (key->type == YAML_SCALAR_NODE && string((char *)key->data.scalar.value) == "dependencies")
                {
                    if (value->type == YAML_SEQUENCE_NODE)
                    {
                        for (yaml_node_item_t *item = value->data.sequence.items.start; item < value->data.sequence.items.top; ++item)
                        {
                            yaml_node_t *dep = yaml_document_get_node(&document, *item);

                            if (dep->type == YAML_MAPPING_NODE)
                            {
                                for (yaml_node_pair_t *dep_pair = dep->data.mapping.pairs.start; dep_pair < dep->data.mapping.pairs.top; ++dep_pair)
                                {
                                    yaml_node_t *dep_key = yaml_document_get_node(&document, dep_pair->key);
                                    yaml_node_t *dep_value = yaml_document_get_node(&document, dep_pair->value);

                                    if (dep_key->type == YAML_SCALAR_NODE && string((char *)dep_key->data.scalar.value) == "pip")
                                    {
                                        if (dep_value->type == YAML_SEQUENCE_NODE)
                                        {
                                            for (yaml_node_item_t *pip_item = dep_value->data.sequence.items.start; pip_item < dep_value->data.sequence.items.top; ++pip_item)
                                            {
                                                yaml_node_t *pip_dep = yaml_document_get_node(&document, *pip_item);

                                                if (pip_dep->type == YAML_SCALAR_NODE)
                                                {
                                                    string line((char *)pip_dep->data.scalar.value);
                                                    if (is_marked_line(line))
                                                    {
                                                        continue;
                                                    }
                                                    auto req = RequirementsTXTLineParser::parse(line);
                                                    if (req)
                                                    {
                                                        req.dependency_type = file_type;
                                                        dependencies.push_back(req);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        yaml_document_delete(&document);
        yaml_parser_delete(&parser);
    }
};

class PipfileParser : public Parser
{
public:
    PipfileParser(string content) : Parser(content)
    {
    }

    void parse()
    {
        try
        {
            json data = json::parse(obj_content);
            if (!data.is_null())
            {
                for (const string &package_type : {"packages", "dev-packages"})
                {
                    if (data.contains(package_type))
                    {
                        for (auto &item : data[package_type].items())
                        {
                            string name = item.key();
                            string specs = item.value().is_string() ? item.value().get<string>() : "*";
                            dependencies.push_back(Dependency{
                                .name = name,
                                .specs = specs,
                                .dependency_type = "pipfile",
                                .line = name + specs,
                                .section = package_type});
                        }
                    }
                }
            }
        }
        catch (const json::exception &e)
        {
        }
    }
};

class PipfileLockParser : public Parser
{
public:
    PipfileLockParser(string content) : Parser(content)
    {
    }

    void parse()
    {
        try
        {
            json data = json::parse(obj_content);
            if (!data.is_null())
            {
                for (const string &package_type : {"default", "develop"})
                {
                    if (data.contains(package_type))
                    {
                        for (auto &item : data[package_type].items())
                        {
                            string name = item.key();
                            if (item.value().contains("version"))
                            {
                                string specs = item.value()["version"];
                                vector<string> hashes = item.value()["hashes"].get<vector<string>>();
                                dependencies.push_back(Dependency{
                                    .name = name,
                                    .specs = specs,
                                    .dependency_type = "pipfile_lock",
                                    .hashes = hashes,
                                    .line = name + specs,
                                    .section = package_type});
                            }
                        }
                    }
                }
            }
        }
        catch (const json::exception &e)
        {
        }
    }
};

class SetupCfgParser : public Parser
{
public:
    SetupCfgParser(string content) : Parser(content)
    {
    }

    void parse()
    {
        ConfigParser parser;
        istringstream stream(obj_content);
        parser.readfp(stream);
        for (const auto &section : parser.values())
        {
            if (section.name == "options")
            {
                vector<string> options = {"install_requires", "setup_requires", "test_require"};
                for (const string &name : options)
                {
                    string content = section.get(name);
                    if (!content.empty())
                    {
                        _parse_content(content);
                    }
                }
            }
            else if (section.name == "options.extras_require")
            {
                for (const string &content : section.values())
                {
                    _parse_content(content);
                }
            }
        }
    }

    void _parse_content(const string &content)
    {
        istringstream content_stream(content);
        string line;
        while (getline(content_stream, line))
        {
            if (is_marked_line(line))
            {
                continue;
            }
            if (!line.empty())
            {
                auto req = RequirementsTXTLineParser::parse(line);
                if (req)
                {
                    req.dependency_type = "setup_cfg";
                    dependencies.push_back(req);
                }
            }
        }
    }
};

PYBIND11_MODULE(parsing_module, m)
{
    m.def("parse", [](const string &content, const py::object &file_type, const py::object &path, const py::object &sha, const py::tuple &marker, const py::function &parser)
          {
              DependencyFile dep_file = {
                  .content = content,
                  .path = path.cast<string>(),
                  .sha = sha.cast<string>(),
                  .marker = marker.cast<tuple<vector<string>, vector<string>>>(),
                  .file_type = file_type.cast<string>(),
                  .parser = parser.cast<function<void(DependencyFile &)>>(),
                  .dependencies = {},
                  .resolved_files = {}};

              dep_file.parse();
              return dep_file.dependencies;
          });
}