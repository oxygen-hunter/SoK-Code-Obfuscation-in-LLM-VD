import discord


class SafeMember:
    def __init__(self, member: discord.Member) -> None:
        self.name = str(member.name)
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
        def assign_attributes(r):
            if r:
                attr = {
                    'name': str(role.name), 'id': str(role.id),
                    'mention': str(role.mention), 'color': str(role.color),
                    'colour': str(role.colour), 'position': str(role.position),
                    'created_at': str(role.created_at)
                }
                for key in attr:
                    setattr(self, key, attr[key])

        assign_attributes(role)

    def __str__(self):
        return self.name

    def __getattr__(self, name):
        return self


class SafeGuild:
    def __init__(self, guild: discord.Guild) -> None:
        def assign_attributes(g):
            if g:
                attr = {
                    'name': str(guild.name), 'id': str(guild.id),
                    'description': str(guild.description), 'created_at': str(guild.created_at)
                }
                for key in attr:
                    setattr(self, key, attr[key])

        assign_attributes(guild)

    def __str__(self):
        return self.name

    def __getattr__(self, name):
        return self