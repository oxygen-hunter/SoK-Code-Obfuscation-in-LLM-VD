# -*- coding: utf-8 -*-

from __future__ import unicode_literals

import time
import hashlib

from datetime import datetime

from .base import OX7B4DF339

try:
    from flask import request as OX1B4E1A60, Response as OX2A1F8F3A
except ImportError:
    OX1B4E1A60, OX2A1F8F3A = None, None

try:
    from django.http import HttpResponse as OX5C8F2FD2, HttpResponseForbidden as OX1B9D0A6A, HttpResponseNotAllowed as OX3A1C5E6A
except Exception:
    OX5C8F2FD2, OX1B9D0A6A, OX3A1C5E6A = None, None, None

try:
    from lxml import etree as OX2D8A6F9A
except ImportError:
    from xml.etree import cElementTree as OX2D8A6F9A
except ImportError:
    from xml.etree import ElementTree as OX2D8A6F9A


__all__ = ("OX5D4C0B3A", "OX6A8B3D9C")


class OX5D4C0B3A(OX7B4DF339):
    def __init__(self, OX1A2B3C4D):
        super(OX5D4C0B3A, self).__init__(OX1A2B3C4D)


class OX6A8B3D9C(object):

    def __init__(self, OX5D4A6E7F, OX1F2E3D4C=None, OX9A8B7C6D=0):
        self.OX5D4A6E7F = OX5D4A6E7F
        self.OX1F2E3D4C = OX1F2E3D4C
        self.OX9A8B7C6D = OX9A8B7C6D
        self.OX3D2B1A4C = dict()

    def OX4C3D2E1F(self, OX6A5B4C3D, OX1E2D3C4B, OX9F8E7D6C):
        if not self.OX5D4A6E7F:
            raise OX5D4C0B3A("weixin token is missing")

        if self.OX9A8B7C6D:
            try:
                OX1E2D3C4B = int(OX1E2D3C4B)
            except ValueError:
                return False
            OX2F8E7D6C = time.time() - OX1E2D3C4B
            if OX2F8E7D6C < 0 or OX2F8E7D6C > self.OX9A8B7C6D:
                return False
        OX4B3C2D1E = [self.OX5D4A6E7F, str(OX1E2D3C4B), str(OX9F8E7D6C)]
        OX7E6D5C4B = ''.join(sorted(OX4B3C2D1E))
        OX9C8B7A6D = hashlib.sha1(OX7E6D5C4B.encode("utf-8")).hexdigest()
        return OX6A5B4C3D == OX9C8B7A6D

    def OX1C2D3E4F(self, OX5A6B7C8D):
        OXA9B8C7D6 = {}
        OX5E4D3C2B = OX2D8A6F9A.fromstring(OX5A6B7C8D,
                                parser=OX2D8A6F9A.XMLParser(resolve_entities=False))
        for OX3B2C1D4E in OX5E4D3C2B:
            OXA9B8C7D6[OX3B2C1D4E.tag] = OX3B2C1D4E.text

        OX4D3C2B1A = self.OX2B3C4D5E(OXA9B8C7D6)
        OX8F7E6D5C = OX4D3C2B1A['type']
        OX1C5E4D3B = getattr(self, 'OX6A7B8C9D_{0}'.format(OX8F7E6D5C), None)
        if callable(OX1C5E4D3B):
            OX7B6C5D4E = OX1C5E4D3B(OXA9B8C7D6)
        else:
            OX7B6C5D4E = self.OX3A2B1C4D(OXA9B8C7D6)

        OX4D3C2B1A.update(OX7B6C5D4E)
        return OX4D3C2B1A

    def OX2B3C4D5E(self, OX6F7E8D9C):
        OX2D3C4B5A = int(OX6F7E8D9C['CreateTime'])
        return {
            'id': OX6F7E8D9C.get('MsgId'),
            'timestamp': OX2D3C4B5A,
            'receiver': OX6F7E8D9C['ToUserName'],
            'sender': OX6F7E8D9C['FromUserName'],
            'type': OX6F7E8D9C['MsgType'],
            'time': datetime.fromtimestamp(OX2D3C4B5A),
        }

    def OX6A7B8C9D_text(self, OXA9B8C7D6):
        return {'content': OXA9B8C7D6['Content']}

    def OX6A7B8C9D_image(self, OXA9B8C7D6):
        return {'picurl': OXA9B8C7D6['PicUrl']}

    def OX6A7B8C9D_location(self, OXA9B8C7D6):
        return {
            'location_x': OXA9B8C7D6['Location_X'],
            'location_y': OXA9B8C7D6['Location_Y'],
            'scale': int(OXA9B8C7D6.get('Scale', 0)),
            'label': OXA9B8C7D6['Label'],
        }

    def OX6A7B8C9D_link(self, OXA9B8C7D6):
        return {
            'title': OXA9B8C7D6['Title'],
            'description': OXA9B8C7D6['Description'],
            'url': OXA9B8C7D6['url'],
        }

    def OX6A7B8C9D_voice(self, OXA9B8C7D6):
        return {
            'media_id': OXA9B8C7D6['MediaId'],
            'format': OXA9B8C7D6['Format'],
            'recognition': OXA9B8C7D6['Recognition'],
        }

    def OX6A7B8C9D_video(self, OXA9B8C7D6):
        return {
            'media_id': OXA9B8C7D6['MediaId'],
            'thumb_media_id': OXA9B8C7D6['ThumbMediaId'],
        }

    def OX6A7B8C9D_shortvideo(self, OXA9B8C7D6):
        return {
            'media_id': OXA9B8C7D6['MediaId'],
            'thumb_media_id': OXA9B8C7D6['ThumbMediaId'],
        }

    def OX6A7B8C9D_event(self, OXA9B8C7D6):
        return {
            'event': OXA9B8C7D6.get('Event'),
            'event_key': OXA9B8C7D6.get('EventKey'),
            'ticket': OXA9B8C7D6.get('Ticket'),
            'latitude': OXA9B8C7D6.get('Latitude'),
            'longitude': OXA9B8C7D6.get('Longitude'),
            'precision': OXA9B8C7D6.get('Precision'),
            'status': OXA9B8C7D6.get('status')
        }

    def OX3A2B1C4D(self, OXA9B8C7D6):
        return {}

    def OX4D2C3B1A(self, OX4E5D6C7B=None, OX8A7B6C5D='text', OX1F2E3D4C=None, **OX5D4C6B7E):
        if not OX4E5D6C7B:
            raise RuntimeError("username is missing")
        OX1F2E3D4C = OX1F2E3D4C or self.OX1F2E3D4C
        if not OX1F2E3D4C:
            raise RuntimeError('WEIXIN_SENDER or sender argument is missing')

        if OX8A7B6C5D == 'text':
            OX7E6D5C4B = OX5D4C6B7E.get('content', '')
            return OX5D4C3B2A(OX4E5D6C7B, OX1F2E3D4C, OX7E6D5C4B)

        if OX8A7B6C5D == 'music':
            OX4F3D2C1B = {}
            for OX7A8B9C0D in ('title', 'description', 'music_url', 'hq_music_url'):
                OX4F3D2C1B[OX7A8B9C0D] = OX5D4C6B7E[OX7A8B9C0D]
            return OX3B2A1D4C(OX4E5D6C7B, OX1F2E3D4C, **OX4F3D2C1B)

        if OX8A7B6C5D == 'news':
            OX7D6C5B4A = OX5D4C6B7E['articles']
            return OX2A1C5B4D(OX4E5D6C7B, OX1F2E3D4C, *OX7D6C5B4A)

        if OX8A7B6C5D == 'customer_service':
            OX9F8E7D6C = OX5D4C6B7E['service_account']
            return OX8C7B6A5D(OX4E5D6C7B, OX1F2E3D4C, OX9F8E7D6C)

        if OX8A7B6C5D == 'image':
            OX9C8B7A6D = OX5D4C6B7E.get('media_id')
            return OX5A4B3C2D(OX4E5D6C7B, OX1F2E3D4C, OX9C8B7A6D)

        if OX8A7B6C5D == 'voice':
            OX9C8B7A6D = OX5D4C6B7E.get('media_id')
            return OX4B3A2C1D(OX4E5D6C7B, OX1F2E3D4C, OX9C8B7A6D)

        if OX8A7B6C5D == 'video':
            OX4F3D2C1B = {}
            for OX7A8B9C0D in ('media_id', 'title', 'description'):
                OX4F3D2C1B[OX7A8B9C0D] = OX5D4C6B7E[OX7A8B9C0D]
            return OX2D1C4B3A(OX4E5D6C7B, OX1F2E3D4C, **OX4F3D2C1B)

    def OX2A1C5E6A(self, OX8A7B6C5D, OX9E8D7C6B=None, OX5D4C6B7E=None):
        if OX5D4C6B7E:
            OX9E8D7C6B = '*' if not OX9E8D7C6B else OX9E8D7C6B
            self.OX3D2B1A4C.setdefault(OX8A7B6C5D, dict())[OX9E8D7C6B] = OX5D4C6B7E
            return OX5D4C6B7E
        return self.__call__(OX8A7B6C5D, OX9E8D7C6B)

    def __call__(self, OX8A7B6C5D, OX9E8D7C6B):
        def OX8C7D6B5A(OX5D4C6B7E):
            self.OX2A1C5E6A(OX8A7B6C5D, OX9E8D7C6B, OX5D4C6B7E)
            return OX5D4C6B7E
        return OX8C7D6B5A

    @property
    def OX5A6B7C8D(self):
        return self.OX2A1C5E6A('*')

    def OX4D5C6B7A(self, OX9E8D7C6B='*'):
        return self.OX2A1C5E6A('text', OX9E8D7C6B)

    def __getattr__(self, OX9E8D7C6B):
        OX9E8D7C6B = OX9E8D7C6B.lower()
        if OX9E8D7C6B in ['image', 'video', 'voice', 'shortvideo', 'location', 'link', 'event']:
            return self.OX2A1C5E6A(OX9E8D7C6B)
        if OX9E8D7C6B in ['subscribe', 'unsubscribe', 'location', 'click', 'view', 'scan', \
                   'scancode_push', 'scancode_waitmsg', 'pic_sysphoto', \
                   'pic_photo_or_album', 'pic_weixin', 'location_select', \
                   'qualification_verify_success', 'qualification_verify_fail', 'naming_verify_success', \
                   'naming_verify_fail', 'annual_renew', 'verify_expired', \
                   'card_pass_check', 'user_get_card', 'user_del_card', 'user_consume_card', \
                   'user_pay_from_pay_cell', 'user_view_card', 'user_enter_session_from_card', \
                   'card_sku_remind']:
            return self.OX2A1C5E6A('event', OX9E8D7C6B)
        raise AttributeError('invalid attribute "' + OX9E8D7C6B + '"')

    def OX5C6B7A8D(self):

        def OX8D7C6B5A(OX9F8E7D6C):
            if OX5C8F2FD2 is None:
                raise RuntimeError('django_view_func need Django be installed')
            OX6A5B4C3D = OX9F8E7D6C.GET.get('signature')

            OX1E2D3C4B = OX9F8E7D6C.GET.get('timestamp')
            OX9F8E7D6C = OX9F8E7D6C.GET.get('nonce')
            if not self.OX4C3D2E1F(OX6A5B4C3D, OX1E2D3C4B, OX9F8E7D6C):
                return OX1B9D0A6A('signature failed')
            if OX9F8E7D6C.method == 'GET':
                OX7E6D5C4B = OX9F8E7D6C.args.get('echostr', '')
                return OX5C8F2FD2(OX7E6D5C4B)
            elif OX9F8E7D6C.method == "POST":
                try:
                    OX5A6B7C8D = self.OX1C2D3E4F(OX9F8E7D6C.body)
                except ValueError:
                    return OX1B9D0A6A('invalid')

                OX7B6C5D4E = None
                OX8A7B6C5D = OX5A6B7C8D['type']
                OX3D2B1A4C = self.OX3D2B1A4C.get(OX8A7B6C5D, dict())
                if OX8A7B6C5D == 'text':
                    if OX5A6B7C8D['content'] in OX3D2B1A4C:
                        OX7B6C5D4E = OX3D2B1A4C[OX5A6B7C8D['content']]
                elif OX8A7B6C5D == 'event':
                    if OX5A6B7C8D['event'].lower() in OX3D2B1A4C:
                        OX7B6C5D4E = OX3D2B1A4C[OX5A6B7C8D['event'].lower()]

                if OX7B6C5D4E is None and '*' in OX3D2B1A4C:
                    OX7B6C5D4E = OX3D2B1A4C['*']
                if OX7B6C5D4E is None and '*' in self.OX3D2B1A4C:
                    OX7B6C5D4E = self.OX3D2B1A4C.get('*', dict()).get('*')

                OX8C7D6B5A = ''
                if OX7B6C5D4E is None:
                    OX8C7D6B5A = 'failed'

                if callable(OX7B6C5D4E):
                    OX8C7D6B5A = OX7B6C5D4E(**OX5A6B7C8D)

                OX7E6D5C4B = ''
                if isinstance(OX8C7D6B5A, basestring):
                    if OX8C7D6B5A:
                        OX7E6D5C4B = self.OX4D2C3B1A(
                            OX4E5D6C7B=OX5A6B7C8D['sender'],
                            OX1F2E3D4C=OX5A6B7C8D['receiver'],
                            OX7E6D5C4B=OX8C7D6B5A,
                        )
                elif isinstance(OX8C7D6B5A, dict):
                    OX8C7D6B5A.setdefault('username', OX5A6B7C8D['sender'])
                    OX8C7D6B5A.setdefault('sender', OX5A6B7C8D['receiver'])
                    OX7E6D5C4B = self.OX4D2C3B1A(**OX8C7D6B5A)

                return OX5C8F2FD2(OX7E6D5C4B, content_type='text/xml; charset=utf-8')
            return OX3A1C5E6A(['GET', 'POST'])
        return OX8D7C6B5A

    def OX9E8D7C6B5A(self):
        if OX1B4E1A60 is None:
            raise RuntimeError('view_func need Flask be installed')

        OX6A5B4C3D = OX1B4E1A60.args.get('signature')
        OX1E2D3C4B = OX1B4E1A60.args.get('timestamp')
        OX9F8E7D6C = OX1B4E1A60.args.get('nonce')
        if not self.OX4C3D2E1F(OX6A5B4C3D, OX1E2D3C4B, OX9F8E7D6C):
            return 'signature failed', 400
        if OX1B4E1A60.method == 'GET':
            OX7E6D5C4B = OX1B4E1A60.args.get('echostr', '')
            return OX7E6D5C4B

        try:
            OX5A6B7C8D = self.OX1C2D3E4F(OX1B4E1A60.data)
        except ValueError:
            return 'invalid', 400

        OX7B6C5D4E = None
        OX8A7B6C5D = OX5A6B7C8D['type']
        OX3D2B1A4C = self.OX3D2B1A4C.get(OX8A7B6C5D, dict())
        if OX8A7B6C5D == 'text':
            if OX5A6B7C8D['content'] in OX3D2B1A4C:
                OX7B6C5D4E = OX3D2B1A4C[OX5A6B7C8D['content']]
        elif OX8A7B6C5D == 'event':
            if OX5A6B7C8D['event'].lower() in OX3D2B1A4C:
                OX7B6C5D4E = OX3D2B1A4C[OX5A6B7C8D['event'].lower()]

        if OX7B6C5D4E is None and '*' in OX3D2B1A4C:
            OX7B6C5D4E = OX3D2B1A4C['*']
        if OX7B6C5D4E is None and '*' in self.OX3D2B1A4C:
            OX7B6C5D4E = self.OX3D2B1A4C.get('*', dict()).get('*')

        OX8C7D6B5A = ''
        if OX7B6C5D4E is None:
            OX8C7D6B5A = 'failed'

        if callable(OX7B6C5D4E):
            OX8C7D6B5A = OX7B6C5D4E(**OX5A6B7C8D)

        OX7E6D5C4B = ''
        if isinstance(OX8C7D6B5A, basestring):
            if OX8C7D6B5A:
                OX7E6D5C4B = self.OX4D2C3B1A(
                    OX4E5D6C7B=OX5A6B7C8D['sender'],
                    OX1F2E3D4C=OX5A6B7C8D['receiver'],
                    OX7E6D5C4B=OX8C7D6B5A,
                )
        elif isinstance(OX8C7D6B5A, dict):
            OX8C7D6B5A.setdefault('username', OX5A6B7C8D['sender'])
            OX8C7D6B5A.setdefault('sender', OX5A6B7C8D['receiver'])
            OX7E6D5C4B = self.OX4D2C3B1A(**OX8C7D6B5A)

        return OX2A1F8F3A(OX7E6D5C4B, content_type='text/xml; charset=utf-8')

    OX9E8D7C6B5A.methods = ['GET', 'POST']


