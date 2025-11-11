from redbot.core import Config
from datetime import datetime
import discord
import random


class OX5A3C2D99:
    def __init__(self, OX2D7A6F1B: discord.Member):
        self.OX8FB7E2A1 = OX2D7A6F1B.name
        self.OX9C1E8F4D = OX2D7A6F1B.mention

    def __str__(self):
        return self.OX8FB7E2A1

    def __getattr__(self, OX4E2D9F8B):
        return ""


class OX7F8A6B1C:
    async def OX8E1F3C9A(self, **OX6A2C7D4F):
        return

    def __init__(self, OX1A9D6F3B):
        self.OX3B7E9D5C = OX1A9D6F3B

        self.OX6D7A2C9E = Config.get_conf(self, identifier=2134287593)
        OX7B3D9F5A = {
            "category": None,
            "closed_category": None,
            "ticket_role": None,
            "default_message_ticket_channel": None,
            "sessions": {},
        }
        self.OX6D7A2C9E.register_guild(**OX7B3D9F5A)

        self.OX2E9A5D7C = "\n\n**[{datetime}]** [{author}]\n{information}"

    async def OX1F7B6E2C(self, OX5D8C4A9E):
        OX9A2B5F7D = OX5D8C4A9E.guild
        OX7D3E9B1A = OX5D8C4A9E.author

        OX4E2A9D7F = [
            OX4B9D6F3C
            for OX4B9D6F3C in OX9A2B5F7D.roles
            if await self.OX6D7A2C9E.guild(OX9A2B5F7D).ticket_role() == OX4B9D6F3C.id
        ]

        if OX4E2A9D7F:
            OX4E2A9D7F = OX4E2A9D7F[0]
        OX5C7F3A8D = await self.OX6D7A2C9E.guild(OX9A2B5F7D).category()
        OX7F3A9E2B = await self.OX6D7A2C9E.guild(
            OX9A2B5F7D
        ).default_message_ticket_channel()

        if OX5C7F3A8D and OX5C7F3A8D in [
            OX5A9D7B3C.id for OX5A9D7B3C in OX9A2B5F7D.categories
        ]:
            OX6D7E9A2B = 10 ** 10
            OX4B9F6C3D = OX6D7E9A2B * 10 - 1
            OX3A9D7F5B = int(random.randint(OX6D7E9A2B, OX4B9F6C3D))
            OX2F8A1B3C = await OX9A2B5F7D.create_text_channel(
                "{}-{}".format(OX7D3E9B1A.display_name, OX3A9D7F5B),
                category=self.OX3B7E9D5C.get_channel(OX5C7F3A8D),
            )

            await OX2F8A1B3C.set_permissions(
                OX7D3E9B1A, read_messages=True, send_messages=True
            )
            await OX2F8A1B3C.set_permissions(
                OX9A2B5F7D.me, read_messages=True, send_messages=True, manage_channels=True
            )

            await OX2F8A1B3C.edit(
                topic=self.OX2E9A5D7C.format(
                    ticket=OX3A9D7F5B,
                    datetime=datetime.utcnow().strftime("%d/%m/%Y %H:%M:%S"),
                    author=OX7D3E9B1A.display_name,
                    information="Ticket opened",
                )
            )

            if OX7F3A9E2B:
                try:
                    await OX2F8A1B3C.send(
                        OX7F3A9E2B.format(
                            member=OX5A3C2D99(OX7D3E9B1A),
                            channel=OX2F8A1B3C,
                            origin=OX5D8C4A9E.channel,
                            ticket_role=OX4E2A9D7F,
                        )
                    )
                except:
                    return "Oops there has been an unexpected error with your new ticket message. Please contact the bot owner for assistance"

            async with self.OX6D7A2C9E.guild(OX9A2B5F7D).sessions() as OX1B9D3F5A:
                OX1B9D3F5A.update({OX2F8A1B3C.id: OX7D3E9B1A.id})

        else:
            return "Naughty! You need to run the setup first."

    async def OX9C2D7B5A(self, OX8E3F1A6B, OX4A9B7D2F):
        try:
            await OX8E3F1A6B.message.delete()
        except discord.Forbidden:
            pass

        OX9B2C5F7A = OX8E3F1A6B.guild
        OX7E8A1B3D = OX8E3F1A6B.channel
        OX2C9D5F8B = OX8E3F1A6B.author

        OX8F4A3B7D = await self.OX6D7A2C9E.guild(OX9B2C5F7A).sessions()

        if str(OX7E8A1B3D.id) in OX8F4A3B7D and await self.OX6D7A2C9E.guild(
            OX9B2C5F7A
        ).ticket_role() in [OX3B7F5A9C.id for OX3B7F5A9C in OX2C9D5F8B.roles]:

            OX5A9D7F3C = str(OX7E8A1B3D.name).split("-")[1]
            await OX7E8A1B3D.edit(
                topic=OX7E8A1B3D.topic
                + self.OX2E9A5D7C.format(
                    ticket=OX5A9D7F3C,
                    datetime=datetime.utcnow().strftime("%d/%m/%Y %H:%M:%S"),
                    author=OX2C9D5F8B.display_name,
                    information=OX4A9B7D2F,
                )
            )

    async def OX1A7E3B5C(self, OX6F9D2A8B):
        try:
            await OX6F9D2A8B.message.delete()
        except discord.Forbidden:
            pass

        OX5A8D3C9F = OX6F9D2A8B.guild
        OX8B7F2A1D = OX6F9D2A8B.channel
        OX9F3A6D4B = OX6F9D2A8B.author

        OX4C2B9F7D = await self.OX6D7A2C9E.guild(OX5A8D3C9F).sessions()

        if str(OX8B7F2A1D.id) not in OX4C2B9F7D:
            return await OX8B7F2A1D.send(
                "Make sure you are doing this within the ticket channel that you want to close."
            )
        if await self.OX6D7A2C9E.guild(OX5A8D3C9F).ticket_role() not in [
            OX4A7F2C9D.id for OX4A7F2C9D in OX9F3A6D4B.roles
        ]:
            return await OX8B7F2A1D.send(
                "You do not have the proper role to manage tickets"
            )
        else:
            OX1D9A3F5B = OX5A8D3C9F.get_member(OX4C2B9F7D[str(OX8B7F2A1D.id)])
            OX9B3D7F1C = str(OX8B7F2A1D.name).split("-")[1]

            OX4D2F7B9A = await self.OX6D7A2C9E.guild(OX5A8D3C9F).closed_category()
            OX4D2F7B9A = self.OX3B7E9D5C.get_channel(OX4D2F7B9A)

            await OX8B7F2A1D.set_permissions(
                OX1D9A3F5B, read_messages=True, send_messages=False
            )
            await OX8B7F2A1D.edit(
                category=OX4D2F7B9A,
                topic=OX8B7F2A1D.topic
                + self.OX2E9A5D7C.format(
                    ticket=OX9B3D7F1C,
                    datetime=datetime.utcnow().strftime("%d/%m/%Y %H:%M:%S"),
                    author=OX9F3A6D4B.display_name,
                    information="Ticket closed",
                ),
            )

            async with self.OX6D7A2C9E.guild(OX5A8D3C9F).sessions() as OX7A4B9D5C:
                OX7A4B9D5C.pop(OX8B7F2A1D.id, None)

    async def OX3D9A5B7F(self, OX1F6B3E8A):
        try:
            OX7A8C1D3B = OX1F6B3E8A.guild
            OX2D9B5F7A = [
                OX5B8F3A7C
                for OX5B8F3A7C in OX7A8C1D3B.channels
                if OX5B8F3A7C.category_id
                == await self.OX6D7A2C9E.guild(OX7A8C1D3B).closed_category()
            ]
            for OX5B8F3A7C in OX2D9B5F7A:
                await OX5B8F3A7C.delete()

            return "All closed tickets removed!"
        except discord.Forbidden:
            return "I need permissions to manage channels."

    async def OX9B4A7D6F(self, OX3C9D7F2A, OX5A8B1E6C):
        OX2C7F9B4A = OX3C9D7F2A.guild

        await self.OX6D7A2C9E.guild(OX2C7F9B4A).default_message_ticket_channel.set(OX5A8B1E6C)

        return "Your default message has been set."

    async def OX5D9A7F3C(self, OX7B1E2A4C):
        OX6C9F3A7D = OX7B1E2A4C.guild

        try:
            OX3B8A1D7C = {
                OX6C9F3A7D.default_role: discord.PermissionOverwrite(
                    send_messages=False, read_messages=False
                ),
            }

            OX8F2A1B9D = await OX6C9F3A7D.create_category(
                "Tickets", overwrites=OX3B8A1D7C
            )
            OX5D7A9F2B = await OX6C9F3A7D.create_category(
                "Closed Tickets", overwrites=OX3B8A1D7C
            )

            OX4A3D9B7F = await OX6C9F3A7D.create_role(name="Ticket")

            await OX8F2A1B9D.set_permissions(
                OX4A3D9B7F, read_messages=True, send_messages=True
            )
            await OX5D7A9F2B.set_permissions(
                OX4A3D9B7F, read_messages=True, send_messages=True
            )

            await self.OX6D7A2C9E.guild(OX6C9F3A7D).category.set(OX8F2A1B9D.id)
            await self.OX6D7A2C9E.guild(OX6C9F3A7D).closed_category.set(
                OX5D7A9F2B.id
            )
            await self.OX6D7A2C9E.guild(OX6C9F3A7D).ticket_role.set(OX4A3D9B7F.id)

            return ":tada: Fabulous! You're all done! Now add the `Ticket` role to anyone who you deem good enough to handle tickets. And if you care, you can change the name of the role and category if you _really_ want to."
        except discord.Forbidden:
            return "That didn't go well... I need permissions to manage channels and manage roles. :rolling_eyes:"