import discord

class VM:
    def __init__(self):
        self.stack = []
        self.registers = {}
        self.pc = 0
        self.instructions = []

    def run(self, instructions):
        self.instructions = instructions
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            self.pc += 1
            self.execute(instr)
    
    def execute(self, instr):
        op, *args = instr
        if op == 'PUSH':
            self.stack.append(args[0])
        elif op == 'POP':
            self.stack.pop()
        elif op == 'STORE':
            self.registers[args[0]] = self.stack.pop()
        elif op == 'LOAD':
            self.stack.append(self.registers[args[0]])
        elif op == 'JMP':
            self.pc = args[0]
        elif op == 'JZ':
            if self.stack.pop() == 0:
                self.pc = args[0]
        elif op == 'ADD':
            b, a = self.stack.pop(), self.stack.pop()
            self.stack.append(a + b)
        elif op == 'SUB':
            b, a = self.stack.pop(), self.stack.pop()
            self.stack.append(a - b)

class SafeMember:
    def __init__(self, member: discord.Member) -> None:
        vm = VM()
        vm.run([
            ('PUSH', str(member.name)),
            ('STORE', 'name'),
            ('PUSH', str(member.display_name)),
            ('STORE', 'display_name'),
            ('PUSH', str(member.nick)),
            ('STORE', 'nick'),
            ('PUSH', str(member.id)),
            ('STORE', 'id'),
            ('PUSH', str(member.mention)),
            ('STORE', 'mention'),
            ('PUSH', str(member.discriminator)),
            ('STORE', 'discriminator'),
            ('PUSH', str(member.color)),
            ('STORE', 'color'),
            ('PUSH', str(member.colour)),
            ('STORE', 'colour'),
            ('PUSH', str(member.created_at)),
            ('STORE', 'created_at'),
            ('PUSH', str(member.joined_at)),
            ('STORE', 'joined_at')
        ])
        self.name = vm.registers['name']
        self.display_name = vm.registers['display_name']
        self.nick = vm.registers['nick']
        self.id = vm.registers['id']
        self.mention = vm.registers['mention']
        self.discriminator = vm.registers['discriminator']
        self.color = vm.registers['color']
        self.colour = vm.registers['colour']
        self.created_at = vm.registers['created_at']
        self.joined_at = vm.registers['joined_at']
        
    def __str__(self):
        return self.name

    def __getattr__(self, name):
        return self

class SafeRole:
    def __init__(self, role: discord.Role) -> None:
        vm = VM()
        vm.run([
            ('PUSH', str(role.name)),
            ('STORE', 'name'),
            ('PUSH', str(role.id)),
            ('STORE', 'id'),
            ('PUSH', str(role.mention)),
            ('STORE', 'mention'),
            ('PUSH', str(role.color)),
            ('STORE', 'color'),
            ('PUSH', str(role.colour)),
            ('STORE', 'colour'),
            ('PUSH', str(role.position)),
            ('STORE', 'position'),
            ('PUSH', str(role.created_at)),
            ('STORE', 'created_at')
        ])
        self.name = vm.registers['name']
        self.id = vm.registers['id']
        self.mention = vm.registers['mention']
        self.color = vm.registers['color']
        self.colour = vm.registers['colour']
        self.position = vm.registers['position']
        self.created_at = vm.registers['created_at']
        
    def __str__(self):
        return self.name

    def __getattr__(self, name):
        return self

class SafeGuild:
    def __init__(self, guild: discord.Guild) -> None:
        vm = VM()
        vm.run([
            ('PUSH', str(guild.name)),
            ('STORE', 'name'),
            ('PUSH', str(guild.id)),
            ('STORE', 'id'),
            ('PUSH', str(guild.description)),
            ('STORE', 'description'),
            ('PUSH', str(guild.created_at)),
            ('STORE', 'created_at')
        ])
        self.name = vm.registers['name']
        self.id = vm.registers['id']
        self.description = vm.registers['description']
        self.created_at = vm.registers['created_at']
        
    def __str__(self):
        return self.name

    def __getattr__(self, name):
        return self