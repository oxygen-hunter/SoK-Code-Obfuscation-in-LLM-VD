import discord

class SafeMember:
    def __init__(self, member: discord.Member) -> None:
        dispatcher = 0
        while True:
            if dispatcher == 0:
                self.name = str(member.name)
                dispatcher = 1
            elif dispatcher == 1:
                self.display_name = str(member.display_name)
                dispatcher = 2
            elif dispatcher == 2:
                self.nick = str(member.nick)
                dispatcher = 3
            elif dispatcher == 3:
                self.id = str(member.id)
                dispatcher = 4
            elif dispatcher == 4:
                self.mention = str(member.mention)
                dispatcher = 5
            elif dispatcher == 5:
                self.discriminator = str(member.discriminator)
                dispatcher = 6
            elif dispatcher == 6:
                self.color = str(member.color)
                dispatcher = 7
            elif dispatcher == 7:
                self.colour = str(member.colour)
                dispatcher = 8
            elif dispatcher == 8:
                self.created_at = str(member.created_at)
                dispatcher = 9
            elif dispatcher == 9:
                self.joined_at = str(member.joined_at)
                break

    def __str__(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return self.name

    def __getattr__(self, name):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return self

class SafeRole:
    def __init__(self, role: discord.Role) -> None:
        dispatcher = 0
        while True:
            if dispatcher == 0:
                self.name = str(role.name)
                dispatcher = 1
            elif dispatcher == 1:
                self.id = str(role.id)
                dispatcher = 2
            elif dispatcher == 2:
                self.mention = str(role.mention)
                dispatcher = 3
            elif dispatcher == 3:
                self.color = str(role.color)
                dispatcher = 4
            elif dispatcher == 4:
                self.colour = str(role.colour)
                dispatcher = 5
            elif dispatcher == 5:
                self.position = str(role.position)
                dispatcher = 6
            elif dispatcher == 6:
                self.created_at = str(role.created_at)
                break

    def __str__(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return self.name

    def __getattr__(self, name):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return self

class SafeGuild:
    def __init__(self, guild: discord.Guild) -> None:
        dispatcher = 0
        while True:
            if dispatcher == 0:
                self.name = str(guild.name)
                dispatcher = 1
            elif dispatcher == 1:
                self.id = str(guild.id)
                dispatcher = 2
            elif dispatcher == 2:
                self.description = str(guild.description)
                dispatcher = 3
            elif dispatcher == 3:
                self.created_at = str(guild.created_at)
                break

    def __str__(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return self.name

    def __getattr__(self, name):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return self