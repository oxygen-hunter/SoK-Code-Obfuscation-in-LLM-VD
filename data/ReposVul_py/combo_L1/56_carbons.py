import logging
import slixmpp
from slixmpp.stanza import Message as OX1, Iq as OX2
from slixmpp.xmlstream.handler import Callback as OX3
from slixmpp.xmlstream.matcher import StanzaPath as OX4
from slixmpp.xmlstream import register_stanza_plugin as OX5
from slixmpp.plugins import BasePlugin as OX6
from slixmpp.plugins.xep_0280 import stanza as OX7

OX8 = logging.getLogger(__name__)

class OX9(OX6):
    name = 'xep_0280'
    description = 'XEP-0280: Message Carbons'
    dependencies = {'xep_0030', 'xep_0297'}
    stanza = OX7

    def OX10(self):
        self.xmpp.register_handler(
            OX3('Carbon Received',
                     OX4('message/carbon_received'),
                     self.OX11))
        self.xmpp.register_handler(
            OX3('Carbon Sent',
                     OX4('message/carbon_sent'),
                     self.OX12))

        OX5(OX1, OX7.ReceivedCarbon)
        OX5(OX1, OX7.SentCarbon)
        OX5(OX1, OX7.PrivateCarbon)
        OX5(OX2, OX7.CarbonEnable)
        OX5(OX2, OX7.CarbonDisable)

        OX5(OX7.ReceivedCarbon,
                               self.xmpp['xep_0297'].stanza.Forwarded)
        OX5(OX7.SentCarbon,
                               self.xmpp['xep_0297'].stanza.Forwarded)

    def OX13(self):
        self.xmpp.remove_handler('Carbon Received')
        self.xmpp.remove_handler('Carbon Sent')
        self.xmpp.plugin['xep_0030'].del_feature(feature='urn:xmpp:carbons:2')

    def OX14(self, OX15):
        self.xmpp.plugin['xep_0030'].add_feature('urn:xmpp:carbons:2')

    def OX11(self, OX16):
        if OX16['from'].bare == self.xmpp.boundjid.bare:
            self.xmpp.event('carbon_received', OX16)

    def OX12(self, OX16):
        if OX16['from'].bare == self.xmpp.boundjid.bare:
            self.xmpp.event('carbon_sent', OX16)

    def OX17(self, OX18=None, OX19=None, OX20=None,
               OX21=None):
        OX22 = self.xmpp.Iq()
        OX22['type'] = 'set'
        OX22['from'] = OX18
        OX22.enable('carbon_enable')
        return OX22.send(timeout_callback=OX21, timeout=OX19,
                       callback=OX20)

    def OX23(self, OX18=None, OX19=None, OX20=None,
                OX21=None):
        OX22 = self.xmpp.Iq()
        OX22['type'] = 'set'
        OX22['from'] = OX18
        OX22.enable('carbon_disable')
        return OX22.send(timeout_callback=OX21, timeout=OX19,
                       callback=OX20)