import inspect
import os
import subprocess
import sys
import tempfile

from nltk.data import ZipFilePathPointer
from nltk.internals import find_dir, find_file, find_jars_within_path
from nltk.parse.api import ParserI
from nltk.parse.dependencygraph import DependencyGraph
from nltk.parse.util import taggedsents_to_conll

def malt_regex_tagger():
    from nltk.tag import RegexpTagger

    _tagger = RegexpTagger(
        [
            (r"\.$", "."),
            (r"\,$", ","),
            (r"\?$", "?"),
            (r"\($", "("),
            (r"\)$", ")"),
            (r"\[$", "["),
            (r"\]$", "]"),
            (r"^-?[0-9]+(\.[0-9]+)?$", "CD"),
            (r"(The|the|A|a|An|an)$", "DT"),
            (r"(He|he|She|she|It|it|I|me|Me|You|you)$", "PRP"),
            (r"(His|his|Her|her|Its|its)$", "PRP$"),
            (r"(my|Your|your|Yours|yours)$", "PRP$"),
            (r"(on|On|in|In|at|At|since|Since)$", "IN"),
            (r"(for|For|ago|Ago|before|Before)$", "IN"),
            (r"(till|Till|until|Until)$", "IN"),
            (r"(by|By|beside|Beside)$", "IN"),
            (r"(under|Under|below|Below)$", "IN"),
            (r"(over|Over|above|Above)$", "IN"),
            (r"(across|Across|through|Through)$", "IN"),
            (r"(into|Into|towards|Towards)$", "IN"),
            (r"(onto|Onto|from|From)$", "IN"),
            (r".*able$", "JJ"),
            (r".*ness$", "NN"),
            (r".*ly$", "RB"),
            (r".*s$", "NNS"),
            (r".*ing$", "VBG"),
            (r".*ed$", "VBD"),
            (r".*", "NN"),
        ]
    )
    return _tagger.tag

def find_maltparser(parser_dirname):
    if os.path.exists(parser_dirname):
        _malt_dir = parser_dirname
    else:
        _malt_dir = find_dir(parser_dirname, env_vars=("MALT_PARSER",))
    malt_dependencies = ["", "", ""]
    _malt_jars = set(find_jars_within_path(_malt_dir))
    _jars = {os.path.split(jar)[1] for jar in _malt_jars}
    malt_dependencies = {"log4j.jar", "libsvm.jar", "liblinear-1.8.jar"}

    assert malt_dependencies.issubset(_jars)
    assert any(
        filter(lambda i: i.startswith("maltparser-") and i.endswith(".jar"), _jars)
    )
    return list(_malt_jars)

def find_malt_model(model_filename):
    if model_filename is None:
        return "malt_temp.mco"
    elif os.path.exists(model_filename):
        return model_filename
    else:
        return find_file(model_filename, env_vars=("MALT_MODEL",), verbose=False)

class MaltParser(ParserI):
    def __init__(
        self,
        parser_dirname="",
        model_filename=None,
        tagger=None,
        additional_java_args=None,
    ):
        self.malt_jars = find_maltparser(parser_dirname)
        self.additional_java_args = (
            additional_java_args if additional_java_args is not None else []
        )
        self.model = find_malt_model(model_filename)
        self._trained = self.model != "malt_temp.mco"
        self.working_dir = tempfile.gettempdir()
        self.tagger = tagger if tagger is not None else malt_regex_tagger()

    def parse_tagged_sents(self, sentences, verbose=False, top_relation_label="null"):
        if not self._trained:
            raise Exception("Parser has not been trained. Call train() first.")

        with tempfile.NamedTemporaryFile(
            prefix="malt_input.conll.", dir=self.working_dir, mode="w", delete=False
        ) as input_file:
            with tempfile.NamedTemporaryFile(
                prefix="malt_output.conll.",
                dir=self.working_dir,
                mode="w",
                delete=False,
            ) as output_file:
                for line in taggedsents_to_conll(sentences):
                    input_file.write(str(line))
                input_file.close()

                cmd = self.generate_malt_command(
                    input_file.name, output_file.name, mode="parse"
                )

                _current_path = os.getcwd()
                try:
                    os.chdir(os.path.split(self.model)[0])
                except:
                    pass
                ret = self._execute(cmd, verbose)
                os.chdir(_current_path)

                if ret != 0:
                    raise Exception(
                        "MaltParser parsing (%s) failed with exit "
                        "code %d" % (" ".join(cmd), ret)
                    )

                with open(output_file.name) as infile:
                    for tree_str in infile.read().split("\n\n"):
                        yield (
                            iter(
                                [
                                    DependencyGraph(
                                        tree_str, top_relation_label=top_relation_label
                                    )
                                ]
                            )
                        )

        os.remove(input_file.name)
        os.remove(output_file.name)

    def parse_sents(self, sentences, verbose=False, top_relation_label="null"):
        tagged_sentences = (self.tagger(sentence) for sentence in sentences)
        return self.parse_tagged_sents(
            tagged_sentences, verbose, top_relation_label=top_relation_label
        )

    def generate_malt_command(self, inputfilename, outputfilename=None, mode=None):
        cmd = ["java"]
        cmd += self.additional_java_args
        classpaths_separator = ";" if sys.platform.startswith("win") else ":"
        cmd += [
            "-cp",
            classpaths_separator.join(self.malt_jars),
        ]
        cmd += ["org.maltparser.Malt"]

        if os.path.exists(self.model):
            cmd += ["-c", os.path.split(self.model)[-1]]
        else:
            cmd += ["-c", self.model]

        cmd += ["-i", inputfilename]
        if mode == "parse":
            cmd += ["-o", outputfilename]
        cmd += ["-m", mode]
        return cmd

    @staticmethod
    def _execute(cmd, verbose=False):
        output = None if verbose else subprocess.PIPE
        p = subprocess.Popen(cmd, stdout=output, stderr=output)
        return p.wait()

    def train(self, depgraphs, verbose=False):
        with tempfile.NamedTemporaryFile(
            prefix="malt_train.conll.", dir=self.working_dir, mode="w", delete=False
        ) as input_file:
            input_str = "\n".join(dg.to_conll(10) for dg in depgraphs)
            input_file.write(str(input_str))
        self.train_from_file(input_file.name, verbose=verbose)
        os.remove(input_file.name)

    def train_from_file(self, conll_file, verbose=False):
        if isinstance(conll_file, ZipFilePathPointer):
            with tempfile.NamedTemporaryFile(
                prefix="malt_train.conll.", dir=self.working_dir, mode="w", delete=False
            ) as input_file:
                with conll_file.open() as conll_input_file:
                    conll_str = conll_input_file.read()
                    input_file.write(str(conll_str))
                return self.train_from_file(input_file.name, verbose=verbose)

        cmd = self.generate_malt_command(conll_file, mode="learn")
        ret = self._execute(cmd, verbose)
        if ret != 0:
            raise Exception(
                "MaltParser training (%s) failed with exit "
                "code %d" % (" ".join(cmd), ret)
            )
        self._trained = True
