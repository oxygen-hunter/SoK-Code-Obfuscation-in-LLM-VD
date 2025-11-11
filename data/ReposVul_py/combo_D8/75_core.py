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

class TicketsCore:
    async def red_delete_data_for_user(self, **kwargs):
        return

    def __init__(self, bot):
        self.bot = bot
        self.config = Config.get_conf(self, identifier=2134287593)
        guild_data = self.get_default_guild_data()
        self.config.register_guild(**guild_data)
        self.ticket_info_format = self.get_ticket_info_format()

    def get_default_guild_data(self):
        return {
            "category": None,
            "closed_category": None,
            "ticket_role": None,
            "default_message_ticket_channel": None,
            "sessions": {},
        }

    def get_ticket_info_format(self):
        return "\n\n**[{datetime}]** [{author}]\n{information}"

    async def create_ticket(self, context):
        guild = context.guild
        author = context.author
        ticket_role = await self.get_ticket_role(guild)
        category_channel = await self.config.guild(guild).category()
        default_message_ticket_channel = await self.config.guild(guild).default_message_ticket_channel()

        if self.is_valid_category(category_channel, guild):
            ticket_id = self.generate_ticket_id()
            ticket_channel = await self.create_ticket_channel(guild, category_channel, author, ticket_id)
            await self.set_channel_permissions(ticket_channel, author, guild)
            await self.edit_channel_topic(ticket_channel, ticket_id, author, "Ticket opened")
            await self.send_default_message(ticket_channel, default_message_ticket_channel, author, context, ticket_role)
            await self.update_sessions(guild, ticket_channel, author)
        else:
            return "Naughty! You need to run the setup first."

    async def get_ticket_role(self, guild):
        return [role for role in guild.roles if await self.config.guild(guild).ticket_role() == role.id]

    def is_valid_category(self, category_channel, guild):
        return category_channel and category_channel in [category.id for category in guild.categories]

    def generate_ticket_id(self):
        n1 = 10 ** 10
        n2 = n1 * 10 - 1
        return int(random.randint(n1, n2))

    async def create_ticket_channel(self, guild, category_channel, author, ticket_id):
        return await guild.create_text_channel(
            "{}-{}".format(author.display_name, ticket_id),
            category=self.bot.get_channel(category_channel),
        )

    async def set_channel_permissions(self, ticket_channel, author, guild):
        await ticket_channel.set_permissions(author, read_messages=True, send_messages=True)
        await ticket_channel.set_permissions(guild.me, read_messages=True, send_messages=True, manage_channels=True)

    async def edit_channel_topic(self, ticket_channel, ticket_id, author, information):
        await ticket_channel.edit(
            topic=self.ticket_info_format.format(
                ticket=ticket_id,
                datetime=datetime.utcnow().strftime("%d/%m/%Y %H:%M:%S"),
                author=author.display_name,
                information=information,
            )
        )

    async def send_default_message(self, ticket_channel, default_message_ticket_channel, author, context, ticket_role):
        if default_message_ticket_channel:
            try:
                await ticket_channel.send(
                    default_message_ticket_channel.format(
                        member=SafeMember(author),
                        channel=ticket_channel,
                        origin=context.channel,
                        ticket_role=ticket_role,
                    )
                )
            except:
                return "Oops there has been an unexpected error with your new ticket message. Please contact the bot owner for assistance"

    async def update_sessions(self, guild, ticket_channel, author):
        async with self.config.guild(guild).sessions() as session:
            session.update({ticket_channel.id: author.id})

    async def update_ticket(self, context, status):
        await self.delete_context_message(context)
        guild = context.guild
        channel = context.channel
        author = context.author
        sessions = await self.config.guild(guild).sessions()

        if self.can_update_ticket(channel, sessions, guild, author):
            ticket_id = str(channel.name).split("-")[1]
            await self.edit_channel_topic(channel, ticket_id, author, status)

    async def delete_context_message(self, context):
        try:
            await context.message.delete()
        except discord.Forbidden:
            pass

    def can_update_ticket(self, channel, sessions, guild, author):
        return str(channel.id) in sessions and await self.config.guild(guild).ticket_role() in [role.id for role in author.roles]

    async def close_ticket(self, context):
        await self.delete_context_message(context)
        guild = context.guild
        channel = context.channel
        author = context.author
        sessions = await self.config.guild(guild).sessions()

        if not self.is_valid_session(channel, sessions):
            return await self.send_invalid_session_message(channel)
        if not await self.has_ticket_management_role(guild, author):
            return await self.send_insufficient_permissions_message(channel)
        else:
            await self.process_ticket_closure(channel, guild, author, sessions)

    def is_valid_session(self, channel, sessions):
        return str(channel.id) in sessions

    async def send_invalid_session_message(self, channel):
        return await channel.send("Make sure you are doing this within the ticket channel that you want to close.")

    async def has_ticket_management_role(self, guild, author):
        return await self.config.guild(guild).ticket_role() in [role.id for role in author.roles]

    async def send_insufficient_permissions_message(self, channel):
        return await channel.send("You do not have the proper role to manage tickets")

    async def process_ticket_closure(self, channel, guild, author, sessions):
        member = guild.get_member(sessions[str(channel.id)])
        ticket_id = str(channel.name).split("-")[1]
        closed_category = await self.config.guild(guild).closed_category()
        closed_category = self.bot.get_channel(closed_category)

        await self.adjust_channel_permissions(channel, member)
        await self.move_channel_to_closed_category(channel, closed_category, ticket_id, author)
        await self.remove_session_entry(guild, channel)

    async def adjust_channel_permissions(self, channel, member):
        await channel.set_permissions(member, read_messages=True, send_messages=False)

    async def move_channel_to_closed_category(self, channel, closed_category, ticket_id, author):
        await channel.edit(
            category=closed_category,
            topic=channel.topic
            + self.ticket_info_format.format(
                ticket=ticket_id,
                datetime=datetime.utcnow().strftime("%d/%m/%Y %H:%M:%S"),
                author=author.display_name,
                information="Ticket closed",
            ),
        )

    async def remove_session_entry(self, guild, channel):
        async with self.config.guild(guild).sessions() as session:
            session.pop(channel.id, None)

    async def purge_tickets(self, context):
        try:
            guild = context.guild
            closed_channels = await self.get_closed_channels(guild)
            await self.delete_closed_channels(closed_channels)
            return "All closed tickets removed!"
        except discord.Forbidden:
            return "I need permissions to manage channels."

    async def get_closed_channels(self, guild):
        return [
            channel for channel in guild.channels if channel.category_id == await self.config.guild(guild).closed_category()
        ]

    async def delete_closed_channels(self, closed_channels):
        for channel in closed_channels:
            await channel.delete()

    async def set_default_message_ticket_channel(self, context, message):
        guild = context.guild
        await self.config.guild(guild).default_message_ticket_channel.set(message)
        return "Your default message has been set."

    async def automatic_setup(self, context):
        guild = context.guild
        try:
            overwrites = self.get_overwrites(guild)
            category_channel, closed_category_channel = await self.create_ticket_categories(guild, overwrites)
            ticket_role = await self.create_ticket_role(guild)
            await self.set_category_permissions(category_channel, closed_category_channel, ticket_role)
            await self.save_setup_config(guild, category_channel, closed_category_channel, ticket_role)
            return ":tada: Fabulous! You're all done! Now add the `Ticket` role to anyone who you deem good enough to handle tickets. And if you care, you can change the name of the role and category if you _really_ want to."
        except discord.Forbidden:
            return "That didn't go well... I need permissions to manage channels and manage roles. :rolling_eyes:"

    def get_overwrites(self, guild):
        return {
            guild.default_role: discord.PermissionOverwrite(
                send_messages=False, read_messages=False
            ),
        }

    async def create_ticket_categories(self, guild, overwrites):
        category_channel = await guild.create_category("Tickets", overwrites=overwrites)
        closed_category_channel = await guild.create_category("Closed Tickets", overwrites=overwrites)
        return category_channel, closed_category_channel

    async def create_ticket_role(self, guild):
        return await guild.create_role(name="Ticket")

    async def set_category_permissions(self, category_channel, closed_category_channel, ticket_role):
        await category_channel.set_permissions(ticket_role, read_messages=True, send_messages=True)
        await closed_category_channel.set_permissions(ticket_role, read_messages=True, send_messages=True)

    async def save_setup_config(self, guild, category_channel, closed_category_channel, ticket_role):
        await self.config.guild(guild).category.set(category_channel.id)
        await self.config.guild(guild).closed_category.set(closed_category_channel.id)
        await self.config.guild(guild).ticket_role.set(ticket_role.id)