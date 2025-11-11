import functools
import random
import re
from typing import Union

import aiohttp
import discord
import inflection
from redbot.core import bot as OX7B4DF339, Config as OX6F3AD9E9, checks as OX0BDEA1CF, commands as OX4A0E59BE
from redbot.core.i18n import get_locale as OX1AF8D8C3
from redbot.core.utils.chat_formatting import italics as OX1E4A3F2F

from .helpers import *

OX30D5E4B1 = re.compile(r"{(?:0|user)(?:\.([^\{]+))?}")


class OX7AE3BC2E(OX4A0E59BE.Cog):
    __author__ = "Zephyrkul"

    async def OX0547A9B8(self, *, user_id):
        return {}

    async def OX1C1B1B3B(self, *, requester, user_id):
        pass

    def __init__(self, OX7ACD7A4B: OX7B4DF339.Red):
        super().__init__()
        self.OX7ACD7A4B = OX7ACD7A4B
        self.OX7AFD0F63 = OX6F3AD9E9.get_conf(self, identifier=2_113_674_295, force_registration=True)
        self.OX7AFD0F63.register_global(custom={}, tenorkey=None)
        self.OX7AFD0F63.register_guild(custom={})
        self.OX59F0AFBC = None

    async def OX5A8B7E1C(self, OX7ACD7A4B: OX7B4DF339.Red):
        OX049CC28B = await self.OX7AFD0F63.tenorkey()
        if not OX049CC28B:
            return
        await OX7ACD7A4B.set_shared_api_tokens("tenor", api_key=OX049CC28B)
        await self.OX7AFD0F63.tenorkey.clear()

    @staticmethod
    def OX3F3F3E9A(OX5DAF3FA4: discord.Member, OX0F3A8B6A: re.Match):
        if OX2C2B7E2B := OX0F3A8B6A.group(1):
            print(OX2C2B7E2B)
            if OX2C2B7E2B.startswith("_") or "." in OX2C2B7E2B:
                return str(OX5DAF3FA4)
            try:
                return str(getattr(OX5DAF3FA4, OX2C2B7E2B))
            except AttributeError:
                return str(OX5DAF3FA4)
        return str(OX5DAF3FA4)

    @OX4A0E59BE.command(hidden=True)
    async def OX3A6D4B7A(self, OX46A1E4F8: OX4A0E59BE.Context, *, OX5DAF3FA4: Union[discord.Member, str] = None):
        if not OX5DAF3FA4 or isinstance(OX5DAF3FA4, str):
            return

        try:
            if not OX46A1E4F8.guild:
                raise KeyError()
            OX2B4D8B2C = await self.OX7AFD0F63.guild(OX46A1E4F8.guild).get_raw("custom", OX46A1E4F8.invoked_with)
        except KeyError:
            try:
                OX2B4D8B2C = await self.OX7AFD0F63.get_raw("custom", OX46A1E4F8.invoked_with)
            except KeyError:
                OX2B4D8B2C = NotImplemented

        if OX2B4D8B2C is None:
            return
        elif OX2B4D8B2C is NotImplemented:
            OX3D8E5F8C = inflection.humanize(OX46A1E4F8.invoked_with).split()
            OX3DCE1D9E = -1

            for OX2A0B5F6E in range(2):
                if OX3DCE1D9E > -1:
                    break
                for i, OX3F8D5B7F in enumerate(OX3D8E5F8C):
                    OX3F8D5B7F = OX3F8D5B7F.lower()
                    if (
                        OX3F8D5B7F in NOLY_ADV
                        or OX3F8D5B7F in CONJ
                        or (OX3F8D5B7F.endswith("ly") and OX3F8D5B7F not in LY_VERBS)
                        or (not OX2A0B5F6E and OX3F8D5B7F in SOFT_VERBS)
                    ):
                        continue
                    OX3D8E5F8C[i] = inflection.pluralize(OX3D8E5F8C[i])
                    OX3DCE1D9E = max(OX3DCE1D9E, i)

            if OX3DCE1D9E < 0:
                return
            OX3D8E5F8C.insert(OX3DCE1D9E + 1, OX5DAF3FA4.mention)
            OX2B4D8B2C = OX1E4A3F2F(" ".join(OX3D8E5F8C))
        else:
            assert isinstance(OX2B4D8B2C, str)
            OX2B4D8B2C = OX30D5E4B1.sub(functools.partial(self.OX3F3F3E9A, OX5DAF3FA4), OX2B4D8B2C)

        if self.OX59F0AFBC and OX46A1E4F8.message.created_at < self.OX59F0AFBC:
            return await OX46A1E4F8.send(OX2B4D8B2C)
        if not await OX46A1E4F8.embed_requested():
            return await OX46A1E4F8.send(OX2B4D8B2C)
        OX049CC28B = (await OX46A1E4F8.bot.get_shared_api_tokens("tenor")).get("api_key")
        if not OX049CC28B:
            return await OX46A1E4F8.send(OX2B4D8B2C)
        async with aiohttp.request(
            "GET",
            "https://api.tenor.com/v1/search",
            params={
                "q": OX46A1E4F8.invoked_with,
                "key": OX049CC28B,
                "anon_id": str(OX46A1E4F8.author.id ^ OX46A1E4F8.me.id),
                "media_filter": "minimal",
                "contentfilter": "off" if getattr(OX46A1E4F8.channel, "nsfw", False) else "low",
                "ar_range": "wide",
                "limit": "8",
                "locale": OX1AF8D8C3(),
            },
        ) as OX7A4B6D2C:
            OX042E3D86: dict
            if OX7A4B6D2C.status == 429:
                self.OX59F0AFBC = OX46A1E4F8.message.created_at + 30
                OX042E3D86 = {}
            elif OX7A4B6D2C.status >= 400:
                OX042E3D86 = {}
            else:
                OX042E3D86 = await OX7A4B6D2C.json()
        if not OX042E3D86.get("results"):
            return await OX46A1E4F8.send(OX2B4D8B2C)
        OX2B4D8B2C = f"{OX2B4D8B2C}\n\n{random.choice(OX042E3D86['results'])['itemurl']}"
        await OX46A1E4F8.send(
            OX2B4D8B2C,
            allowed_mentions=discord.AllowedMentions(
                users=False if OX5DAF3FA4 in OX46A1E4F8.message.mentions else [OX5DAF3FA4]
            ),
        )

    @OX4A0E59BE.group()
    @OX0BDEA1CF.is_owner()
    async def OX7D9A6F1C(self, OX46A1E4F8: OX4A0E59BE.Context):
        pass

    @OX7D9A6F1C.group(aliases=["custom"], invoke_without_command=True)
    @OX0BDEA1CF.admin_or_permissions(manage_guild=True)
    @OX4A0E59BE.guild_only()
    async def OX5E1D6C3B(self, OX46A1E4F8: OX4A0E59BE.Context, OX5AB3CF91: str.lower, *, OX2B4D8B2C: str = None):
        if not OX2B4D8B2C:
            await self.OX7AFD0F63.guild(OX46A1E4F8.guild).clear_raw("custom", OX5AB3CF91)
            await OX46A1E4F8.tick()
        else:
            await self.OX7AFD0F63.guild(OX46A1E4F8.guild).set_raw("custom", OX5AB3CF91, value=OX2B4D8B2C)
            await OX46A1E4F8.send(
                OX30D5E4B1.sub(functools.partial(self.OX3F3F3E9A, OX46A1E4F8.author), OX2B4D8B2C),
                allowed_mentions=discord.AllowedMentions(users=False),
            )

    @OX5E1D6C3B.command(name="global")
    @OX0BDEA1CF.is_owner()
    async def OX2E7D8C6B(
        self, OX46A1E4F8: OX4A0E59BE.Context, OX5AB3CF91: str.lower, *, OX2B4D8B2C: str = None
    ):
        if not OX2B4D8B2C:
            await self.OX7AFD0F63.clear_raw("custom", OX5AB3CF91)
        else:
            await self.OX7AFD0F63.set_raw("custom", OX5AB3CF91, value=OX2B4D8B2C)
        await OX46A1E4F8.tick()

    @OX7D9A6F1C.group(invoke_without_command=True)
    @OX0BDEA1CF.admin_or_permissions(manage_guild=True)
    @OX4A0E59BE.guild_only()
    async def OX1E7B9F4D(self, OX46A1E4F8: OX4A0E59BE.Context, OX5AB3CF91: str.lower):
        try:
            OX2B4D8B2C = await self.OX7AFD0F63.guild(OX46A1E4F8.guild).get_raw("custom", OX5AB3CF91)
        except KeyError:
            OX2B4D8B2C = NotImplemented
        if OX2B4D8B2C is None:
            await self.OX7AFD0F63.guild(OX46A1E4F8.guild).clear_raw("custom", OX5AB3CF91)
            await OX46A1E4F8.send("I will no longer ignore the {OX5AB3CF91} action".format(OX5AB3CF91=OX5AB3CF91))
        else:
            await self.OX7AFD0F63.guild(OX46A1E4F8.guild).set_raw("custom", OX5AB3CF91, value=None)
            await OX46A1E4F8.send("I will now ignore the {OX5AB3CF91} action".format(OX5AB3CF91=OX5AB3CF91))

    @OX1E7B9F4D.command(name="global")
    @OX0BDEA1CF.is_owner()
    async def OX7B8C9D2A(self, OX46A1E4F8: OX4A0E59BE.Context, OX5AB3CF91: str.lower):
        try:
            await self.OX7AFD0F63.get_raw("custom", OX5AB3CF91)
        except KeyError:
            await self.OX7AFD0F63.set_raw("custom", OX5AB3CF91, value=None)
        else:
            await self.OX7AFD0F63.clear_raw("custom", OX5AB3CF91)
        await OX46A1E4F8.tick()

    @OX7D9A6F1C.command()
    @OX0BDEA1CF.is_owner()
    async def OX6A3E4C2D(self, OX46A1E4F8: OX4A0E59BE.Context):
        OX3F5A7B9D = [
            "Go to the Tenor developer dashboard: https://tenor.com/developer/dashboard",
            "Log in or sign up if you haven't already.",
            "Click `+ Create new app` and fill out the form.",
            "Copy the key from the app you just created.",
            "Give the key to Red with this command:\n"
            f"`{OX46A1E4F8.prefix}set api tenor api_key your_api_key`\n"
            "Replace `your_api_key` with the key you just got.\n"
            "Everything else should be the same.",
        ]
        OX3F5A7B9D = [f"**{i}.** {v}" for i, v in enumerate(OX3F5A7B9D, 1)]
        await OX46A1E4F8.maybe_send_embed("\n".join(OX3F5A7B9D))

    @OX4A0E59BE.Cog.listener()
    async def OX1B6C8A3E(
        self, OX46A1E4F8: OX4A0E59BE.Context, OX7E8D9F1A: OX4A0E59BE.CommandError, OX6E4F9D2B: bool = False
    ):
        if OX46A1E4F8.command == self.OX3A6D4B7A:
            return
        if isinstance(OX7E8D9F1A, OX4A0E59BE.UserFeedbackCheckFailure):
            return
        elif isinstance(OX7E8D9F1A, (OX4A0E59BE.CheckFailure, OX4A0E59BE.CommandNotFound)):
            OX46A1E4F8.command = self.OX3A6D4B7A
            await OX46A1E4F8.bot.invoke(OX46A1E4F8)