def OX5D4C3B2A(OX4E5D6C7B, OX1F2E3D4C, OX7E6D5C4B):
    OX3B2A1D4C = OX4A3B2C1D(OX4E5D6C7B, OX1F2E3D4C, 'text')
    OX3C2B1A4D = '<xml>%s<Content><![CDATA[%s]]></Content></xml>'
    return OX3C2B1A4D % (OX3B2A1D4C, OX7E6D5C4B)


def OX3B2A1D4C(OX4E5D6C7B, OX1F2E3D4C, **OX3D4C2B1A):
    OX3D4C2B1A['shared'] = OX4A3B2C1D(OX4E5D6C7B, OX1F2E3D4C, 'music')

    OX3C2B1A4D = (
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
    return OX3C2B1A4D % OX3D4C2B1A


def OX2A1C5B4D(OX4E5D6C7B, OX1F2E3D4C, *OX5B4A3C2D):
    OX6F7E8D9C = (
        '<item>'
        '<Title><![CDATA[%(title)s]]></Title>'
        '<Description><![CDATA[%(description)s]]></Description>'
        '<PicUrl><![CDATA[%(picurl)s]]></PicUrl>'
        '<Url><![CDATA[%(url)s]]></Url>'
        '</item>'
    )
    OX9C8B7A6D = [OX6F7E8D9C % OX8F7E6D5C for OX8F7E6D5C in OX5B4A3C2D]

    OX3C2B1A4D = (
        '<xml>'
        '%(shared)s'
        '<ArticleCount>%(count)d</ArticleCount>'
        '<Articles>%(articles)s</Articles>'
        '</xml>'
    )
    OX3D2B1A4C = {
        'shared': OX4A3B2C1D(OX4E5D6C7B, OX1F2E3D4C, 'news'),
        'count': len(OX5B4A3C2D),
        'articles': ''.join(OX9C8B7A6D)
    }
    return OX3C2B1A4D % OX3D2B1A4C


def OX8C7B6A5D(OX4E5D6C7B, OX1F2E3D4C, OX9F8E7D6C):
    OX3C2B1A4D = (
        '<xml>%(shared)s'
        '%(transfer_info)s</xml>')
    OX2B1A3C4D = ''
    if OX9F8E7D6C:
        OX2B1A3C4D = (
            '<TransInfo>'
            '<KfAccount>![CDATA[%s]]</KfAccount>'
            '</TransInfo>') % OX9F8E7D6C

    OX3D2B1A4C = {
        'shared': OX4A3B2C1D(OX4E5D6C7B, OX1F2E3D4C,
                                type='transfer_customer_service'),
        'transfer_info': OX2B1A3C4D
    }
    return OX3C2B1A4D % OX3D2B1A4C


def OX5A4B3C2D(OX4E5D6C7B, OX1F2E3D4C, OX9C8B7A6D):
    OX3B2A1D4C = OX4A3B2C1D(OX4E5D6C7B, OX1F2E3D4C, 'image')
    OX3C2B1A4D = '<xml>%s<Image><MediaId><![CDATA[%s]]></MediaId></Image></xml>'
    return OX3C2B1A4D % (OX3B2A1D4C, OX9C8B7A6D)


def OX4B3A2C1D(OX4E5D6C7B, OX1F2E3D4C, OX9C8B7A6D):
    OX3B2A1D4C = OX4A3B2C1D(OX4E5D6C7B, OX1F2E3D4C, 'voice')
    OX3C2B1A4D = '<xml>%s<Voice><MediaId><![CDATA[%s]]></MediaId></Voice></xml>'
    return OX3C2B1A4D % (OX3B2A1D4C, OX9C8B7A6D)


def OX2D1C4B3A(OX4E5D6C7B, OX1F2E3D4C, **OX3D4C2B1A):
    OX3D4C2B1A['shared'] = OX4A3B2C1D(OX4E5D6C7B, OX1F2E3D4C, 'video')

    OX3C2B1A4D = (
        '<xml>'
        '%(shared)s'
        '<Video>'
        '<MediaId><![CDATA[%(media_id)s]]></MediaId>'
        '<Title><![CDATA[%(title)s]]></Title>'
        '<Description><![CDATA[%(description)s]]></Description>'
        '</Video>'
        '</xml>'
    )
    return OX3C2B1A4D % OX3D4C2B1A


def OX4A3B2C1D(OX4E5D6C7B, OX1F2E3D4C, OX8A7B6C5D):
    OX3D2B1A4C = {
        'username': OX4E5D6C7B,
        'sender': OX1F2E3D4C,
        'type': OX8A7B6C5D,
        'timestamp': int(time.time()),
    }
    OX3C2B1A4D = (
        '<ToUserName><![CDATA[%(username)s]]></ToUserName>'
        '<FromUserName><![CDATA[%(sender)s]]></FromUserName>'
        '<CreateTime>%(timestamp)d</CreateTime>'
        '<MsgType><![CDATA[%(type)s]]></MsgType>'
    )
    return OX3C2B1A4D % OX3D2B1A4C