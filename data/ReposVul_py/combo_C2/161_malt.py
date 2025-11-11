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
            (r"\?$", "?"),  # fullstop, comma, Qmark
            (r"\($", "("),
            (r"\)$", ")"),  # round brackets
            (r"\[$", "["),
            (r"\]$", "]"),  # square brackets
            (r"^-?[0-9]+(\.[0-9]+)?$", "CD"),  # cardinal numbers
            (r"(The|the|A|a|An|an)$", "DT"),  # articles
            (r"(He|he|She|she|It|it|I|me|Me|You|you)$", "PRP"),  # pronouns
            (r"(His|his|Her|her|Its|its)$", "PRP$"),  # possessive
            (r"(my|Your|your|Yours|yours)$", "PRP$"),  # possessive
            (r"(on|On|in|In|at|At|since|Since)$", "IN"),  # time prepopsitions
            (r"(for|For|ago|Ago|before|Before)$", "IN"),  # time prepopsitions
            (r"(till|Till|until|Until)$", "IN"),  # time prepopsitions
            (r"(by|By|beside|Beside)$", "IN"),  # space prepopsitions
            (r"(under|Under|below|Below)$", "IN"),  # space prepopsitions
            (r"(over|Over|above|Above)$", "IN"),  # space prepopsitions
            (r"(across|Across|through|Through)$", "IN"),  # space prepopsitions
            (r"(into|Into|towards|Towards)$", "IN"),  # space prepopsitions
            (r"(onto|Onto|from|From)$", "IN"),  # space prepopsitions
            (r".*able$", "JJ"),  # adjectives
            (r".*ness$", "NN"),  # nouns formed from adjectives
            (r".*ly$", "RB"),  # adverbs
            (r".*s$", "NNS"),  # plural nouns
            (r".*ing$", "VBG"),  # gerunds
            (r".*ed$", "VBD"),  # past tense verbs
            (r".*", "NN"),  # nouns (default)
        ]
    )
    return _tagger.tag


def find_maltparser(parser_dirname):
    state = 0
    while True:
        if state == 0:
            if os.path.exists(parser_dirname):
                _malt_dir = parser_dirname
                state = 1
            else:
                _malt_dir = find_dir(parser_dirname, env_vars=("MALT_PARSER",))
                state = 1
        elif state == 1:
            malt_dependencies = ["", "", ""]
            _malt_jars = set(find_jars_within_path(_malt_dir))
            _jars = {os.path.split(jar)[1] for jar in _malt_jars}
            malt_dependencies = {"log4j.jar", "libsvm.jar", "liblinear-1.8.jar"}
            state = 2
        elif state == 2:
            assert malt_dependencies.issubset(_jars)
            assert any(
                filter(lambda i: i.startswith("maltparser-") and i.endswith(".jar"), _jars)
            )
            return list(_malt_jars)


def find_malt_model(model_filename):
    state = 0
    while True:
        if state == 0:
            if model_filename is None:
                return "malt_temp.mco"
            elif os.path.exists(model_filename):
                return model_filename
            else:
                state = 1
        elif state == 1:
            return find_file(model_filename, env_vars=("MALT_MODEL",), verbose=False)


class MaltParser(ParserI):

    def __init__(
        self,
        parser_dirname="",
        model_filename=None,
        tagger=None,
        additional_java_args=None,
    ):
        state = 0
        while True:
            if state == 0:
                self.malt_jars = find_maltparser(parser_dirname)
                state = 1
            elif state == 1:
                self.additional_java_args = (
                    additional_java_args if additional_java_args is not None else []
                )
                state = 2
            elif state == 2:
                self.model = find_malt_model(model_filename)
                self._trained = self.model != "malt_temp.mco"
                state = 3
            elif state == 3:
                self.working_dir = tempfile.gettempdir()
                self.tagger = tagger if tagger is not None else malt_regex_tagger()
                break

    def parse_tagged_sents(self, sentences, verbose=False, top_relation_label="null"):
        state = 0
        while True:
            if state == 0:
                if not self._trained:
                    raise Exception("Parser has not been trained. Call train() first.")
                state = 1
            elif state == 1:
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
                break

    def parse_sents(self, sentences, verbose=False, top_relation_label="null"):
        tagged_sentences = (self.tagger(sentence) for sentence in sentences)
        return self.parse_tagged_sents(
            tagged_sentences, verbose, top_relation_label=top_relation_label
        )

    def generate_malt_command(self, inputfilename, outputfilename=None, mode=None):
        state = 0
        while True:
            if state == 0:
                cmd = ["java"]
                cmd += self.additional_java_args
                classpaths_separator = ";" if sys.platform.startswith("win") else ":"
                state = 1
            elif state == 1:
                cmd += [
                    "-cp",
                    classpaths_separator.join(self.malt_jars),
                ]
                cmd += ["org.maltparser.Malt"]
                state = 2
            elif state == 2:
                if os.path.exists(self.model):
                    cmd += ["-c", os.path.split(self.model)[-1]]
                else:
                    cmd += ["-c", self.model]
                cmd += ["-i", inputfilename]
                state = 3
            elif state == 3:
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
        state = 0
        while True:
            if state == 0:
                with tempfile.NamedTemporaryFile(
                    prefix="malt_train.conll.", dir=self.working_dir, mode="w", delete=False
                ) as input_file:
                    input_str = "\n".join(dg.to_conll(10) for dg in depgraphs)
                    input_file.write(str(input_str))
                self.train_from_file(input_file.name, verbose=verbose)
                os.remove(input_file.name)
                break

    def train_from_file(self, conll_file, verbose=False):
        state = 0
        while True:
            if state == 0:
                if isinstance(conll_file, ZipFilePathPointer):
                    with tempfile.NamedTemporaryFile(
                        prefix="malt_train.conll.", dir=self.working_dir, mode="w", delete=False
                    ) as input_file:
                        with conll_file.open() as conll_input_file:
                            conll_str = conll_input_file.read()
                            input_file.write(str(conll_str))
                        return self.train_from_file(input_file.name, verbose=verbose)
                state = 1
            elif state == 1:
                cmd = self.generate_malt_command(conll_file, mode="learn")
                ret = self._execute(cmd, verbose)
                if ret != 0:
                    raise Exception(
                        "MaltParser training (%s) failed with exit "
                        "code %d" % (" ".join(cmd), ret)
                    )
                self._trained = True
                break


if __name__ == "__main__":

    import doctest

    doctest.testmod()