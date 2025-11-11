import logging
import slixmpp
from slixmpp.stanza import Message, Iq
from slixmpp.xmlstream.handler import Callback
from slixmpp.xmlstream.matcher import StanzaPath
from slixmpp.xmlstream import register_stanza_plugin
from slixmpp.plugins import BasePlugin
from slixmpp.plugins.xep_0280 import stanza

def getName():
    return 'xep_0280'

def getDescription():
    return 'XEP-0280: Message Carbons'

def getDependencies():
    return {'xep_0030', 'xep_0297'}

def getStanza():
    return stanza

def getLogger():
    return logging.getLogger(__name__)

def getTypeSet():
    return 'set'

log = getLogger()

class XEP_0280(BasePlugin):

    def __init__(self, *args, **kwargs):
        self.name = getName()
        self.description = getDescription()
        self.dependencies = getDependencies()
        self.stanza = getStanza()
        super(XEP_0280, self).__init__(*args, **kwargs)

    def plugin_init(self):
        self.xmpp.register_handler(
            Callback('Carbon Received',
                     StanzaPath('message/carbon_received'),
                     self._handle_carbon_received))
        self.xmpp.register_handler(
            Callback('Carbon Sent',
                     StanzaPath('message/carbon_sent'),
                     self._handle_carbon_sent))

        register_stanza_plugin(Message, self.stanza.ReceivedCarbon)
        register_stanza_plugin(Message, self.stanza.SentCarbon)
        register_stanza_plugin(Message, self.stanza.PrivateCarbon)
        register_stanza_plugin(Iq, self.stanza.CarbonEnable)
        register_stanza_plugin(Iq, self.stanza.CarbonDisable)

        register_stanza_plugin(self.stanza.ReceivedCarbon,
                               self.xmpp['xep_0297'].stanza.Forwarded)
        register_stanza_plugin(self.stanza.SentCarbon,
                               self.xmpp['xep_0297'].stanza.Forwarded)

    def plugin_end(self):
        self.xmpp.remove_handler('Carbon Received')
        self.xmpp.remove_handler('Carbon Sent')
        self.xmpp.plugin['xep_0030'].del_feature(feature='urn:xmpp:carbons:2')

    def session_bind(self, jid):
        self.xmpp.plugin['xep_0030'].add_feature('urn:xmpp:carbons:2')

    def _handle_carbon_received(self, msg):
        if msg['from'].bare == self.xmpp.boundjid.bare:
            self.xmpp.event('carbon_received', msg)

    def _handle_carbon_sent(self, msg):
        if msg['from'].bare == self.xmpp.boundjid.bare:
            self.xmpp.event('carbon_sent', msg)

    def enable(self, ifrom=None, timeout=None, callback=None,
               timeout_callback=None):
        iq = self.xmpp.Iq()
        iq['type'] = getTypeSet()
        iq['from'] = ifrom
        iq.enable('carbon_enable')
        return iq.send(timeout_callback=timeout_callback, timeout=timeout,
                       callback=callback)

    def disable(self, ifrom=None, timeout=None, callback=None,
                timeout_callback=None):
        iq = self.xmpp.Iq()
        iq['type'] = getTypeSet()
        iq['from'] = ifrom
        iq.enable('carbon_disable')
        return iq.send(timeout_callback=timeout_callback, timeout=timeout,
                       callback=callback)