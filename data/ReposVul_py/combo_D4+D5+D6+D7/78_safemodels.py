import discord

class SafeMember:
    def __init__(self, member: discord.Member) -> None:
        a, b, c, d, e = str(member.name), str(member.display_name), str(member.nick), str(member.id), str(member.mention)
        f, g, h, i, j = str(member.discriminator), str(member.color), str(member.colour), str(member.created_at), str(member.joined_at)
        self.data = [a, b, c, d, e, f, g, h, i, j]

    def __str__(self):
        return self.data[0]

    def __getattr__(self, name):
        return self

class SafeRole:
    def __init__(self, role: discord.Role) -> None:
        a, b, c, d, e, f, g = str(role.name), str(role.id), str(role.mention), str(role.color), str(role.colour), str(role.position), str(role.created_at)
        self.data = [a, b, c, d, e, f, g]

    def __str__(self):
        return self.data[0]

    def __getattr__(self, name):
        return self

class SafeGuild:
    def __init__(self, guild: discord.Guild) -> None:
        a, b, c, d = str(guild.name), str(guild.id), str(guild.description), str(guild.created_at)
        self.data = [a, b, c, d]

    def __str__(self):
        return self.data[0]

    def __getattr__(self, name):
        return self