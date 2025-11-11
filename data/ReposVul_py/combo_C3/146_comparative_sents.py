import re
from nltk.corpus.reader.api import *
from nltk.tokenize import *

STARS = re.compile(r"^\*+$")
COMPARISON = re.compile(r"<cs-[1234]>")
CLOSE_COMPARISON = re.compile(r"</cs-[1234]>")
GRAD_COMPARISON = re.compile(r"<cs-[123]>")
NON_GRAD_COMPARISON = re.compile(r"<cs-4>")
ENTITIES_FEATS = re.compile(r"(\d)_((?:[\.\w\s/-](?!\d_))+)")
KEYWORD = re.compile(r"\(([^\(]*)\)$")

class Comparison:
    def __init__(self, text=None, comp_type=None, entity_1=None, entity_2=None, feature=None, keyword=None):
        self.text = text
        self.comp_type = comp_type
        self.entity_1 = entity_1
        self.entity_2 = entity_2
        self.feature = feature
        self.keyword = keyword

    def __repr__(self):
        return ('Comparison(text="{}", comp_type={}, entity_1="{}", entity_2="{}", feature="{}", keyword="{}")').format(
            self.text, self.comp_type, self.entity_1, self.entity_2, self.feature, self.keyword
        )

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = []

    def load_program(self, program):
        self.program = program

    def run(self):
        while self.pc < len(self.program):
            instr = self.program[self.pc]
            self.execute(instr)
            self.pc += 1

    def execute(self, instr):
        op, *args = instr
        if op == 'PUSH':
            self.stack.append(args[0])
        elif op == 'POP':
            self.stack.pop()
        elif op == 'ADD':
            self.stack.append(self.stack.pop() + self.stack.pop())
        elif op == 'SUB':
            self.stack.append(self.stack.pop() - self.stack.pop())
        elif op == 'LOAD':
            self.stack.append(self.stack[args[0]])
        elif op == 'STORE':
            self.stack[args[0]] = self.stack.pop()
        elif op == 'JMP':
            self.pc = args[0] - 1
        elif op == 'JZ':
            if not self.stack.pop():
                self.pc = args[0] - 1

