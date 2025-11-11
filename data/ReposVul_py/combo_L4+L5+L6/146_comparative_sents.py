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
        return self._process_readme(raw_text, keywords)

    def _process_readme(self, raw_text, keywords, index=0):
        if index >= len(raw_text.split("\n")):
            return keywords
        line = raw_text.split("\n")[index]
        if line and not line.startswith("//"):
            keywords.append(line.strip())
        return self._process_readme(raw_text, keywords, index + 1)

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
                self._process_grad_comparisons(stream, grad_comparisons, comparison_text, comparison_bundle)
            if non_grad_comparisons:
                self._process_non_grad_comparisons(non_grad_comparisons, comparison_text, comparison_bundle)
            return comparison_bundle
        return self._read_comparison_block(stream)

    def _process_grad_comparisons(self, stream, grad_comparisons, comparison_text, comparison_bundle, index=0):
        if index >= len(grad_comparisons):
            return
        comp = grad_comparisons[index]
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
        self._process_grad_comparisons(stream, grad_comparisons, comparison_text, comparison_bundle, index + 1)

    def _process_non_grad_comparisons(self, non_grad_comparisons, comparison_text, comparison_bundle, index=0):
        if index >= len(non_grad_comparisons):
            return
        comp = non_grad_comparisons[index]
        comp_type = int(re.match(r"<cs-(\d)>", comp).group(1))
        comparison = Comparison(text=comparison_text, comp_type=comp_type)
        comparison_bundle.append(comparison)
        self._process_non_grad_comparisons(non_grad_comparisons, comparison_text, comparison_bundle, index + 1)

    def _read_keyword_block(self, stream):
        keywords = []
        for comparison in self._read_comparison_block(stream):
            keywords.append(comparison.keyword)
        return keywords

    def _read_sent_block(self, stream):
        line = stream.readline()
        if re.match(STARS, line):
            return self._read_sent_block(stream)
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
        return self._read_sent_block(stream)

    def _read_word_block(self, stream):
        return self._process_words(stream, [])

    def _process_words(self, stream, words):
        sent = self._read_sent_block(stream)
        if not sent:
            return words
        words.extend(sent)
        return self._process_words(stream, words)