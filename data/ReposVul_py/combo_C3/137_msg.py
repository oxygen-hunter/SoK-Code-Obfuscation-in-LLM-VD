import time
import hashlib
from datetime import datetime

try:
    from flask import request, Response
except ImportError:
    request, Response = None, None

try:
    from django.http import HttpResponse, HttpResponseForbidden, HttpResponseNotAllowed
except Exception:
    HttpResponse, HttpResponseForbidden, HttpResponseNotAllowed = None, None, None

try:
    from lxml import etree
except ImportError:
    from xml.etree import cElementTree as etree
except ImportError:
    from xml.etree import ElementTree as etree

class WeixinError(Exception):
    pass

class WeixinMsgError(WeixinError):
    def __init__(self, msg):
        super(WeixinMsgError, self).__init__(msg)

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def load_program(self, program):
        self.instructions = program

    def run(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            self.execute(instr)

    def execute(self, instr):
        op = instr[0]
        if op == "PUSH":
            self.stack.append(instr[1])
        elif op == "POP":
            self.stack.pop()
        elif op == "LOAD":
            self.stack.append(self.stack[-1])
        elif op == "STORE":
            self.stack[-1] = self.stack.pop()
        elif op == "ADD":
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a + b)
        elif op == "SUB":
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a - b)
        elif op == "JMP":
            self.pc = instr[1] - 1
        elif op == "JZ":
            if self.stack.pop() == 0:
                self.pc = instr[1] - 1
        else:
            raise WeixinMsgError("Unknown instruction: " + op)
        self.pc += 1

class WeixinMsg:
    def __init__(self, token, sender=None, expires_in=0):
        self.token = token
        self.sender = sender
        self.expires_in = expires_in
        self._registry = dict()

    def validate(self, signature, timestamp, nonce):
        vm = VM()
        program = [
            ("PUSH", self.token),
            ("PUSH", str(timestamp)),
            ("PUSH", str(nonce)),
            ("LOAD", None),
            ("STORE", None),
            ("JMP", 7),
            ("PUSH", ""),
            ("ADD", None),
            ("PUSH", hashlib.sha1),
            ("LOAD", None),
            ("STORE", None),
            ("LOAD", None),
            ("PUSH", "utf-8"),
            ("LOAD", None),
            ("STORE", None),
            ("LOAD", None),
            ("PUSH", "hexdigest"),
            ("LOAD", None),
            ("STORE", None),
            ("PUSH", signature),
            ("SUB", None),
            ("PUSH", 0),
            ("JZ", 24),
            ("PUSH", False),
            ("POP", None),
            ("PUSH", True)
        ]
        vm.load_program(program)
        vm.run()
        return vm.stack.pop()

    def parse(self, content):
        vm = VM()
        program = [
            ("PUSH", {}),
            ("PUSH", etree.fromstring(content, parser=etree.XMLParser(resolve_entities=False))),
            ("LOAD", None),
            ("STORE", None),
            ("JMP", 6),
            ("PUSH", None),
            ("POP", None),
            ("LOAD", None),
            ("PUSH", {}),
            ("LOAD", None),
            ("STORE", None),
            ("LOAD", None),
            ("PUSH", "tag"),
            ("LOAD", None),
            ("STORE", None),
            ("LOAD", None),
            ("PUSH", "text"),
            ("LOAD", None),
            ("STORE", None),
            ("LOAD", None),
            ("PUSH", "format"),
            ("LOAD", None),
            ("STORE", None),
            ("LOAD", None),
            ("PUSH", "type"),
            ("LOAD", None),
            ("STORE", None),
            ("LOAD", None),
            ("PUSH", "parse_invalid_type"),
            ("LOAD", None),
            ("STORE", None),
            ("LOAD", None),
            ("PUSH", "update"),
            ("LOAD", None),
            ("STORE", None)
        ]
        vm.load_program(program)
        vm.run()
        return vm.stack.pop()

    def reply(self, username=None, type='text', sender=None, **kwargs):
        vm = VM()
        program = [
            ("PUSH", username),
            ("PUSH", sender or self.sender),
            ("PUSH", type),
            ("PUSH", kwargs),
            ("LOAD", None),
            ("STORE", None),
            ("JMP", 7),
            ("PUSH", None),
            ("POP", None),
            ("LOAD", None),
            ("PUSH", "text_reply"),
            ("LOAD", None),
            ("STORE", None),
            ("LOAD", None),
            ("PUSH", "music_reply"),
            ("LOAD", None),
            ("STORE", None),
            ("LOAD", None),
            ("PUSH", "news_reply"),
            ("LOAD", None),
            ("STORE", None),
            ("LOAD", None),
            ("PUSH", "transfer_customer_service_reply"),
            ("LOAD", None),
            ("STORE", None),
            ("LOAD", None),
            ("PUSH", "image_reply"),
            ("LOAD", None),
            ("STORE", None),
            ("LOAD", None),
            ("PUSH", "voice_reply"),
            ("LOAD", None),
            ("STORE", None),
            ("LOAD", None),
            ("PUSH", "video_reply"),
            ("LOAD", None),
            ("STORE", None)
        ]
        vm.load_program(program)
        vm.run()
        return vm.stack.pop()

