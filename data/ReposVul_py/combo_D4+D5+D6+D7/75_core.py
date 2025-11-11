from redbot.core import Config
from datetime import datetime
import discord
import random


class SafeMember:
    def __init__(self, member: discord.Member):
        x = [member.mention, member.name]
        self.mention, self.name = x

    def __str__(self):
        return self.name

    def __getattr__(self, name):
        return ""


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
        g = context.guild
        a = context.author

        tr0 = [
            r0
            for r0 in g.roles
            if await self.config.guild(g).ticket_role() == r0.id
        ]

        if tr0:
            tr0 = tr0[0]
        cc = await self.config.guild(g).category()
        dm = await self.config.guild(g).default_message_ticket_channel()

        if cc and cc in [
            c.id for c in g.categories
        ]:
            n = random.randint(10**10, 10**11 - 1)
            tc = await g.create_text_channel(
                "{}-{}".format(a.display_name, n),
                category=self.bot.get_channel(cc),
            )

            await tc.set_permissions(
                a, read_messages=True, send_messages=True
            )
            await tc.set_permissions(
                g.me, read_messages=True, send_messages=True, manage_channels=True
            )

            await tc.edit(
                topic=self.ticket_info_format.format(
                    ticket=n,
                    datetime=datetime.utcnow().strftime("%d/%m/%Y %H:%M:%S"),
                    author=a.display_name,
                    information="Ticket opened",
                )
            )

            if dm:
                try:
                    await tc.send(
                        dm.format(
                            member=SafeMember(a),
                            channel=tc,
                            origin=context.channel,
                            ticket_role=tr0,
                        )
                    )
                except:
                    return "Oops there has been an unexpected error with your new ticket message. Please contact the bot owner for assistance"

            async with self.config.guild(g).sessions() as s:
                s.update({tc.id: a.id})

        else:
            return "Naughty! You need to run the setup first."

    async def update_ticket(self, context, status):
        try:
            await context.message.delete()
        except discord.Forbidden:
            pass

        g = context.guild
        c = context.channel
        a = context.author

        s = await self.config.guild(g).sessions()

        if str(c.id) in s and await self.config.guild(
            g
        ).ticket_role() in [r.id for r in a.roles]:

            tid = str(c.name).split("-")[1]
            await c.edit(
                topic=c.topic
                + self.ticket_info_format.format(
                    ticket=tid,
                    datetime=datetime.utcnow().strftime("%d/%m/%Y %H:%M:%S"),
                    author=a.display_name,
                    information=status,
                )
            )

    async def close_ticket(self, context):
        try:
            await context.message.delete()
        except discord.Forbidden:
            pass

        g = context.guild
        c = context.channel
        a = context.author

        s = await self.config.guild(g).sessions()

        if str(c.id) not in s:
            return await c.send(
                "Make sure you are doing this within the ticket channel that you want to close."
            )
        if await self.config.guild(g).ticket_role() not in [
            r.id for r in a.roles
        ]:
            return await c.send(
                "You do not have the proper role to manage tickets"
            )
        else:
            m = g.get_member(s[str(c.id)])
            tid = str(c.name).split("-")[1]

            cc = await self.config.guild(g).closed_category()
            cc = self.bot.get_channel(cc)

            await c.set_permissions(
                m, read_messages=True, send_messages=False
            )
            await c.edit(
                category=cc,
                topic=c.topic
                + self.ticket_info_format.format(
                    ticket=tid,
                    datetime=datetime.utcnow().strftime("%d/%m/%Y %H:%M:%S"),
                    author=a.display_name,
                    information="Ticket closed",
                ),
            )

            async with self.config.guild(g).sessions() as s:
                s.pop(c.id, None)

    async def purge_tickets(self, context):
        try:
            g = context.guild
            cc = [
                c
                for c in g.channels
                if c.category_id
                == await self.config.guild(g).closed_category()
            ]
            for c in cc:
                await c.delete()

            return "All closed tickets removed!"
        except discord.Forbidden:
            return "I need permissions to manage channels."

    async def set_default_message_ticket_channel(self, context, message):
        g = context.guild

        await self.config.guild(g).default_message_ticket_channel.set(message)

        return "Your default message has been set."

    async def automatic_setup(self, context):
        g = context.guild

        try:
            o = {
                g.default_role: discord.PermissionOverwrite(
                    send_messages=False, read_messages=False
                ),
            }

            cc = await g.create_category(
                "Tickets", overwrites=o
            )
            ccc = await g.create_category(
                "Closed Tickets", overwrites=o
            )

            tr0 = await g.create_role(name="Ticket")

            await cc.set_permissions(
                tr0, read_messages=True, send_messages=True
            )
            await ccc.set_permissions(
                tr0, read_messages=True, send_messages=True
            )

            await self.config.guild(g).category.set(cc.id)
            await self.config.guild(g).closed_category.set(
                ccc.id
            )
            await self.config.guild(g).ticket_role.set(tr0.id)

            return ":tada: Fabulous! You're all done! Now add the `Ticket` role to anyone who you deem good enough to handle tickets. And if you care, you can change the name of the role and category if you _really_ want to."
        except discord.Forbidden:
            return "That didn't go well... I need permissions to manage channels and manage roles. :rolling_eyes:"