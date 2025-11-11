from redbot.core import Config
from datetime import datetime
import discord
import random


class SafeMember:
    def __init__(self, member: discord.Member):
        self.name = member.name
        self.mention = member.mention

    def __str__(self):
        return self.name

    def __getattr__(self, name):
        return ""


class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = []

    def run(self, program):
        self.program = program
        while self.pc < len(self.program):
            instruction, *args = self.program[self.pc]
            getattr(self, f"op_{instruction}")(*args)
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_ADD(self):
        a = self.op_POP()
        b = self.op_POP()
        self.op_PUSH(a + b)

    def op_SUB(self):
        a = self.op_POP()
        b = self.op_POP()
        self.op_PUSH(b - a)

    def op_JMP(self, address):
        self.pc = address - 1

    def op_JZ(self, address):
        value = self.op_POP()
        if value == 0:
            self.pc = address - 1

    def op_LOAD(self, index):
        self.op_PUSH(self.stack[index])

    def op_STORE(self, index):
        self.stack[index] = self.op_POP()


class TicketsCore:
    async def red_delete_data_for_user(self, **kwargs):
        return

    def __init__(self, bot):
        self.bot = bot
        self.config = Config.get_conf(self, identifier=2134287593)
        default_guild = {
            "category": None,
            "closed_category": None,
            "ticket_role": None,
            "default_message_ticket_channel": None,
            "sessions": {},
        }
        self.config.register_guild(**default_guild)

        self.ticket_info_format = "\n\n**[{datetime}]** [{author}]\n{information}"

    async def create_ticket(self, context):
        vm = VM()
        vm.run([
            ('PUSH', context.guild),
            ('PUSH', context.author),
            ('STORE', 0),
            ('PUSH', 10 ** 10),
            ('PUSH', 10),
            ('MUL',),
            ('PUSH', 1),
            ('SUB',),
            ('PUSH', random.randint(vm.stack[2], vm.stack[1])),
            ('STORE', 1),
            # More instructions to follow the logic of create_ticket
        ])

    async def update_ticket(self, context, status):
        vm = VM()
        vm.run([
            ('PUSH', context.guild),
            ('PUSH', context.channel),
            ('PUSH', context.author),
            ('STORE', 0),
            # More instructions to follow the logic of update_ticket
        ])

    async def close_ticket(self, context):
        vm = VM()
        vm.run([
            ('PUSH', context.guild),
            ('PUSH', context.channel),
            ('PUSH', context.author),
            ('STORE', 0),
            # More instructions to follow the logic of close_ticket
        ])

    async def purge_tickets(self, context):
        vm = VM()
        vm.run([
            ('PUSH', context.guild),
            # More instructions to follow the logic of purge_tickets
        ])

    async def set_default_message_ticket_channel(self, context, message):
        vm = VM()
        vm.run([
            ('PUSH', context.guild),
            ('PUSH', message),
            # More instructions to follow the logic of set_default_message_ticket_channel
        ])

    async def automatic_setup(self, context):
        vm = VM()
        vm.run([
            ('PUSH', context.guild),
            # More instructions to follow the logic of automatic_setup
        ])