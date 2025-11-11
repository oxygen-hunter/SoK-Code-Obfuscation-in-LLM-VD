import re
from .scanner import ScannerParser
from .util import PUNCTUATION, ESCAPE_TEXT, escape_url, unikey

HTML_TAGNAME = r'[A-Za-z][A-Za-z0-9-]*'
HTML_ATTRIBUTES = (
    r'(?:\s+[A-Za-z_:][A-Za-z0-9_.:-]*'
    r'(?:\s*=\s*(?:[^ "\'=<>`]+|\'[^\']*?\'|"[^\"]*?"))?)*'
)
ESCAPE_CHAR = re.compile(r'\\([' + PUNCTUATION + r'])')
LINK_TEXT = r'(?:\[(?:\\.|[^\[\]\\])*\]|\\.|`[^`]*`|[^\[\]\\`])*?'
LINK_LABEL = r'(?:[^\\\[\]]|' + ESCAPE_TEXT + r'){0,1000}'

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = []
        self.state = {}

    def run(self, program, state):
        self.program = program
        self.state = state
        while self.pc < len(self.program):
            inst = self.program[self.pc]
            self.pc += 1
            opcode = inst[0]
            if opcode == 'PUSH':
                self.stack.append(inst[1])
            elif opcode == 'POP':
                self.stack.pop()
            elif opcode == 'CALL':
                func = inst[1]
                args = inst[2:]
                self.stack.append(func(*args))
            elif opcode == 'LOAD':
                self.stack.append(self.state[inst[1]])
            elif opcode == 'STORE':
                self.state[inst[1]] = self.stack.pop()
            elif opcode == 'JMP':
                self.pc = inst[1]
            elif opcode == 'JZ':
                if self.stack.pop() == 0:
                    self.pc = inst[1]

    def execute(self, program, state):
        self.run(program, state)
        return self.stack.pop()

class InlineParser(ScannerParser):
    def __init__(self, renderer, hard_wrap=False):
        super(InlineParser, self).__init__()
        if hard_wrap:
            self.LINEBREAK = r' *\n(?!\s*$)'
        self.renderer = renderer
        rules = list(self.RULE_NAMES)
        rules.remove('ref_link')
        rules.remove('ref_link2')
        self.ref_link_rules = rules

    def parse_escape(self, m, state):
        return [('PUSH', m.group(0)[1:]), ('CALL', lambda x: ('text', x))]

    def parse_auto_link(self, m, state):
        return [
            ('LOAD', '_in_link'),
            ('JZ', 5),
            ('PUSH', m.group(0)),
            ('CALL', lambda x: ('text', x)),
            ('JMP', 9),
            ('PUSH', m.group(1)),
            ('CALL', self._process_auto_link),
            ('CALL', lambda x: ('link', *x))
        ]

    def _process_auto_link(self, text):
        schemes = ('mailto:', 'http://', 'https://')
        if '@' in text and not text.lower().startswith(schemes):
            link = 'mailto:' + text
        else:
            link = text
        return escape_url(link), text

    def parse_std_link(self, m, state):
        return [
            ('PUSH', m.group(0)),
            ('PUSH', m.group(1)),
            ('PUSH', ESCAPE_CHAR.sub(r'\1', m.group(2))),
            ('PUSH', m.group(3)),
            ('CALL', self._process_std_link),
            ('CALL', lambda x: ('image', *x) if x[0] == '!' else ('link', *x))
        ]

    def _process_std_link(self, line, text, link, title):
        if link.startswith('<') and link.endswith('>'):
            link = link[1:-1]
        if title:
            title = ESCAPE_CHAR.sub(r'\1', title[1:-1])
        return line, escape_url(link), text, title

    def parse_ref_link(self, m, state):
        return [
            ('PUSH', m.group(0)),
            ('PUSH', m.group(1)),
            ('PUSH', unikey(m.group(2) or m.group(1))),
            ('LOAD', 'def_links'),
            ('CALL', self._process_ref_link),
            ('CALL', lambda x: list(self._scan(x[0], state, self.ref_link_rules)) if x[1] is None else ('image', *x[1]) if x[0][0] == '!' else ('link', *x[1]))
        ]

    def _process_ref_link(self, line, text, key, def_links):
        if not def_links or key not in def_links:
            return line, None
        link, title = def_links.get(key)
        link = ESCAPE_CHAR.sub(r'\1', link)
        if title:
            title = ESCAPE_CHAR.sub(r'\1', title)
        return line, (escape_url(link), text, title)

    def parse_ref_link2(self, m, state):
        return self.parse_ref_link(m, state)

    def tokenize_link(self, line, link, text, title, state):
        return [
            ('LOAD', '_in_link'),
            ('JZ', 4),
            ('PUSH', line),
            ('CALL', lambda x: ('text', x)),
            ('STORE', '_in_link'),
            ('PUSH', self.render(text, state)),
            ('STORE', '_in_link'),
            ('CALL', lambda x: ('link', *x))
        ]

    def parse_asterisk_emphasis(self, m, state):
        return [
            ('PUSH', m.group(1)),
            ('PUSH', m.group(2)),
            ('CALL', self._process_emphasis),
            ('CALL', lambda x: ('emphasis', x) if len(x[0]) == 1 else ('strong', x))
        ]

    def _process_emphasis(self, marker, text):
        return self.render(text, {})

    def parse_underscore_emphasis(self, m, state):
        return self.parse_asterisk_emphasis(m, state)

    def parse_codespan(self, m, state):
        return [
            ('PUSH', m.group(2)),
            ('CALL', lambda x: ('codespan', re.sub(r'[ \n]+', ' ', x.strip())))
        ]

    def parse_linebreak(self, m, state):
        return [('PUSH', 'linebreak')]

    def parse_inline_html(self, m, state):
        return [('PUSH', m.group(0)), ('CALL', lambda x: ('inline_html', x))]

    def parse_text(self, text, state):
        return [('PUSH', text), ('CALL', lambda x: ('text', x))]

    def parse(self, s, state, rules=None):
        if rules is None:
            rules = self.rules

        vm = VM()
        program = []
        for t in self._scan(s, state, rules):
            program.extend(t)
        return vm.execute(program, state)

    def render(self, s, state, rules=None):
        tokens = self.parse(s, state, rules)
        return self.renderer.finalize(tokens)

    def __call__(self, s, state):
        return self.render(s, state)