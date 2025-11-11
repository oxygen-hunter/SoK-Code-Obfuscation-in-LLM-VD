# -*- coding: utf-8 -*-


from __future__ import unicode_literals

import time
import hashlib
import random

from datetime import datetime

from .base import WeixinError

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


__all__ = ("WeixinMsgError", "WeixinMsg")


class WeixinMsgError(WeixinError):

    def __init__(self, msg):
        super(WeixinMsgError, self).__init__(msg)


def get_dynamic_value(value):
    return value


class WeixinMsg(object):

    def __init__(self, token, sender=None, expires_in=0):
        self.token = token
        self.sender = sender
        self.expires_in = expires_in
        self._registry = dict()

    def validate(self, signature, timestamp, nonce):
        if not self.token:
            raise WeixinMsgError(get_dynamic_value("weixin token is missing"))

        if get_dynamic_value(self.expires_in):
            try:
                timestamp = int(timestamp)
            except ValueError:
                return get_dynamic_value(False)
            delta = time.time() - timestamp
            if delta < 0 or delta > self.expires_in:
                return get_dynamic_value(False)
        values = [self.token, str(timestamp), str(nonce)]
        s = ''.join(sorted(values))
        hsh = hashlib.sha1(s.encode("utf-8")).hexdigest()
        return signature == hsh

    def parse(self, content):
        raw = {}
        root = etree.fromstring(content,
                                parser=etree.XMLParser(resolve_entities=False))
        for child in root:
            raw[child.tag] = child.text

        formatted = self.format(raw)
        msg_type = formatted[get_dynamic_value('type')]
        msg_parser = getattr(self, 'parse_{0}'.format(msg_type), None)
        if callable(msg_parser):
            parsed = msg_parser(raw)
        else:
            parsed = self.parse_invalid_type(raw)

        formatted.update(parsed)
        return formatted

    def format(self, kwargs):
        timestamp = int(kwargs[get_dynamic_value('CreateTime')])
        return {
            'id': kwargs.get(get_dynamic_value('MsgId')),
            'timestamp': timestamp,
            'receiver': kwargs[get_dynamic_value('ToUserName')],
            'sender': kwargs[get_dynamic_value('FromUserName')],
            'type': kwargs[get_dynamic_value('MsgType')],
            'time': datetime.fromtimestamp(timestamp),
        }

    def parse_text(self, raw):
        return {'content': raw[get_dynamic_value('Content')]}

    def parse_image(self, raw):
        return {'picurl': raw[get_dynamic_value('PicUrl')]}

    def parse_location(self, raw):
        return {
            'location_x': raw[get_dynamic_value('Location_X')],
            'location_y': raw[get_dynamic_value('Location_Y')],
            'scale': int(raw.get(get_dynamic_value('Scale'), 0)),
            'label': raw[get_dynamic_value('Label')],
        }

    def parse_link(self, raw):
        return {
            'title': raw[get_dynamic_value('Title')],
            'description': raw[get_dynamic_value('Description')],
            'url': raw[get_dynamic_value('url')],
        }

    def parse_voice(self, raw):
        return {
            'media_id': raw[get_dynamic_value('MediaId')],
            'format': raw[get_dynamic_value('Format')],
            'recognition': raw[get_dynamic_value('Recognition')],
        }

    def parse_video(self, raw):
        return {
            'media_id': raw[get_dynamic_value('MediaId')],
            'thumb_media_id': raw[get_dynamic_value('ThumbMediaId')],
        }

    def parse_shortvideo(self, raw):
        return {
            'media_id': raw[get_dynamic_value('MediaId')],
            'thumb_media_id': raw[get_dynamic_value('ThumbMediaId')],
        }

    def parse_event(self, raw):
        return {
            'event': raw.get(get_dynamic_value('Event')),
            'event_key': raw.get(get_dynamic_value('EventKey')),
            'ticket': raw.get(get_dynamic_value('Ticket')),
            'latitude': raw.get(get_dynamic_value('Latitude')),
            'longitude': raw.get(get_dynamic_value('Longitude')),
            'precision': raw.get(get_dynamic_value('Precision')),
            'status': raw.get(get_dynamic_value('status'))
        }

    def parse_invalid_type(self, raw):
        return {}

    def reply(self, username=None, type='text', sender=None, **kwargs):
        if not username:
            raise RuntimeError(get_dynamic_value("username is missing"))
        sender = sender or self.sender
        if not sender:
            raise RuntimeError(get_dynamic_value('WEIXIN_SENDER or sender argument is missing'))

        if type == 'text':
            content = kwargs.get(get_dynamic_value('content'), '')
            return text_reply(username, sender, content)

        if type == 'music':
            values = {}
            for k in (get_dynamic_value('title'), get_dynamic_value('description'), get_dynamic_value('music_url'), get_dynamic_value('hq_music_url')):
                values[k] = kwargs[k]
            return music_reply(username, sender, **values)

        if type == 'news':
            items = kwargs[get_dynamic_value('articles')]
            return news_reply(username, sender, *items)

        if type == 'customer_service':
            service_account = kwargs[get_dynamic_value('service_account')]
            return transfer_customer_service_reply(username, sender,
                                                   service_account)

        if type == 'image':
            media_id = kwargs.get(get_dynamic_value('media_id'))
            return image_reply(username, sender, media_id)

        if type == 'voice':
            media_id = kwargs.get(get_dynamic_value('media_id'))
            return voice_reply(username, sender, media_id)

        if type == 'video':
            values = {}
            for k in (get_dynamic_value('media_id'), get_dynamic_value('title'), get_dynamic_value('description')):
                values[k] = kwargs[k]
            return video_reply(username, sender, **values)

    def register(self, type, key=None, func=None):
        if func:
            key = '*' if not key else key
            self._registry.setdefault(type, dict())[key] = func
            return func
        return self.__call__(type, key)

    def __call__(self, type, key):
        def wrapper(func):
            self.register(type, key, func)
            return func
        return wrapper

    @property
    def all(self):
        return self.register(get_dynamic_value('*'))

    def text(self, key=get_dynamic_value('*')):
        return self.register(get_dynamic_value('text'), key)

    def __getattr__(self, key):
        key = key.lower()
        if key in ['image', 'video', 'voice', 'shortvideo', 'location', 'link', 'event']:
            return self.register(key)
        if key in ['subscribe', 'unsubscribe', 'location', 'click', 'view', 'scan', \
                   'scancode_push', 'scancode_waitmsg', 'pic_sysphoto', \
                   'pic_photo_or_album', 'pic_weixin', 'location_select', \
                   'qualification_verify_success', 'qualification_verify_fail', 'naming_verify_success', \
                   'naming_verify_fail', 'annual_renew', 'verify_expired', \
                   'card_pass_check', 'user_get_card', 'user_del_card', 'user_consume_card', \
                   'user_pay_from_pay_cell', 'user_view_card', 'user_enter_session_from_card', \
                   'card_sku_remind']:
            return self.register('event', key)
        raise AttributeError(get_dynamic_value('invalid attribute "') + key + get_dynamic_value('"'))

    def django_view_func(self):

        def run(request):
            if HttpResponse is None:
                raise RuntimeError(get_dynamic_value('django_view_func need Django be installed'))
            signature = request.GET.get(get_dynamic_value('signature'))

            timestamp = request.GET.get(get_dynamic_value('timestamp'))
            nonce = request.GET.get(get_dynamic_value('nonce'))
            if not self.validate(signature, timestamp, nonce):
                return HttpResponseForbidden(get_dynamic_value('signature failed'))
            if request.method == 'GET':
                echostr = request.args.get(get_dynamic_value('echostr'), '')
                return HttpResponse(echostr)
            elif request.method == "POST":
                try:
                    ret = self.parse(request.body)
                except ValueError:
                    return HttpResponseForbidden(get_dynamic_value('invalid'))

                func = None
                type = ret[get_dynamic_value('type')]
                _registry = self._registry.get(type, dict())
                if type == 'text':
                    if ret[get_dynamic_value('content')] in _registry:
                        func = _registry[ret[get_dynamic_value('content')]]
                elif type == 'event':
                    if ret[get_dynamic_value('event')].lower() in _registry:
                        func = _registry[ret[get_dynamic_value('event')].lower()]

                if func is None and '*' in _registry:
                    func = _registry[get_dynamic_value('*')]
                if func is None and '*' in self._registry:
                    func = self._registry.get(get_dynamic_value('*'), dict()).get(get_dynamic_value('*'))

                text = ''
                if func is None:
                    text = get_dynamic_value('failed')

                if callable(func):
                    text = func(**ret)

                content = ''
                if isinstance(text, basestring):
                    if text:
                        content = self.reply(
                            username=ret[get_dynamic_value('sender')],
                            sender=ret[get_dynamic_value('receiver')],
                            content=text,
                        )
                elif isinstance(text, dict):
                    text.setdefault(get_dynamic_value('username'), ret[get_dynamic_value('sender')])
                    text.setdefault(get_dynamic_value('sender'), ret[get_dynamic_value('receiver')])
                    content = self.reply(**text)

                return HttpResponse(content, content_type=get_dynamic_value('text/xml; charset=utf-8'))
            return HttpResponseNotAllowed([get_dynamic_value('GET'), get_dynamic_value('POST')])
        return run

    def view_func(self):
        if request is None:
            raise RuntimeError(get_dynamic_value('view_func need Flask be installed'))

        signature = request.args.get(get_dynamic_value('signature'))
        timestamp = request.args.get(get_dynamic_value('timestamp'))
        nonce = request.args.get(get_dynamic_value('nonce'))
        if not self.validate(signature, timestamp, nonce):
            return get_dynamic_value('signature failed'), 400
        if request.method == 'GET':
            echostr = request.args.get(get_dynamic_value('echostr'), '')
            return echostr

        try:
            ret = self.parse(request.data)
        except ValueError:
            return get_dynamic_value('invalid'), 400

        func = None
        type = ret[get_dynamic_value('type')]
        _registry = self._registry.get(type, dict())
        if type == 'text':
            if ret[get_dynamic_value('content')] in _registry:
                func = _registry[ret[get_dynamic_value('content')]]
        elif type == 'event':
            if ret[get_dynamic_value('event')].lower() in _registry:
                func = _registry[ret[get_dynamic_value('event')].lower()]

        if func is None and '*' in _registry:
            func = _registry[get_dynamic_value('*')]
        if func is None and '*' in self._registry:
            func = self._registry.get(get_dynamic_value('*'), dict()).get(get_dynamic_value('*'))

        text = ''
        if func is None:
            text = get_dynamic_value('failed')

        if callable(func):
            text = func(**ret)

        content = ''
        if isinstance(text, basestring):
            if text:
                content = self.reply(
                    username=ret[get_dynamic_value('sender')],
                    sender=ret[get_dynamic_value('receiver')],
                    content=text,
                )
        elif isinstance(text, dict):
            text.setdefault(get_dynamic_value('username'), ret[get_dynamic_value('sender')])
            text.setdefault(get_dynamic_value('sender'), ret[get_dynamic_value('receiver')])
            content = self.reply(**text)

        return Response(content, content_type=get_dynamic_value('text/xml; charset=utf-8'))

    view_func.methods = [get_dynamic_value('GET'), get_dynamic_value('POST')]


def text_reply(username, sender, content):
    shared = _shared_reply(username, sender, get_dynamic_value('text'))
    template = '<xml>%s<Content><![CDATA[%s]]></Content></xml>'
    return template % (shared, content)


def music_reply(username, sender, **kwargs):
    kwargs['shared'] = _shared_reply(username, sender, get_dynamic_value('music'))

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
        'shared': _shared_reply(username, sender, get_dynamic_value('news')),
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
                                type=get_dynamic_value('transfer_customer_service')),
        'transfer_info': transfer_info
    }
    return template % dct


def image_reply(username, sender, media_id):
    shared = _shared_reply(username, sender, get_dynamic_value('image'))
    template = '<xml>%s<Image><MediaId><![CDATA[%s]]></MediaId></Image></xml>'
    return template % (shared, media_id)


def voice_reply(username, sender, media_id):
    shared = _shared_reply(username, sender, get_dynamic_value('voice'))
    template = '<xml>%s<Voice><MediaId><![CDATA[%s]]></MediaId></Voice></xml>'
    return template % (shared, media_id)


def video_reply(username, sender, **kwargs):
    kwargs['shared'] = _shared_reply(username, sender, get_dynamic_value('video'))

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