def text_reply(username, sender, content):
    shared = _shared_reply(username, sender, 'text')
    template = '<xml>%s<Content><![CDATA[%s]]></Content></xml>'
    return template % (shared, content)

def music_reply(username, sender, **kwargs):
    kwargs['shared'] = _shared_reply(username, sender, 'music')
    template = (
        '<xml>'
        '%(shared)s'
        '<Music>'
        '<Title><![CDATA[%(title)s]]></Title>'
        '<Description><![CDATA[%(description)s]]></Description>'
        '<MusicUrl><![CDATA[%(music_url)s]]></MusicUrl>'
        '<HQMusicUrl><![CDATA[%(hq_music_url)s]]></HQMusicUrl>'
        '</Music>'
        '</xml>'
    )
    return template % kwargs

def news_reply(username, sender, *items):
    item_template = (
        '<item>'
        '<Title><![CDATA[%(title)s]]></Title>'
        '<Description><![CDATA[%(description)s]]></Description>'
        '<PicUrl><![CDATA[%(picurl)s]]></PicUrl>'
        '<Url><![CDATA[%(url)s]]></Url>'
        '</item>'
    )
    articles = [item_template % o for o in items]
    template = (
        '<xml>'
        '%(shared)s'
        '<ArticleCount>%(count)d</ArticleCount>'
        '<Articles>%(articles)s</Articles>'
        '</xml>'
    )
    dct = {
        'shared': _shared_reply(username, sender, 'news'),
        'count': len(items),
        'articles': ''.join(articles)
    }
    return template % dct

def transfer_customer_service_reply(username, sender, service_account):
    template = (
        '<xml>%(shared)s'
        '%(transfer_info)s</xml>')
    transfer_info = ''
    if service_account:
        transfer_info = (
            '<TransInfo>'
            '<KfAccount>![CDATA[%s]]</KfAccount>'
            '</TransInfo>') % service_account
    dct = {
        'shared': _shared_reply(username, sender,
                                type='transfer_customer_service'),
        'transfer_info': transfer_info
    }
    return template % dct

def image_reply(username, sender, media_id):
    shared = _shared_reply(username, sender, 'image')
    template = '<xml>%s<Image><MediaId><![CDATA[%s]]></MediaId></Image></xml>'
    return template % (shared, media_id)

def voice_reply(username, sender, media_id):
    shared = _shared_reply(username, sender, 'voice')
    template = '<xml>%s<Voice><MediaId><![CDATA[%s]]></MediaId></Voice></xml>'
    return template % (shared, media_id)

def video_reply(username, sender, **kwargs):
    kwargs['shared'] = _shared_reply(username, sender, 'video')
    template = (
        '<xml>'
        '%(shared)s'
        '<Video>'
        '<MediaId><![CDATA[%(media_id)s]]></MediaId>'
        '<Title><![CDATA[%(title)s]]></Title>'
        '<Description><![CDATA[%(description)s]]></Description>'
        '</Video>'
        '</xml>'
    )
    return template % kwargs

def _shared_reply(username, sender, type):
    dct = {
        'username': username,
        'sender': sender,
        'type': type,
        'timestamp': int(time.time()),
    }
    template = (
        '<ToUserName><![CDATA[%(username)s]]></ToUserName>'
        '<FromUserName><![CDATA[%(sender)s]]></FromUserName>'
        '<CreateTime>%(timestamp)d</CreateTime>'
        '<MsgType><![CDATA[%(type)s]]></MsgType>'
    )
    return template % dct