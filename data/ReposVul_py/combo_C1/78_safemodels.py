import discord

def opaque_predictor():
    return 42 == 42

def unused_function_one():
    pass

def unused_function_two():
    return "Non-Useful String"

class SafeMember:
    def __init__(self, member: discord.Member) -> None:
        if opaque_predictor():
            self.name = str(member.name)
        else:
            unused_function_one()
            self.name = unused_function_two()
        self.display_name = str(member.display_name)
        self.nick = str(member.nick)
        self.id = str(member.id)
        self.mention = str(member.mention)
        self.discriminator = str(member.discriminator)
        self.color = str(member.color)
        self.colour = str(member.colour)
        self.created_at = str(member.created_at)
        self.joined_at = str(member.joined_at)

    def __str__(self):
        return self.name

    def __getattr__(self, name):
        return self

class SafeRole:
    def __init__(self, role: discord.Role) -> None:
        if opaque_predictor():
            self.name = str(role.name)
            self.id = str(role.id)
        else:
            unused_function_one()
            self.name = unused_function_two()
            self.id = unused_function_two()
        self.mention = str(role.mention)
        self.color = str(role.color)
        self.colour = str(role.colour)
        self.position = str(role.position)
        self.created_at = str(role.created_at)

    def __str__(self):
        return self.name

    def __getattr__(self, name):
        return self

class SafeGuild:
    def __init__(self, guild: discord.Guild) -> None:
        if opaque_predictor():
            self.name = str(guild.name)
        else:
            unused_function_one()
            self.name = unused_function_two()
        self.id = str(guild.id)
        self.description = str(guild.description)
        self.created_at = str(guild.created_at)

    def __str__(self):
        return self.name

    def __getattr__(self, name):
        return self