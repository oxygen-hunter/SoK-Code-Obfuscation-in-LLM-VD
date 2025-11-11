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
    def __init__(self, keyword=None, feature=None, text=None, entity_1=None, comp_type=None, entity_2=None):
        self.entity_2 = entity_2
        self.text = text
        self.entity_1 = entity_1
        self.keyword = keyword
        self.feature = feature
        self.comp_type = comp_type

    def __repr__(self):
        return (
            'Comparison(text="{}", comp_type={}, entity_1="{}", entity_2="{}", '
            'feature="{}", keyword="{}")'
        ).format(
            self.text,
            self.comp_type,
            self.entity_1,
            self.entity_2,
            self.feature,
            self.keyword,
        )


class ComparativeSentencesCorpusReader(CorpusReader):

    CorpusView = StreamBackedCorpusView

    def __init__(
        self,
        root,
        fileids,
        word_tokenizer=WhitespaceTokenizer(),
        sent_tokenizer=None,
        encoding="utf8",
    ):
        CorpusReader.__init__(self, root, fileids, encoding)
        self._readme = "README.txt"
        self._word_tokenizer = word_tokenizer
        self._sent_tokenizer = sent_tokenizer

    def comparisons(self, fileids=None):
        if fileids is None:
            fileids = self._fileids
        elif isinstance(fileids, str):
            fileids = [fileids]
        return concat(
            [
                self.CorpusView(p, self._read_comparison_block, encoding=e)
                for (p, e, f) in self.abspaths(fileids, True, True)
            ]
        )

    def keywords(self, fileids=None):
        all_keywords = concat(
            [
                self.CorpusView(p, self._read_keyword_block, encoding=e)
                for (p, e, f) in self.abspaths(fileids, True, True)
            ]
        )

        return {k.lower() for k in all_keywords if k}

    def keywords_readme(self):
        k = []
        with self.open("listOfkeywords.txt") as f:
            raw = f.read()
        for l in raw.split("\n"):
            if not l or l.startswith("//"):
                continue
            k.append(l.strip())
        return k

    def sents(self, fileids=None):
        return concat(
            [
                self.CorpusView(p, self._read_sent_block, encoding=e)
                for (p, e, f) in self.abspaths(fileids, True, True)
            ]
        )

    def words(self, fileids=None):
        return concat(
            [
                self.CorpusView(p, self._read_word_block, encoding=e)
                for (p, e, f) in self.abspaths(fileids, True, True)
            ]
        )

    def _read_comparison_block(self, s):
        while True:
            l = s.readline()
            if not l:
                return []
            ct = re.findall(COMPARISON, l)
            if ct:
                gcs = re.findall(GRAD_COMPARISON, l)
                ngcs = re.findall(NON_GRAD_COMPARISON, l)
                ct = s.readline().strip()
                if self._word_tokenizer:
                    ct = self._word_tokenizer.tokenize(ct)
                s.readline()
                cb = []
                if gcs:
                    for c in gcs:
                        ct = int(re.match(r"<cs-(\d)>", c).group(1))
                        co = Comparison(text=ct, comp_type=ct)
                        l = s.readline()
                        ef = ENTITIES_FEATS.findall(l)
                        if ef:
                            for (c, e) in ef:
                                if c == "1":
                                    co.entity_1 = e.strip()
                                elif c == "2":
                                    co.entity_2 = e.strip()
                                elif c == "3":
                                    co.feature = e.strip()
                        k = KEYWORD.findall(l)
                        if k:
                            co.keyword = k[0]
                        cb.append(co)
                if ngcs:
                    for c in ngcs:
                        ct = int(re.match(r"<cs-(\d)>", c).group(1))
                        co = Comparison(text=ct, comp_type=ct)
                        cb.append(co)
                return cb

    def _read_keyword_block(self, s):
        k = []
        for c in self._read_comparison_block(s):
            k.append(c.keyword)
        return k

    def _read_sent_block(self, s):
        while True:
            l = s.readline()
            if re.match(STARS, l):
                while True:
                    l = s.readline()
                    if re.match(STARS, l):
                        break
                continue
            if (
                not re.findall(COMPARISON, l)
                and not ENTITIES_FEATS.findall(l)
                and not re.findall(CLOSE_COMPARISON, l)
            ):
                if self._sent_tokenizer:
                    return [
                        self._word_tokenizer.tokenize(s)
                        for s in self._sent_tokenizer.tokenize(l)
                    ]
                else:
                    return [self._word_tokenizer.tokenize(l)]

    def _read_word_block(self, s):
        w = []
        for st in self._read_sent_block(s):
            w.extend(st)
        return w