class ComparativeSentencesCorpusReader(CorpusReader):
    CorpusView = StreamBackedCorpusView

    def __init__(self, root, fileids, word_tokenizer=WhitespaceTokenizer(), sent_tokenizer=None, encoding="utf8"):
        CorpusReader.__init__(self, root, fileids, encoding)
        self._word_tokenizer = word_tokenizer
        self._sent_tokenizer = sent_tokenizer
        self._readme = "README.txt"

    def comparisons(self, fileids=None):
        vm = VM()
        vm.load_program([
            ('PUSH', self._fileids if fileids is None else [fileids] if isinstance(fileids, str) else fileids),
            ('STORE', 0),
            ('PUSH', []),
            ('STORE', 1),
            ('LOAD', 0),
            ('PUSH', len(fileids) if fileids else 0),
            ('STORE', 2),
            ('PUSH', 0),
            ('STORE', 3),
            ('LOAD', 3),
            ('LOAD', 2),
            ('SUB',),
            ('JZ', 18),
            ('LOAD', 0),
            ('LOAD', 3),
            ('LOAD', 1),
            ('PUSH', self.CorpusView),
            ('PUSH', self._read_comparison_block),
            ('LOAD', 3),
            ('JMP', 9),
            ('PUSH', self.abspaths),
            ('LOAD', 0),
            ('PUSH', False),
            ('PUSH', True),
            ('CALL', 3),
            ('JMP', 1),
            ('LOAD', 1),
            ('RETURN',)
        ])
        vm.run()
        return vm.stack.pop()

    def keywords(self, fileids=None):
        vm = VM()
        vm.load_program([
            ('PUSH', self._fileids if fileids is None else [fileids] if isinstance(fileids, str) else fileids),
            ('STORE', 0),
            ('PUSH', []),
            ('STORE', 1),
            ('LOAD', 0),
            ('PUSH', len(fileids) if fileids else 0),
            ('STORE', 2),
            ('PUSH', 0),
            ('STORE', 3),
            ('LOAD', 3),
            ('LOAD', 2),
            ('SUB',),
            ('JZ', 18),
            ('LOAD', 0),
            ('LOAD', 3),
            ('LOAD', 1),
            ('PUSH', self.CorpusView),
            ('PUSH', self._read_keyword_block),
            ('LOAD', 3),
            ('JMP', 9),
            ('PUSH', self.abspaths),
            ('LOAD', 0),
            ('PUSH', False),
            ('PUSH', True),
            ('CALL', 3),
            ('JMP', 1),
            ('LOAD', 1),
            ('RETURN',)
        ])
        vm.run()
        all_keywords = vm.stack.pop()
        return {keyword.lower() for keyword in all_keywords if keyword}

    def keywords_readme(self):
        keywords = []
        with self.open("listOfkeywords.txt") as fp:
            raw_text = fp.read()
        for line in raw_text.split("\n"):
            if not line or line.startswith("//"):
                continue
            keywords.append(line.strip())
        return keywords

    def sents(self, fileids=None):
        vm = VM()
        vm.load_program([
            ('PUSH', self._fileids if fileids is None else [fileids] if isinstance(fileids, str) else fileids),
            ('STORE', 0),
            ('PUSH', []),
            ('STORE', 1),
            ('LOAD', 0),
            ('PUSH', len(fileids) if fileids else 0),
            ('STORE', 2),
            ('PUSH', 0),
            ('STORE', 3),
            ('LOAD', 3),
            ('LOAD', 2),
            ('SUB',),
            ('JZ', 18),
            ('LOAD', 0),
            ('LOAD', 3),
            ('LOAD', 1),
            ('PUSH', self.CorpusView),
            ('PUSH', self._read_sent_block),
            ('LOAD', 3),
            ('JMP', 9),
            ('PUSH', self.abspaths),
            ('LOAD', 0),
            ('PUSH', False),
            ('PUSH', True),
            ('CALL', 3),
            ('JMP', 1),
            ('LOAD', 1),
            ('RETURN',)
        ])
        vm.run()
        return vm.stack.pop()

    def words(self, fileids=None):
        vm = VM()
        vm.load_program([
            ('PUSH', self._fileids if fileids is None else [fileids] if isinstance(fileids, str) else fileids),
            ('STORE', 0),
            ('PUSH', []),
            ('STORE', 1),
            ('LOAD', 0),
            ('PUSH', len(fileids) if fileids else 0),
            ('STORE', 2),
            ('PUSH', 0),
            ('STORE', 3),
            ('LOAD', 3),
            ('LOAD', 2),
            ('SUB',),
            ('JZ', 18),
            ('LOAD', 0),
            ('LOAD', 3),
            ('LOAD', 1),
            ('PUSH', self.CorpusView),
            ('PUSH', self._read_word_block),
            ('LOAD', 3),
            ('JMP', 9),
            ('PUSH', self.abspaths),
            ('LOAD', 0),
            ('PUSH', False),
            ('PUSH', True),
            ('CALL', 3),
            ('JMP', 1),
            ('LOAD', 1),
            ('RETURN',)
        ])
        vm.run()
        return vm.stack.pop()

    def _read_comparison_block(self, stream):
        while True:
            line = stream.readline()
            if not line:
                return []
            comparison_tags = re.findall(COMPARISON, line)
            if comparison_tags:
                grad_comparisons = re.findall(GRAD_COMPARISON, line)
                non_grad_comparisons = re.findall(NON_GRAD_COMPARISON, line)
                comparison_text = stream.readline().strip()
                if self._word_tokenizer:
                    comparison_text = self._word_tokenizer.tokenize(comparison_text)
                stream.readline()
                comparison_bundle = []
                if grad_comparisons:
                    for comp in grad_comparisons:
                        comp_type = int(re.match(r"<cs-(\d)>", comp).group(1))
                        comparison = Comparison(text=comparison_text, comp_type=comp_type)
                        line = stream.readline()
                        entities_feats = ENTITIES_FEATS.findall(line)
                        if entities_feats:
                            for (code, entity_feat) in entities_feats:
                                if code == "1":
                                    comparison.entity_1 = entity_feat.strip()
                                elif code == "2":
                                    comparison.entity_2 = entity_feat.strip()
                                elif code == "3":
                                    comparison.feature = entity_feat.strip()
                        keyword = KEYWORD.findall(line)
                        if keyword:
                            comparison.keyword = keyword[0]
                        comparison_bundle.append(comparison)
                if non_grad_comparisons:
                    for comp in non_grad_comparisons:
                        comp_type = int(re.match(r"<cs-(\d)>", comp).group(1))
                        comparison = Comparison(text=comparison_text, comp_type=comp_type)
                        comparison_bundle.append(comparison)
                return comparison_bundle

    def _read_keyword_block(self, stream):
        keywords = []
        for comparison in self._read_comparison_block(stream):
            keywords.append(comparison.keyword)
        return keywords

    def _read_sent_block(self, stream):
        while True:
            line = stream.readline()
            if re.match(STARS, line):
                while True:
                    line = stream.readline()
                    if re.match(STARS, line):
                        break
                continue
            if (
                not re.findall(COMPARISON, line)
                and not ENTITIES_FEATS.findall(line)
                and not re.findall(CLOSE_COMPARISON, line)
            ):
                if self._sent_tokenizer:
                    return [
                        self._word_tokenizer.tokenize(sent)
                        for sent in self._sent_tokenizer.tokenize(line)
                    ]
                else:
                    return [self._word_tokenizer.tokenize(line)]

    def _read_word_block(self, stream):
        words = []
        for sent in self._read_sent_block(stream):
            words.extend(sent)
        return words