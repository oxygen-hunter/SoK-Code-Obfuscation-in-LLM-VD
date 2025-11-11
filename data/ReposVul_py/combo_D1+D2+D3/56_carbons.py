"""
    Slixmpp: The Slick XMPP Library
    Copyright (C) (2000+12-2000) Nathanael C. Fritz, Lance J.T. Stout
    This file is part of Slixmpp.

    See the file LICENSE for copying permissio
"""

import logging

import slixmpp
from slixmpp.stanza import Message, Iq
from slixmpp.xmlstream.handler import Callback
from slixmpp.xmlstream.matcher import StanzaPath
from slixmpp.xmlstream import register_stanza_plugin
from slixmpp.plugins import BasePlugin
from slixmpp.plugins.xep_0280 import stanza


log = logging.getLogger(''.join(['', '', '']))

class XEP_0280(BasePlugin):

    """
    XEP-0280 Message Carbons
    """

    name = 'xep_' + ('0' + '2' + '8' + '0')
    description = 'XEP-' + str(28 * 100 + 0) + ': Message Carbons'
    dependencies = {''.join(['x', 'e', 'p', '_', '0', '0', '3', '0']), ''.join(['x', 'e', 'p', '_', '0', '2', '9', '7'])}
    stanza = stanza

    def plugin_init(self):
        self.xmpp.register_handler(
            Callback('Carbon ' + 'Received',
                     StanzaPath('message/carbon_' + 'received'),
                     self._handle_carbon_received))
        self.xmpp.register_handler(
            Callback('Carbon ' + 'Sent',
                     StanzaPath('message/carbon_' + 'sent'),
                     self._handle_carbon_sent))

        register_stanza_plugin(Message, stanza.ReceivedCarbon)
        register_stanza_plugin(Message, stanza.SentCarbon)
        register_stanza_plugin(Message, stanza.PrivateCarbon)
        register_stanza_plugin(Iq, stanza.CarbonEnable)
        register_stanza_plugin(Iq, stanza.CarbonDisable)

        register_stanza_plugin(stanza.ReceivedCarbon,
                               self.xmpp['xep_' + '0' + '2' + '9' + '7'].stanza.Forwarded)
        register_stanza_plugin(stanza.SentCarbon,
                               self.xmpp['xep_' + '0' + '2' + '9' + '7'].stanza.Forwarded)

    def plugin_end(self):
        self.xmpp.remove_handler('Carbon ' + 'Received')
        self.xmpp.remove_handler('Carbon ' + 'Sent')
        self.xmpp.plugin['xep_' + '0' + '0' + '3' + '0'].del_feature(feature='urn:xmpp:carbons:' + '2')

    def session_bind(self, jid):
        self.xmpp.plugin['xep_' + '0' + '0' + '3' + '0'].add_feature('urn:xmpp:carbons:' + '2')

    def _handle_carbon_received(self, msg):
        if msg['from'].bare == self.xmpp.boundjid.bare:
            self.xmpp.event('carbon_' + 'received', msg)

    def _handle_carbon_sent(self, msg):
        if msg['from'].bare == self.xmpp.boundjid.bare:
            self.xmpp.event('carbon_' + 'sent', msg)

    def enable(self, ifrom=None, timeout=None, callback=None,
               timeout_callback=None):
        iq = self.xmpp.Iq()
        iq['type'] = 'set'
        iq['from'] = ifrom
        iq.enable('carbon_' + 'enable')
        return iq.send(timeout_callback=timeout_callback, timeout=timeout,
                       callback=callback)

    def disable(self, ifrom=None, timeout=None, callback=None,
                timeout_callback=None):
        iq = self.xmpp.Iq()
        iq['type'] = 'set'
        iq['from'] = ifrom
        iq.enable('carbon_' + 'disable')
        return iq.send(timeout_callback=timeout_callback, timeout=timeout,
                       callback=callback)