```

```c++
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <iterator>
#include <algorithm>

extern "C" {
    #include <unistd.h>
}

class MaltParser {
public:
    MaltParser(std::string parser_dirname="", std::string model_filename="", std::string tagger="", std::vector<std::string> additional_java_args={})
        : additional_java_args(additional_java_args), working_dir("/tmp") {
        find_maltparser(parser_dirname);
        find_malt_model(model_filename);
        _trained = model != "malt_temp.mco";
        this->tagger = tagger.empty() ? malt_regex_tagger() : tagger;
    }

    void parse_tagged_sents(const std::vector<std::vector<std::pair<std::string, std::string>>>& sentences, bool verbose=false, std::string top_relation_label="null") {
        if (!_trained) {
            throw std::runtime_error("Parser has not been trained. Call train() first.");
        }

        std::string input_file_name = "/tmp/malt_input.conll." + std::to_string(getpid());
        std::string output_file_name = "/tmp/malt_output.conll." + std::to_string(getpid());

        std::ofstream input_file(input_file_name);
        for (const auto& sentence : sentences) {
            for (const auto& word_tag : sentence) {
                input_file << word_tag.first << "\t" << word_tag.second << "\n";
            }
            input_file << "\n";
        }
        input_file.close();

        std::vector<std::string> cmd = generate_malt_command(input_file_name, output_file_name, "parse");
        std::string current_path = getcwd(NULL, 0);
        chdir(model.substr(0, model.find_last_of("/")).c_str());

        int ret = _execute(cmd, verbose);
        chdir(current_path.c_str());

        if (ret != 0) {
            throw std::runtime_error("MaltParser parsing failed with exit code " + std::to_string(ret));
        }

        std::ifstream infile(output_file_name);
        std::string tree_str;
        while (std::getline(infile, tree_str, '\n')) {
            std::cout << tree_str << std::endl;
        }
        infile.close();

        remove(input_file_name.c_str());
        remove(output_file_name.c_str());
    }

private:
    std::vector<std::string> malt_jars;
    std::vector<std::string> additional_java_args;
    std::string model;
    bool _trained;
    std::string working_dir;
    std::string tagger;

    std::string malt_regex_tagger() {
        return "default_tagger";
    }

    void find_maltparser(std::string parser_dirname) {
        if (access(parser_dirname.c_str(), F_OK) != -1) {
            // path exists
            malt_jars.push_back(parser_dirname);
        } else {
            char* env = std::getenv("MALT_PARSER");
            if (env != NULL) {
                malt_jars.push_back(std::string(env));
            }
        }
    }

    void find_malt_model(std::string model_filename) {
        if (model_filename.empty()) {
            model = "malt_temp.mco";
        } else if (access(model_filename.c_str(), F_OK) != -1) {
            model = model_filename;
        } else {
            char* env = std::getenv("MALT_MODEL");
            if (env != NULL) {
                model = std::string(env);
            }
        }
    }

    std::vector<std::string> generate_malt_command(std::string inputfilename, std::string outputfilename, std::string mode) {
        std::vector<std::string> cmd = {"java"};
        cmd.insert(cmd.end(), additional_java_args.begin(), additional_java_args.end());
        std::string classpaths_separator = ":";
        std::string classpath = std::accumulate(malt_jars.begin(), malt_jars.end(), std::string(), [&](const std::string& a, const std::string& b) {
            return a + (a.length() > 0 ? classpaths_separator : "") + b;
        });
        cmd.push_back("-cp");
        cmd.push_back(classpath);
        cmd.push_back("org.maltparser.Malt");

        if (access(model.c_str(), F_OK) != -1) {
            cmd.push_back("-c");
            cmd.push_back(model.substr(model.find_last_of("/") + 1));
        } else {
            cmd.push_back("-c");
            cmd.push_back(model);
        }

        cmd.push_back("-i");
        cmd.push_back(inputfilename);
        if (mode == "parse") {
            cmd.push_back("-o");
            cmd.push_back(outputfilename);
        }
        cmd.push_back("-m");
        cmd.push_back(mode);
        return cmd;
    }

    int _execute(const std::vector<std::string>& cmd, bool verbose) {
        std::string command_str = std::accumulate(cmd.begin(), cmd.end(), std::string(), [](const std::string& a, const std::string& b) {
            return a + (a.length() > 0 ? " " : "") + b;
        });
        return system(command_str.c_str());
    }
};