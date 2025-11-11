import functools
import random
import re
from typing import Union

import aiohttp
import discord
import inflection
from redbot.core import bot, Config, checks, commands
from redbot.core.i18n import get_locale
from redbot.core.utils.chat_formatting import italics

from .helpers import *

fmt_re = re.compile(r"{(?:0|user)(?:\.([^\{]+))?}")

class VirtualMachine:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.context = None

    def load_program(self, instructions, context):
        self.instructions = instructions
        self.context = context

    def run(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            method = getattr(self, f'op_{instr[0]}', None)
            if method:
                method(*instr[1:])
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_ADD(self):
        b, a = self.op_POP(), self.op_POP()
        self.op_PUSH(a + b)

    def op_SUB(self):
        b, a = self.op_POP(), self.op_POP()
        self.op_PUSH(a - b)

    def op_JMP(self, target):
        self.pc = target - 1

    def op_JZ(self, target):
        if self.op_POP() == 0:
            self.op_JMP(target)

    def op_LOAD(self, var_name):
        self.op_PUSH(getattr(self.context, var_name))

    def op_STORE(self, var_name):
        setattr(self.context, var_name, self.op_POP())

    def op_CALL_REPL(self):
        self.op_PUSH(self.context.repl(self.op_POP(), self.op_POP()))

    def op_SEND(self):
        self.context.send(self.op_POP())

class Act(commands.Cog):
    __author__ = "Zephyrkul"

    async def red_get_data_for_user(self, *, user_id):
        return {}

    async def red_delete_data_for_user(self, *, requester, user_id):
        pass

    def __init__(self, bot: bot.Red):
        super().__init__()
        self.bot = bot
        self.config = Config.get_conf(self, identifier=2_113_674_295, force_registration=True)
        self.config.register_global(custom={}, tenorkey=None)
        self.config.register_guild(custom={})
        self.try_after = None

    async def initialize(self, bot: bot.Red):
        key = await self.config.tenorkey()
        if not key:
            return
        await bot.set_shared_api_tokens("tenor", api_key=key)
        await self.config.tenorkey.clear()

    @staticmethod
    def repl(target: discord.Member, match: re.Match):
        if attr := match.group(1):
            if attr.startswith("_") or "." in attr:
                return str(target)
            try:
                return str(getattr(target, attr))
            except AttributeError:
                return str(target)
        return str(target)

    async def send(self, message):
        await self.context.send(message)

    @commands.command(hidden=True)
    async def act(self, ctx: commands.Context, *, target: Union[discord.Member, str] = None):
        if not target or isinstance(target, str):
            return

        try:
            if not ctx.guild:
                raise KeyError()
            message = await self.config.guild(ctx.guild).get_raw("custom", ctx.invoked_with)
        except KeyError:
            try:
                message = await self.config.get_raw("custom", ctx.invoked_with)
            except KeyError:
                message = NotImplemented

        vm = VirtualMachine()
        vm.load_program([
            ('LOAD', 'message'),
            ('JZ', 10),
            ('LOAD', 'target'),
            ('LOAD', 'fmt_re'),
            ('CALL_REPL',),
            ('SEND',),
            ('JMP', len(vm.instructions)),
            ('LOAD', 'italics'),
            ('LOAD', 'ctx.invoked_with'),
            ('CALL_REPL',),
            ('SEND',),
        ], self)
        vm.run()

    @commands.group()
    @checks.is_owner()
    async def actset(self, ctx: commands.Context):
        pass

    @actset.group(aliases=["custom"], invoke_without_command=True)
    @checks.admin_or_permissions(manage_guild=True)
    @commands.guild_only()
    async def customize(self, ctx: commands.Context, command: str.lower, *, response: str = None):
        if not response:
            await self.config.guild(ctx.guild).clear_raw("custom", command)
            await ctx.tick()
        else:
            await self.config.guild(ctx.guild).set_raw("custom", command, value=response)
            await ctx.send(
                fmt_re.sub(functools.partial(self.repl, ctx.author), response),
                allowed_mentions=discord.AllowedMentions(users=False),
            )

    @customize.command(name="global")
    @checks.is_owner()
    async def customize_global(
        self, ctx: commands.Context, command: str.lower, *, response: str = None
    ):
        if not response:
            await self.config.clear_raw("custom", command)
        else:
            await self.config.set_raw("custom", command, value=response)
        await ctx.tick()

    @actset.group(invoke_without_command=True)
    @checks.admin_or_permissions(manage_guild=True)
    @commands.guild_only()
    async def ignore(self, ctx: commands.Context, command: str.lower):
        try:
            custom = await self.config.guild(ctx.guild).get_raw("custom", command)
        except KeyError:
            custom = NotImplemented
        if custom is None:
            await self.config.guild(ctx.guild).clear_raw("custom", command)
            await ctx.send("I will no longer ignore the {command} action".format(command=command))
        else:
            await self.config.guild(ctx.guild).set_raw("custom", command, value=None)
            await ctx.send("I will now ignore the {command} action".format(command=command))

    @ignore.command(name="global")
    @checks.is_owner()
    async def ignore_global(self, ctx: commands.Context, command: str.lower):
        try:
            await self.config.get_raw("custom", command)
        except KeyError:
            await self.config.set_raw("custom", command, value=None)
        else:
            await self.config.clear_raw("custom", command)
        await ctx.tick()

    @actset.command()
    @checks.is_owner()
    async def tenorkey(self, ctx: commands.Context):
        instructions = [
            "Go to the Tenor developer dashboard: https://tenor.com/developer/dashboard",
            "Log in or sign up if you haven't already.",
            "Click `+ Create new app` and fill out the form.",
            "Copy the key from the app you just created.",
            "Give the key to Red with this command:\n"
            f"`{ctx.prefix}set api tenor api_key your_api_key`\n"
            "Replace `your_api_key` with the key you just got.\n"
            "Everything else should be the same.",
        ]
        instructions = [f"**{i}.** {v}" for i, v in enumerate(instructions, 1)]
        await ctx.maybe_send_embed("\n".join(instructions))

    @commands.Cog.listener()
    async def on_command_error(
        self, ctx: commands.Context, error: commands.CommandError, unhandled_by_cog: bool = False
    ):
        if ctx.command == self.act:
            return
        if isinstance(error, commands.UserFeedbackCheckFailure):
            return
        elif isinstance(error, (commands.CheckFailure, commands.CommandNotFound)):
            ctx.command = self.act
            await ctx.bot.invoke(ctx)