import re
from .scanner import ScannerParser
from .util import PUNCTUATION, ESCAPE_TEXT, escape_url, unikey

def getHTMLTagName():
    return r'[A-Za-z][A-Za-z0-9-]*'

def getHTMLAttributes():
    return (
        r'(?:\s+[A-Za-z_:][A-Za-z0-9_.:-]*'
        r'(?:\s*=\s*(?:[^ "\'=<>`]+|\'[^\']*?\'|"[^\"]*?"))?)*'
    )

def getEscapeChar():
    return re.compile(r'\\([' + PUNCTUATION + r'])')

def getLinkText():
    return r'(?:\[(?:\\.|[^\[\]\\])*\]|\\.|`[^`]*`|[^\[\]\\`])*?'

def getLinkLabel():
    return r'(?:[^\\\[\]]|' + ESCAPE_TEXT + r'){0,1000}'

class InlineParser(ScannerParser):
    ESCAPE = ESCAPE_TEXT

    def getAutoLink(self):
        return (
            r'(?<!\\)(?:\\\\)*<([A-Za-z][A-Za-z0-9+.-]{1,31}:'
            r"[^ <>]*?|[A-Za-z0-9.!#$%&'*+/=?^_`{|}~-]+@[A-Za-z0-9]"
            r'(?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?'
            r'(?:\.[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)*)>'
        )

    def getStdLink(self):
        return (
            r'!?\[(' + getLinkText() + r')\]\(\s*'
            r'(<(?:\\[<>]?|[^\s<>\\])*>|'
            r'(?:\\[()]?|\([^\s\x00-\x1f\\]*\)|[^\s\x00-\x1f()\\])*?)'
            r'(?:\s+('
            r'''"(?:\\"?|[^"\\])*"|'(?:\\'?|[^'\\])*'|\((?:\\\)?|[^)\\])*\)'''
            r'))?\s*\)'
        )

    def getRefLink(self):
        return (
            r'!?\[(' + getLinkText() + r')\]'
            r'\[(' + getLinkLabel() + r')\]'
        )

    def getRefLink2(self):
        return r'!?\[(' + getLinkLabel() + r')\]'

    def getAsteriskEmphasis(self):
        return (
            r'(\*{1,2})(?=[^\s*])('
            r'(?:(?:(?<!\\)(?:\\\\)*\*)|[^*])+'
            r')(?<!\\)\1'
        )

    def getUnderscoreEmphasis(self):
        return (
            r'\b(_{1,2})(?=[^\s_])([\s\S]*?'
            r'(?:' + ESCAPE_TEXT + r'|[^\s_]))\1'
            r'(?!_|[^\s' + PUNCTUATION + r'])\b'
        )

    def getCodespan(self):
        return (
            r'(?<!\\|`)(?:\\\\)*(`+)(?!`)([\s\S]+?)(?<!`)\1(?!`)'
        )

    def getLinebreak(self):
        return r'(?:\\| {2,})\n(?!\s*$)'

    def getInlineHtml(self):
        return (
            r'(?<!\\)<' + getHTMLTagName() + getHTMLAttributes() + r'\s*/?>|'  # open tag
            r'(?<!\\)</' + getHTMLTagName() + r'\s*>|'  # close tag
            r'(?<!\\)<!--(?!>|->)(?:(?!--)[\s\S])+?(?<!-)-->|'  # comment
            r'(?<!\\)<\?[\s\S]+?\?>|'
            r'(?<!\\)<![A-Z][\s\S]+?>|'  # doctype
            r'(?<!\\)<!\[CDATA[\s\S]+?\]\]>'  # cdata
        )

    def getRuleNames(self):
        return (
            'escape', 'inline_html', 'auto_link',
            'std_link', 'ref_link', 'ref_link2',
            'asterisk_emphasis', 'underscore_emphasis',
            'codespan', 'linebreak',
        )

    def __init__(self, renderer, hard_wrap=False):
        super(InlineParser, self).__init__()
        if hard_wrap:
            self.LINEBREAK = r' *\n(?!\s*$)'
        self.renderer = renderer
        rules = list(self.getRuleNames())
        rules.remove('ref_link')
        rules.remove('ref_link2')
        self.ref_link_rules = rules

    def parse_escape(self, m, state):
        text = m.group(0)[1:]
        return 'text', text

    def parse_auto_link(self, m, state):
        if state.get('_in_link'):
            return 'text', m.group(0)

        text = m.group(1)
        schemes = ('mailto:', 'http://', 'https://')
        if '@' in text and not text.lower().startswith(schemes):
            link = 'mailto:' + text
        else:
            link = text
        return 'link', escape_url(link), text

    def parse_std_link(self, m, state):
        line = m.group(0)
        text = m.group(1)
        link = getEscapeChar().sub(r'\1', m.group(2))
        if link.startswith('<') and link.endswith('>'):
            link = link[1:-1]

        title = m.group(3)
        if title:
            title = getEscapeChar().sub(r'\1', title[1:-1])

        if line[0] == '!':
            return 'image', escape_url(link), text, title

        return self.tokenize_link(line, link, text, title, state)

    def parse_ref_link(self, m, state):
        line = m.group(0)
        text = m.group(1)
        key = unikey(m.group(2) or text)
        def_links = state.get('def_links')
        if not def_links or key not in def_links:
            return list(self._scan(line, state, self.ref_link_rules))

        link, title = def_links.get(key)
        link = getEscapeChar().sub(r'\1', link)
        if title:
            title = getEscapeChar().sub(r'\1', title)

        if line[0] == '!':
            return 'image', escape_url(link), text, title

        return self.tokenize_link(line, link, text, title, state)

    def parse_ref_link2(self, m, state):
        return self.parse_ref_link(m, state)

    def tokenize_link(self, line, link, text, title, state):
        if state.get('_in_link'):
            return 'text', line
        state['_in_link'] = True
        text = self.render(text, state)
        state['_in_link'] = False
        return 'link', escape_url(link), text, title

    def parse_asterisk_emphasis(self, m, state):
        return self.tokenize_emphasis(m, state)

    def parse_underscore_emphasis(self, m, state):
        return self.tokenize_emphasis(m, state)

    def tokenize_emphasis(self, m, state):
        marker = m.group(1)
        text = m.group(2)
        if len(marker) == 1:
            return 'emphasis', self.render(text, state)
        return 'strong', self.render(text, state)

    def parse_codespan(self, m, state):
        code = re.sub(r'[ \n]+', ' ', m.group(2).strip())
        return 'codespan', code

    def parse_linebreak(self, m, state):
        return 'linebreak',

    def parse_inline_html(self, m, state):
        html = m.group(0)
        return 'inline_html', html

    def parse_text(self, text, state):
        return 'text', text

    def parse(self, s, state, rules=None):
        if rules is None:
            rules = self.rules

        tokens = (
            self.renderer._get_method(t[0])(*t[1:])
            for t in self._scan(s, state, rules)
        )
        return tokens

    def render(self, s, state, rules=None):
        tokens = self.parse(s, state, rules)
        return self.renderer.finalize(tokens)

    def __call__(self, s, state):
        return self.render(s, state)