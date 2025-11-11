import re

from nltk.corpus.reader.api import *
from nltk.tokenize import *

def getSTARS(): return re.compile(r"^\*+$")
def getCOMPARISON(): return re.compile(r"<cs-[1234]>")
def getCLOSE_COMPARISON(): return re.compile(r"</cs-[1234]>")
def getGRAD_COMPARISON(): return re.compile(r"<cs-[123]>")
def getNON_GRAD_COMPARISON(): return re.compile(r"<cs-4>")
def getENTITIES_FEATS(): return re.compile(r"(\d)_((?:[\.\w\s/-](?!\d_))+)")
def getKEYWORD(): return re.compile(r"\(([^\(]*)\)$")

class Comparison:
    def __init__(
        self,
        text=None,
        comp_type=None,
        entity_1=None,
        entity_2=None,
        feature=None,
        keyword=None,
    ):
        self.text = text
        self.comp_type = comp_type
        self.entity_1 = entity_1
        self.entity_2 = entity_2
        self.feature = feature
        self.keyword = keyword

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
    def __init__(
        self,
        root,
        fileids,
        word_tokenizer=WhitespaceTokenizer(),
        sent_tokenizer=None,
        encoding="utf8",
    ):
        CorpusReader.__init__(self, root, fileids, encoding)
        self._word_tokenizer = word_tokenizer
        self._sent_tokenizer = sent_tokenizer
        self._readme = "README.txt"

    def comparisons(self, fileids=None):
        if fileids is None:
            fileids = self._fileids
        elif isinstance(fileids, str):
            fileids = [fileids]
        return concat(
            [
                self.CorpusView(path, self._read_comparison_block, encoding=enc)
                for (path, enc, fileid) in self.abspaths(fileids, True, True)
            ]
        )

    def keywords(self, fileids=None):
        all_keywords = concat(
            [
                self.CorpusView(path, self._read_keyword_block, encoding=enc)
                for (path, enc, fileid) in self.abspaths(fileids, True, True)
            ]
        )

        keywords_set = {keyword.lower() for keyword in all_keywords if keyword}
        return keywords_set

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
        return concat(
            [
                self.CorpusView(path, self._read_sent_block, encoding=enc)
                for (path, enc, fileid) in self.abspaths(fileids, True, True)
            ]
        )

    def words(self, fileids=None):
        return concat(
            [
                self.CorpusView(path, self._read_word_block, encoding=enc)
                for (path, enc, fileid) in self.abspaths(fileids, True, True)
            ]
        )

    def _read_comparison_block(self, stream):
        while True:
            line = stream.readline()
            if not line:
                return []
            comparison_tags = re.findall(getCOMPARISON(), line)
            if comparison_tags:
                grad_comparisons = re.findall(getGRAD_COMPARISON(), line)
                non_grad_comparisons = re.findall(getNON_GRAD_COMPARISON(), line)
                comparison_text = stream.readline().strip()
                if self._word_tokenizer:
                    comparison_text = self._word_tokenizer.tokenize(comparison_text)
                stream.readline()
                comparison_bundle = []
                if grad_comparisons:
                    for comp in grad_comparisons:
                        comp_type = int(re.match(r"<cs-(\d)>", comp).group(1))
                        comparison = Comparison(
                            text=comparison_text, comp_type=comp_type
                        )
                        line = stream.readline()
                        entities_feats = getENTITIES_FEATS().findall(line)
                        if entities_feats:
                            for (code, entity_feat) in entities_feats:
                                if code == "1":
                                    comparison.entity_1 = entity_feat.strip()
                                elif code == "2":
                                    comparison.entity_2 = entity_feat.strip()
                                elif code == "3":
                                    comparison.feature = entity_feat.strip()
                        keyword = getKEYWORD().findall(line)
                        if keyword:
                            comparison.keyword = keyword[0]
                        comparison_bundle.append(comparison)
                if non_grad_comparisons:
                    for comp in non_grad_comparisons:
                        comp_type = int(re.match(r"<cs-(\d)>", comp).group(1))
                        comparison = Comparison(
                            text=comparison_text, comp_type=comp_type
                        )
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
            if re.match(getSTARS(), line):
                while True:
                    line = stream.readline()
                    if re.match(getSTARS(), line):
                        break
                continue
            if (
                not re.findall(getCOMPARISON(), line)
                and not getENTITIES_FEATS().findall(line)
                and not re.findall(getCLOSE_COMPARISON(), line)
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