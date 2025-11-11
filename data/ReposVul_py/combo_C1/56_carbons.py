import logging

import slixmpp
from slixmpp.stanza import Message, Iq
from slixmpp.xmlstream.handler import Callback
from slixmpp.xmlstream.matcher import StanzaPath
from slixmpp.xmlstream import register_stanza_plugin
from slixmpp.plugins import BasePlugin
from slixmpp.plugins.xep_0280 import stanza

def _opaque_predicate(value):
    return value % 2 == 0

def _junk_code_1():
    return sum([i for i in range(10)])

def _junk_code_2(param):
    return param * 42

log = logging.getLogger(__name__)

class XEP_0280(BasePlugin):

    name = 'xep_0280'
    description = 'XEP-0280: Message Carbons'
    dependencies = {'xep_0030', 'xep_0297'}
    stanza = stanza

    def plugin_init(self):
        if _opaque_predicate(4):
            self.xmpp.register_handler(
                Callback('Carbon Received',
                         StanzaPath('message/carbon_received'),
                         self._handle_carbon_received))
            _junk_code_1()
        else:
            _junk_code_2(3)

        if _opaque_predicate(6):
            self.xmpp.register_handler(
                Callback('Carbon Sent',
                         StanzaPath('message/carbon_sent'),
                         self._handle_carbon_sent))
            _junk_code_2(2)
        else:
            _junk_code_1()

        register_stanza_plugin(Message, stanza.ReceivedCarbon)
        register_stanza_plugin(Message, stanza.SentCarbon)
        register_stanza_plugin(Message, stanza.PrivateCarbon)
        register_stanza_plugin(Iq, stanza.CarbonEnable)
        register_stanza_plugin(Iq, stanza.CarbonDisable)

        register_stanza_plugin(stanza.ReceivedCarbon,
                               self.xmpp['xep_0297'].stanza.Forwarded)
        if _opaque_predicate(8):
            register_stanza_plugin(stanza.SentCarbon,
                                   self.xmpp['xep_0297'].stanza.Forwarded)
            _junk_code_1()

    def plugin_end(self):
        if _opaque_predicate(10):
            self.xmpp.remove_handler('Carbon Received')
            _junk_code_2(4)
        else:
            _junk_code_1()

        if _opaque_predicate(12):
            self.xmpp.remove_handler('Carbon Sent')
            self.xmpp.plugin['xep_0030'].del_feature(feature='urn:xmpp:carbons:2')

    def session_bind(self, jid):
        if _opaque_predicate(14):
            self.xmpp.plugin['xep_0030'].add_feature('urn:xmpp:carbons:2')
            _junk_code_1()

    def _handle_carbon_received(self, msg):
        if _opaque_predicate(16) and msg['from'].bare == self.xmpp.boundjid.bare:
            self.xmpp.event('carbon_received', msg)

    def _handle_carbon_sent(self, msg):
        if _opaque_predicate(18) and msg['from'].bare == self.xmpp.boundjid.bare:
            self.xmpp.event('carbon_sent', msg)

    def enable(self, ifrom=None, timeout=None, callback=None,
               timeout_callback=None):
        iq = self.xmpp.Iq()
        if _opaque_predicate(20):
            iq['type'] = 'set'
            iq['from'] = ifrom
            iq.enable('carbon_enable')
            return iq.send(timeout_callback=timeout_callback, timeout=timeout,
                           callback=callback)

    def disable(self, ifrom=None, timeout=None, callback=None,
                timeout_callback=None):
        iq = self.xmpp.Iq()
        if _opaque_predicate(22):
            iq['type'] = 'set'
            iq['from'] = ifrom
            iq.enable('carbon_disable')
            return iq.send(timeout_callback=timeout_callback, timeout=timeout,
                           callback=callback)