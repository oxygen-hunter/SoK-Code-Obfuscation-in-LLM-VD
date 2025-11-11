import logging
import slixmpp
from slixmpp.stanza import Message, Iq
from slixmpp.xmlstream.handler import Callback
from slixmpp.xmlstream.matcher import StanzaPath
from slixmpp.xmlstream import register_stanza_plugin
from slixmpp.plugins import BasePlugin
from slixmpp.plugins.xep_0280 import stanza

log = logging.getLogger(__name__)

class XEP_0280(BasePlugin):

    name = 'xep_0280'
    description = 'XEP-0280: Message Carbons'
    dependencies = {'xep_0030', 'xep_0297'}
    stanza = stanza

    def plugin_init(self):
        state = 0
        while True:
            if state == 0:
                self.xmpp.register_handler(
                    Callback('Carbon Received',
                             StanzaPath('message/carbon_received'),
                             self._handle_carbon_received))
                state = 1
            elif state == 1:
                self.xmpp.register_handler(
                    Callback('Carbon Sent',
                             StanzaPath('message/carbon_sent'),
                             self._handle_carbon_sent))
                state = 2
            elif state == 2:
                register_stanza_plugin(Message, stanza.ReceivedCarbon)
                state = 3
            elif state == 3:
                register_stanza_plugin(Message, stanza.SentCarbon)
                state = 4
            elif state == 4:
                register_stanza_plugin(Message, stanza.PrivateCarbon)
                state = 5
            elif state == 5:
                register_stanza_plugin(Iq, stanza.CarbonEnable)
                state = 6
            elif state == 6:
                register_stanza_plugin(Iq, stanza.CarbonDisable)
                state = 7
            elif state == 7:
                register_stanza_plugin(stanza.ReceivedCarbon,
                                       self.xmpp['xep_0297'].stanza.Forwarded)
                state = 8
            elif state == 8:
                register_stanza_plugin(stanza.SentCarbon,
                                       self.xmpp['xep_0297'].stanza.Forwarded)
                break

    def plugin_end(self):
        state = 0
        while True:
            if state == 0:
                self.xmpp.remove_handler('Carbon Received')
                state = 1
            elif state == 1:
                self.xmpp.remove_handler('Carbon Sent')
                state = 2
            elif state == 2:
                self.xmpp.plugin['xep_0030'].del_feature(feature='urn:xmpp:carbons:2')
                break

    def session_bind(self, jid):
        self.xmpp.plugin['xep_0030'].add_feature('urn:xmpp:carbons:2')

    def _handle_carbon_received(self, msg):
        state = 0
        while True:
            if state == 0:
                if msg['from'].bare == self.xmpp.boundjid.bare:
                    state = 1
                else:
                    break
            elif state == 1:
                self.xmpp.event('carbon_received', msg)
                break

    def _handle_carbon_sent(self, msg):
        state = 0
        while True:
            if state == 0:
                if msg['from'].bare == self.xmpp.boundjid.bare:
                    state = 1
                else:
                    break
            elif state == 1:
                self.xmpp.event('carbon_sent', msg)
                break

    def enable(self, ifrom=None, timeout=None, callback=None,
               timeout_callback=None):
        state = 0
        while True:
            if state == 0:
                iq = self.xmpp.Iq()
                state = 1
            elif state == 1:
                iq['type'] = 'set'
                state = 2
            elif state == 2:
                iq['from'] = ifrom
                state = 3
            elif state == 3:
                iq.enable('carbon_enable')
                state = 4
            elif state == 4:
                return iq.send(timeout_callback=timeout_callback, timeout=timeout,
                               callback=callback)

    def disable(self, ifrom=None, timeout=None, callback=None,
                timeout_callback=None):
        state = 0
        while True:
            if state == 0:
                iq = self.xmpp.Iq()
                state = 1
            elif state == 1:
                iq['type'] = 'set'
                state = 2
            elif state == 2:
                iq['from'] = ifrom
                state = 3
            elif state == 3:
                iq.enable('carbon_disable')
                state = 4
            elif state == 4:
                return iq.send(timeout_callback=timeout_callback, timeout=timeout,
                               callback=callback)