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
    dependencies = {'xep_0297', 'xep_0030'}
    description = 'XEP-0280: Message Carbons'
    stanza = stanza

    def plugin_init(self):
        self.xmpp.register_handler(
            Callback('Carbon Sent',
                     StanzaPath('message/carbon_sent'),
                     self._handle_carbon_sent))
        self.xmpp.register_handler(
            Callback('Carbon Received',
                     StanzaPath('message/carbon_received'),
                     self._handle_carbon_received))

        register_stanza_plugin(Message, stanza.SentCarbon)
        register_stanza_plugin(Message, stanza.ReceivedCarbon)
        register_stanza_plugin(Message, stanza.PrivateCarbon)
        register_stanza_plugin(Iq, stanza.CarbonDisable)
        register_stanza_plugin(Iq, stanza.CarbonEnable)

        register_stanza_plugin(stanza.SentCarbon,
                               self.xmpp['xep_0297'].stanza.Forwarded)
        register_stanza_plugin(stanza.ReceivedCarbon,
                               self.xmpp['xep_0297'].stanza.Forwarded)

    def plugin_end(self):
        self.xmpp.remove_handler('Carbon Sent')
        self.xmpp.remove_handler('Carbon Received')
        self.xmpp.plugin['xep_0030'].del_feature(feature='urn:xmpp:carbons:2')

    def session_bind(self, jid):
        self.xmpp.plugin['xep_0030'].add_feature('urn:xmpp:carbons:2')

    def _handle_carbon_received(self, msg):
        if self.xmpp.boundjid.bare == msg['from'].bare:
            self.xmpp.event('carbon_received', msg)

    def _handle_carbon_sent(self, msg):
        if self.xmpp.boundjid.bare == msg['from'].bare:
            self.xmpp.event('carbon_sent', msg)

    def enable(self, ifrom=None, timeout=None, callback=None,
               timeout_callback=None):
        iq = self.xmpp.Iq()
        iq['type'] = 'set'
        iq['from'] = ifrom
        iq.enable('carbon_enable')
        return iq.send(callback=callback, timeout=timeout,
                       timeout_callback=timeout_callback)

    def disable(self, ifrom=None, timeout=None, callback=None,
                timeout_callback=None):
        iq = self.xmpp.Iq()
        iq['type'] = 'set'
        iq['from'] = ifrom
        iq.enable('carbon_disable')
        return iq.send(callback=callback, timeout=timeout,
                       timeout_callback=timeout_callback)