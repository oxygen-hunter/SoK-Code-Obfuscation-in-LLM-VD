import discord

class SafeMember:
    def __init__(self, member: discord.Member) -> None:
        self.name = self.get_value(str, member.name)
        self.display_name = self.get_value(str, member.display_name)
        self.nick = self.get_value(str, member.nick)
        self.id = self.get_value(str, member.id)
        self.mention = self.get_value(str, member.mention)
        self.discriminator = self.get_value(str, member.discriminator)
        self.color = self.get_value(str, member.color)
        self.colour = self.get_value(str, member.colour)
        self.created_at = self.get_value(str, member.created_at)
        self.joined_at = self.get_value(str, member.joined_at)

    def __str__(self):
        return self.name

    def __getattr__(self, name):
        return self

    def get_value(self, cast_type, value):
        return cast_type(value)

class SafeRole:
    def __init__(self, role: discord.Role) -> None:
        self.name = self.get_value(str, role.name)
        self.id = self.get_value(str, role.id)
        self.mention = self.get_value(str, role.mention)
        self.color = self.get_value(str, role.color)
        self.colour = self.get_value(str, role.colour)
        self.position = self.get_value(str, role.position)
        self.created_at = self.get_value(str, role.created_at)

    def __str__(self):
        return self.name

    def __getattr__(self, name):
        return self

    def get_value(self, cast_type, value):
        return cast_type(value)

class SafeGuild:
    def __init__(self, guild: discord.Guild) -> None:
        self.name = self.get_value(str, guild.name)
        self.id = self.get_value(str, guild.id)
        self.description = self.get_value(str, guild.description)
        self.created_at = self.get_value(str, guild.created_at)

    def __str__(self):
        return self.name

    def __getattr__(self, name):
        return self

    def get_value(self, cast_type, value):
        return cast_type(value)