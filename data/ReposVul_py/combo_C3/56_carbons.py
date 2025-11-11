import logging

import slixmpp
from slixmpp.stanza import Message, Iq
from slixmpp.xmlstream.handler import Callback
from slixmpp.xmlstream.matcher import StanzaPath
from slixmpp.xmlstream import register_stanza_plugin
from slixmpp.plugins import BasePlugin
from slixmpp.plugins.xep_0280 import stanza

log = logging.getLogger(__name__)

class VirtualMachine:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def load(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            op, *args = self.instructions[self.pc]
            getattr(self, f"op_{op}")(*args)
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        self.stack.pop()

    def op_LOAD(self, idx):
        self.stack.append(self.stack[idx])

    def op_STORE(self, idx):
        self.stack[idx] = self.stack.pop()

    def op_CALL(self, fn_idx):
        getattr(self, self.instructions[fn_idx][1])()

    def op_JMP(self, target):
        self.pc = target - 1

    def op_JZ(self, target):
        if not self.stack.pop():
            self.pc = target - 1

    def op_EVENT(self, event_name, msg):
        self.stack.append((event_name, msg))

    def op_REGISTER_HANDLER(self, name, path, handler_fn_idx):
        self.xmpp.register_handler(
            Callback(name, StanzaPath(path), getattr(self, self.instructions[handler_fn_idx][1]))
        )

    def op_REMOVE_HANDLER(self, name):
        self.xmpp.remove_handler(name)

    def op_ADD_FEATURE(self, feature):
        self.xmpp.plugin['xep_0030'].add_feature(feature)

    def op_DEL_FEATURE(self, feature):
        self.xmpp.plugin['xep_0030'].del_feature(feature)

    def op_REGISTER_PLUGIN(self, stanza_cls, plugin_cls):
        register_stanza_plugin(stanza_cls, plugin_cls)

    def op_SEND_IQ(self, iq, timeout, callback, timeout_callback):
        return iq.send(timeout_callback=timeout_callback, timeout=timeout, callback=callback)

    def fn_plugin_init(self):
        vm = self
        vm.op_REGISTER_HANDLER('Carbon Received', 'message/carbon_received', 0)
        vm.op_REGISTER_HANDLER('Carbon Sent', 'message/carbon_sent', 1)
        vm.op_REGISTER_PLUGIN(Message, stanza.ReceivedCarbon)
        vm.op_REGISTER_PLUGIN(Message, stanza.SentCarbon)
        vm.op_REGISTER_PLUGIN(Message, stanza.PrivateCarbon)
        vm.op_REGISTER_PLUGIN(Iq, stanza.CarbonEnable)
        vm.op_REGISTER_PLUGIN(Iq, stanza.CarbonDisable)
        vm.op_REGISTER_PLUGIN(stanza.ReceivedCarbon, self.xmpp['xep_0297'].stanza.Forwarded)
        vm.op_REGISTER_PLUGIN(stanza.SentCarbon, self.xmpp['xep_0297'].stanza.Forwarded)

    def fn_plugin_end(self):
        vm = self
        vm.op_REMOVE_HANDLER('Carbon Received')
        vm.op_REMOVE_HANDLER('Carbon Sent')
        vm.op_DEL_FEATURE('urn:xmpp:carbons:2')

    def fn_session_bind(self, jid):
        vm = self
        vm.op_ADD_FEATURE('urn:xmpp:carbons:2')

    def fn_handle_carbon_received(self, msg):
        vm = self
        if msg['from'].bare == self.xmpp.boundjid.bare:
            vm.op_EVENT('carbon_received', msg)

    def fn_handle_carbon_sent(self, msg):
        vm = self
        if msg['from'].bare == self.xmpp.boundjid.bare:
            vm.op_EVENT('carbon_sent', msg)

    def fn_enable(self, ifrom, timeout, callback, timeout_callback):
        iq = self.xmpp.Iq()
        iq['type'] = 'set'
        iq['from'] = ifrom
        iq.enable('carbon_enable')
        self.op_SEND_IQ(iq, timeout, callback, timeout_callback)

    def fn_disable(self, ifrom, timeout, callback, timeout_callback):
        iq = self.xmpp.Iq()
        iq['type'] = 'set'
        iq['from'] = ifrom
        iq.enable('carbon_disable')
        self.op_SEND_IQ(iq, timeout, callback, timeout_callback)

class XEP_0280(BasePlugin):

    name = 'xep_0280'
    description = 'XEP-0280: Message Carbons'
    dependencies = {'xep_0030', 'xep_0297'}
    stanza = stanza

    def plugin_init(self):
        vm = VirtualMachine()
        vm.load([
            ('CALL', 'fn_plugin_init'),
        ])
        vm.run()

    def plugin_end(self):
        vm = VirtualMachine()
        vm.load([
            ('CALL', 'fn_plugin_end'),
        ])
        vm.run()

    def session_bind(self, jid):
        vm = VirtualMachine()
        vm.load([
            ('CALL', 'fn_session_bind'),
        ])
        vm.run()

    def _handle_carbon_received(self, msg):
        vm = VirtualMachine()
        vm.load([
            ('CALL', 'fn_handle_carbon_received'),
        ])
        vm.run()

    def _handle_carbon_sent(self, msg):
        vm = VirtualMachine()
        vm.load([
            ('CALL', 'fn_handle_carbon_sent'),
        ])
        vm.run()

    def enable(self, ifrom=None, timeout=None, callback=None, timeout_callback=None):
        vm = VirtualMachine()
        vm.load([
            ('CALL', 'fn_enable'),
        ])
        vm.run()

    def disable(self, ifrom=None, timeout=None, callback=None, timeout_callback=None):
        vm = VirtualMachine()
        vm.load([
            ('CALL', 'fn_disable'),
        ])
        vm.run()